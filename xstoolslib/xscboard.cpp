/*----------------------------------------------------------------------------------
	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
	02111-1307, USA.

	1997-2010 - X Engineering Software Systems Corp.
----------------------------------------------------------------------------------*/


#include <cassert>
#include <ctime>
#include <fstream>
#include <string>

#include <string.h>

using namespace std;

#include "progress.h"
#include "jtaginstr.h"
#include "lptjtag.h"
#include "usbjtag.h"
#include "xscboard.h"


#define	DWNLD_BUFFER_SIZE	0x80000

// get execution time for a statement
//#ifdef _DEBUG
#if 0
#include "precision_timer.h"
precision_timer prec_timer = 0;
#define TIME_IT(stmnt)\
{\
	prec_timer.start();\
	stmnt;\
	prec_timer.stop();\
}
#else
int prec_timer = 0;
#define TIME_IT(stmnt) stmnt;
#endif


//##########################################################################################################
// Look at xsboard.h for a description of the interface.
XSCBoard::XSCBoard(void)
{
	XSError* err = new XSError(cerr);		// create error-reporting channel
	brdModel = NULL;
	Setup(err,"XSC-3S1600E",0);				// use a default board model and USB port at this point
}


//##########################################################################################################
// Look at xsboard.h for a description of the interface.
XSCBoard::~XSCBoard(void)
{
	;
}
		

//##########################################################################################################
// Look at xsboard.h for a description of the interface.
void XSCBoard::SetFlags(unsigned long f)
{
	flags = f;
}


//##########################################################################################################
// Look at xsboard.h for a description of the interface.
unsigned long XSCBoard::GetFlags(void)
{
	return flags;
}


//##########################################################################################################
// Look at xsboard.h for a description of the interface.
bool XSCBoard::Setup(XSError* err, const char* model, unsigned int prtNum)
{
	// store some global information for the board
	brdErr = err;
	portType = PORTTYPE_USBJTAG;
	portNum = prtNum;

	// store name of board model with port type appended
	if(brdModel!=NULL)
		free(brdModel);
	string fullModel = (string)model;
	brdModel = (char*)malloc(fullModel.length()+1);
	if(brdModel==NULL)
		brdErr->SimpleMsg(XSErrorFatal,"out of memory!!\n");
	strcpy(brdModel,fullModel.c_str());

	// get information about all the models of XS boards
	int numBoards = GetXSBoardInfo(&brdInfo);

	// use the board model name to find the corresponding index into the list of boards
	for(brdIndex=0; brdIndex<numBoards; brdIndex++)
	{
		if(strcmp(brdModel,brdInfo[brdIndex].brdModel) == 0)
			break;
	}
	if(brdIndex>=numBoards)
	{
		brdErr->SimpleMsg(XSErrorMajor,"unknown type of XS Board!\n");
		return false;
	}

	// scan all the ports
	ScanPorts(err);

	// get pointer to the port for this board
	jtagportFPGA = GetPort(portType,portNum,1);
	if(jtagportFPGA == NULL)
		brdErr->SimpleMsg(XSErrorMajor,"unable to open port to FPGA!\n");

	// return true if no errors occurred during setup
	return err->IsError() ? false:true;
}


//##########################################################################################################
// Look at xsboard.h for a description of the interface.
bool XSCBoard::Configure(const char *fileName)
{
    if(strlen(fileName)==0)
		return true; // nothing to do, so it must be OK

	// Check the suffix of the bitstream file.
	string suffix = GetSuffix(fileName);
	if(suffix=="BIT")
	{
		// Download the configuration bitstream to the FPGA.
		jtagportFPGA->SetPROG(0); // Erase FPGA.
		jtagportFPGA->SetPROG(0); // FPGA erases incompletely unless this is done again.  I don't know why.
		jtagportFPGA->SetPROG(1);
		InsertDelay(1,MILLISECONDS);
		bool status;
		TIME_IT(status = jtagportFPGA->DownloadConfigFile(fileName))
		DEBUG_STMT("Time to download bitstream" << prec_timer << " seconds")
		return status;
	}
	
	// Not a .BIT file, so report the error.
	brdErr->SetSeverity(XSErrorMajor);
	*brdErr << "Only .BIT files can be downloaded into the FPGA on the " << brdModel << " Board\n";
	brdErr->EndMsg();
	return false;
}


//##########################################################################################################
// Look at xsboard.h for a description of the interface.
bool XSCBoard::ConfigureInterface(const char *fileName)
{
	// No interface CPLD on the XSC Board, so always return false.
	return false;
}


//##########################################################################################################
// Look at xsboard.h for a description of the interface.
bool XSCBoard::DownloadRAM(const char *fileName, bool bigEndianBytes, bool bigEndianBits, bool doStart, bool doEnd)
{

	// if there is no RAM interface bitstream file, then this board model does not support RAM downloads
	string msg;
	if(strlen(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].ramBitstreamFile)==0)
	{
		msg = brdModel + (string)" does not support RAM downloads!!\n";
		brdErr->SimpleMsg(XSErrorMajor,msg);
		return false;
	}

	// stop if no hex file was given
    if(strlen(fileName)==0)
	{
		brdErr->SimpleMsg(XSErrorMajor,"no download file was given!\n");
		return false;
	}

	// check the file suffix to see if it is appropriate for downloading to RAM
	string suffix = GetSuffix(fileName);
	if(suffix!="HEX" && suffix!="EXO" && suffix!="XES" && suffix!="MCS")
	{
        string Msg;

        Msg = "Only .HEX, .MCS, .EXO or .XES files can be downloaded into the RAM on the ";
        Msg += brdModel;
        Msg += "!!\n";
        brdErr->SimpleMsg(XSErrorMajor, Msg);
        return false;
	}

	// otherwise open the hex file	
    ifstream is(fileName, ios::binary);
	if(!is || is.fail() || is.eof()!=0)
	{ // error - couldn't open hex file
		brdErr->SetSeverity(XSErrorMajor);
        *brdErr << "Could not open " << fileName << "\n";
		brdErr->EndMsg();
		return false;
	}

	// query the FPGA to see if the RAM interface is already loaded
	Bitstream null(0), instr(0), addr(0), cnt(0), data(0), payload(0);
	// download the USER instruction to the FPGA to enable the JTAG circuitry
	jtagportFPGA->InitTAP();
	jtagportFPGA->GotoTAPState(ShiftIR);
	instr.FromString(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].userInstruction);
	jtagportFPGA->SendRcvBitstream(instr,null);
	// go to the SHIFT-DR state where the RAM interface circuitry can be controlled
	jtagportFPGA->GoThruTAPSequence(UpdateIR,SelectDRScan,CaptureDR,ShiftDR,-1);
	// download the instruction that gets the interface capabilities from the FPGA
	instr.FromString(INSTR_CAPABILITIES);
	jtagportFPGA->SendRcvBitstream(instr,null);
	// readback the capabilities of the interface
	data.Resize(TDO_LENGTH);
	jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
	jtagportFPGA->SendRcvBitstream(null,data);
	// check the capabilities to see if RAM writes are supported
	bool ramIntfcAlreadyLoaded = hasCapability(data,CAPABLE_RAM_WRITE_BIT);

	// only download the RAM interface if this is the first download of data to the RAM.
	// otherwise the interface should already be in place.
	if(doStart && !ramIntfcAlreadyLoaded)
	{
		// configure the FPGA with the RAM interface circuit.
        if(Configure(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].ramBitstreamFile) == false)
		{
			brdErr->SimpleMsg(XSErrorMajor,"error downloading RAM interface circuit!!\n");
			return false;
		}
	}

	// download the USER instruction to the FPGA to enable the JTAG circuitry
	// that will be used to access the RAM
	jtagportFPGA->InitTAP();
	jtagportFPGA->GotoTAPState(ShiftIR);
	instr.FromString(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].userInstruction);
	jtagportFPGA->SendRcvBitstream(instr,null);

	// go to the SHIFT-DR state where the RAM interface circuitry can be controlled
	jtagportFPGA->GoThruTAPSequence(UpdateIR,SelectDRScan,CaptureDR,ShiftDR,-1);

	// download the instruction that gets the RAM organization from the FPGA
	instr.FromString(INSTR_RAM_SIZE);
	jtagportFPGA->SendRcvBitstream(instr,null);

	// readback the sizes of the RAM address and data buses
	data.Resize(TDO_LENGTH);
	jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
	jtagportFPGA->SendRcvBitstream(null,data);
	unsigned char *sizes = data.ToCharString();
	unsigned dataWidth = sizes[0];
	unsigned addrWidth = sizes[1];
	delete [] sizes;

	// stride is the number of byte addresses that are contained in each RAM word address
	unsigned stride = dataWidth / 8; // stride is 1,2,4 for data bus width of 8, 16 or 32
	// address mask zeroes the lower bits of the byte address for alignment to the RAM word size
	unsigned long addrMask = ~(stride - 1);

	DEBUG_STMT(" ")
	DEBUG_STMT("dataWidth = " << dataWidth)
	DEBUG_STMT("addrWidth = " << addrWidth)
	DEBUG_STMT("stride    = " << stride)
	DEBUG_STMT("addrMask  = " << addrMask)
	
	// determine the size of the hex file
	is.seekg(0,ios::end);	// move pointer to end of file
	streampos streamEndPos = is.tellg();	// pointer position = position of end of file
	is.seekg(0,ios::beg);	// return pointer to beginning of file

	// create a progress indicator to track the progress of the download to RAM
	string desc("RAM Download"), subdesc("Downloading "+StripPrefix(fileName));
	Progress *progressGauge = new Progress(brdErr,desc,subdesc,0,streamEndPos);
	assert(progressGauge != NULL);		// make sure progress indicator is initialized
	progressGauge->Report(0);			// start progress indicator at zero

	instr.FromString(INSTR_RAM_WRITE);	// prepare instruction for downloading data to RAM

	// read hex records from the hex file, place contiguous records into a buffer,
	// and download the buffer to RAM when it is filled or a non-contiguous record is encountered.
	unsigned char bytes[DWNLD_BUFFER_SIZE];	// buffer for data bytes from hex records
	unsigned byteIndex = 0;				// index to next free byte in the buffer
	unsigned long loAddr;				// starting byte address of the current hex record 
	unsigned numBytes;					// number of bytes in the current hex record
	unsigned long nextAddr = 0;			// next contiguous address for the next hex record
	unsigned wordAddr;					// address of word in RAM
	unsigned numWords;					// number of words to download
	unsigned char oprnds[16];			// general storage for various operands
	unsigned int i;						// general loop counter
	HexRecord hx;						// holds current hex record
	while(is.eof()==0)
	{
		is >> hx;	// get a hex record from the hex file

		if(is.eof()!=0)
			break; // terminate downloading loop when stream goes dry
		if(is.fail())
		{ // terminate downloading loop when the stream is interrupted
			brdErr->SetSeverity(XSErrorMajor);
			*brdErr << "error reading stream\n";
			brdErr->EndMsg();
			break;
		}
		if(hx.IsError())
		{ // some error in the hex record itself
			brdErr->SetSeverity(XSErrorMajor);
			*brdErr << "hex record: " << hx.GetErrMsg() << "\n";
			brdErr->EndMsg();
			break;
		}
		if(!hx.IsData()) // skip over non-data hex records
			continue;

		// check address and length of current hex record to make sure it aligns with RAM word size
		numBytes = hx.GetLength();
		if(numBytes % stride)
		{
			brdErr->SimpleMsg(XSErrorMajor,"Cannot download an odd number of bytes to multibyte-wide RAM!\n");
			return false;
		}
		loAddr = hx.GetAddress();
		if(loAddr & ~addrMask)
		{
			brdErr->SimpleMsg(XSErrorMajor,"Cannot download to multibyte-wide RAM using an odd byte-starting address!\n");
			return false;
		}
		
		// stop filling the buffer and download it if 1) the address of the current hex record is not
		// contiguous with the preceding hex records or 2) the buffer is full
		if((loAddr != nextAddr && byteIndex > 0) || byteIndex+numBytes > DWNLD_BUFFER_SIZE)
		{
			// store the number of words that will be downloaded to RAM into the download instruction operands
			numWords = byteIndex / stride;
			for(i=0; i<(addrWidth+7)/8; i++,numWords>>=8)
				oprnds[i] = numWords & 0xFF;
			cnt.FromCharString(addrWidth,oprnds);
			
			// send the RAM download instruction and the RAM address and download length  
			payload = cnt + addr + instr;
			jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
			jtagportFPGA->SendRcvBitstream(payload,null);
			
			// now download the data words to RAM
			data.FromCharString(8*byteIndex,bytes);
			jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
			TIME_IT(jtagportFPGA->SendRcvBitstream(data,null))
				DEBUG_STMT("Time to download " << dec << 8*byteIndex << " bits = " << prec_timer << " seconds")
				DEBUG_STMT("Transfer rate = " << dec << 8*byteIndex / prec_timer << " bps")
				
				byteIndex = 0;	// buffer is now empty, so restart at the beginning
		}
		
		// if this is the first record to go in the buffer, then the starting download address
		// is the starting address for the data that will be stored in the buffer
		if(byteIndex==0)
		{
			wordAddr = loAddr / stride; // adjust the byte starting address for the RAM word size
			// partition the word address into bytes and store in the operand storage area
			for(i=0; i<(addrWidth+7)/8; i++,wordAddr>>=8)
				oprnds[i] = wordAddr & 0xFF;
			addr.FromCharString(addrWidth,oprnds);
		}
		
		// PROBLEM:
		//	Lower index of bytes[] points to least-significant byte, but this gets loaded with
		//	the lower index of hx[] which is the most-significant byte.
		// append the data in the current hex record to the end of the buffer
		//			for(int i=0; i<numBytes; i++)
		//				bytes[byteIndex++] = hx[i];
		for(int i=0; i<numBytes; i+=stride)
		{
			if(bigEndianBytes)
			{
				for(int j=stride-1; j>=0; j--)
					bytes[byteIndex++] = bigEndianBits ? reverseByteBits[hx[i+j]] : hx[i+j];
			}
			else // little-endian bytes
			{
				for(int j=0; j<stride; j++)
					bytes[byteIndex++] = bigEndianBits ? reverseByteBits[hx[i+j]] : hx[i+j];
			}
		}
		
		// compute the address for the next contiguous hex record
		nextAddr = loAddr + numBytes;
		
		// show how much of the hex file has been downloaded
		progressGauge->Report(is.tellg());
		
		if(brdErr->IsError())
			break; // terminate downloading loop if some error was reported
	}

	// handle any data remaining in the buffer after the hex file is empty
	if(byteIndex>0)
	{
		// store the number of words that will be downloaded to RAM into the download instruction operands
		numWords = byteIndex / stride;
		for(i=0; i<(addrWidth+7)/8; i++,numWords>>=8)
			oprnds[i] = numWords & 0xFF;
		cnt.FromCharString(addrWidth,oprnds);

		// send the RAM download instruction and the RAM address and download length  
		payload = cnt + addr + instr;
		jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
		jtagportFPGA->SendRcvBitstream(payload,null);

		// now download the data words to RAM
		data.FromCharString(8*byteIndex,bytes);
		jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
		TIME_IT(jtagportFPGA->SendRcvBitstream(data,null))
		DEBUG_STMT("Time to download " << dec << 8*byteIndex << " bits = " << prec_timer << " seconds")
		DEBUG_STMT("Transfer rate = " << dec << 8*byteIndex / prec_timer << " bps")

		// show how much of the hex file has been downloaded
		progressGauge->Report(is.tellg());
	}

	delete progressGauge;
	
	is.close();  // close-up the hex file
	
	return true;
}


//##########################################################################################################
// Look at xsboard.h for a description of the interface.
bool XSCBoard::UploadRAM(const char *fileName, const char* format, unsigned int loAddr, unsigned int hiAddr,
			bool bigEndianBytes, bool bigEndianBits, bool doStart, bool doEnd)			
{
	// if there is no RAM interface bitstream file, then this board model does not support RAM uploads
	string msg;
	if(strlen(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].ramBitstreamFile)==0)
	{
		msg = brdModel + (string)" does not support RAM uploads!!\n";
		brdErr->SimpleMsg(XSErrorMajor,msg);
		return false;
	}

	// stop if no hex file was given
    if(strlen(fileName)==0)
	{
		brdErr->SimpleMsg(XSErrorMajor,"no upload file was given!\n");
		return false;  // stop if no hex file was given
	}

	// otherwise open the hex file	
    ofstream os(fileName);
	if(os.fail() || os.eof()!=0)
	{ // error - couldn't open hex file
		brdErr->SetSeverity(XSErrorMajor);
        *brdErr << "could not open " << fileName << "\n";
		brdErr->EndMsg();
		return false;
	}
	
	// query the FPGA to see if the RAM interface is already loaded
	Bitstream instr(0), null(0), addr(0), cnt(0), data(0), payload(0);
	// download the USER instruction to the FPGA to enable the JTAG circuitry
	jtagportFPGA->InitTAP();
	jtagportFPGA->GotoTAPState(ShiftIR);
	instr.FromString(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].userInstruction);
	jtagportFPGA->SendRcvBitstream(instr,null);
	// go to the SHIFT-DR state where the RAM interface circuitry can be controlled
	jtagportFPGA->GoThruTAPSequence(UpdateIR,SelectDRScan,CaptureDR,ShiftDR,-1);
	// download the instruction that gets the interface capabilities from the FPGA
	instr.FromString(INSTR_CAPABILITIES);
	jtagportFPGA->SendRcvBitstream(instr,null);
	// readback the capabilities of the interface
	data.Resize(TDO_LENGTH);
	jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
	jtagportFPGA->SendRcvBitstream(null,data);
	// check the capabilities to see if RAM reads are supported
	bool ramIntfcAlreadyLoaded = hasCapability(data,CAPABLE_RAM_READ_BIT);

	// only download the RAM interface if this is the first upload of data from the RAM.
	// otherwise the interface should already be in place.
	if(doStart && !ramIntfcAlreadyLoaded)
	{
		// configure the FPGA with the RAM interface circuit.
        if(Configure(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].ramBitstreamFile) == false)
		{
			brdErr->SimpleMsg(XSErrorMajor,"error downloading RAM interface circuit!!\n");
			return false;
		}
	}

	// download the USER instruction to the FPGA to enable the JTAG circuitry
	// that will be used to access the RAM
	jtagportFPGA->InitTAP();
	jtagportFPGA->GotoTAPState(ShiftIR);
	instr.FromString(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].userInstruction);
	jtagportFPGA->SendRcvBitstream(instr,null);

	// go to the SHIFT-DR state where the RAM interface circuitry can be controlled
	jtagportFPGA->GoThruTAPSequence(UpdateIR,SelectDRScan,CaptureDR,ShiftDR,-1);

	// download the instruction that gets the RAM organization from the FPGA
	instr.FromString(INSTR_RAM_SIZE);
	jtagportFPGA->SendRcvBitstream(instr,null);

	// readback the sizes of the RAM address and data buses
	data.Resize(TDO_LENGTH);
	jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
	jtagportFPGA->SendRcvBitstream(null,data);
	unsigned char *sizes = data.ToCharString();
	unsigned dataWidth = sizes[0];
	unsigned addrWidth = sizes[1];
	delete [] sizes;

	// stride is the number of byte addresses that are contained in each RAM word address
	unsigned stride = dataWidth / 8; // stride is 1,2,4 for data bus width of 8, 16 or 32
	// address mask zeroes the lower bits of the byte address for alignment to the RAM word size
	unsigned long addrMask = ~(stride - 1);

	DEBUG_STMT(" ")
	DEBUG_STMT("dataWidth = " << dataWidth)
	DEBUG_STMT("addrWidth = " << addrWidth)
	DEBUG_STMT("stride    = " << stride)
	DEBUG_STMT("addrMask  = " << addrMask)

	// check address and length of the upload byte address range to make sure it aligns with RAM word size
	if((hiAddr-loAddr+1) % stride)
	{
		brdErr->SimpleMsg(XSErrorMajor,"Cannot upload an odd number of bytes from multibyte-wide RAM!\n");
		return false;
	}
	if(loAddr & ~addrMask)
	{
		brdErr->SimpleMsg(XSErrorMajor,"Cannot upload from multibyte-wide RAM using an odd byte-starting address!\n");
		return false;
	}

	// create a progress indicator to track the progress of the upload from RAM
	string desc("RAM Upload"), subdesc("Uploading "+StripPrefix(fileName));
	Progress *progressGauge = new Progress(brdErr,desc,subdesc,loAddr,hiAddr);
	assert(progressGauge != NULL);	// make sure progress indicator is initialized
	progressGauge->Report(loAddr);	// start progress indicator at zero

	instr.FromString(INSTR_RAM_READ);	// prepare instruction for uploading data from RAM

	// read hex records from RAM, divide it into hex records, and place the records in the hex file
	unsigned wordAddr = loAddr / stride;	// address of word in RAM
	unsigned numBytes = hiAddr-loAddr+1;	// number of bytes to upload to the hex file
	unsigned numWords = numBytes / stride;	// number of RAM words to upload
	unsigned char oprnds[16];				// general storage for various operands
	unsigned i;								// general loop counter

	DEBUG_STMT(" ")
	DEBUG_STMT("wordAddr   = " << hex << wordAddr)
	DEBUG_STMT("numBytes   = " << numBytes)
	DEBUG_STMT("numWords   = " << numWords)

	// partition the word address into bytes and store in the operand storage area
	for(i=0; i<(addrWidth+7)/8; i++)
		oprnds[i] = (wordAddr>>(8*i)) & 0xFF;
	addr.FromCharString(addrWidth,oprnds);
	// store the number of words that will be uploaded from RAM into the upload instruction operands
	for(i=0; i<(addrWidth+7)/8; i++)
		oprnds[i] = (numWords>>(8*i)) & 0xFF;
	cnt.FromCharString(addrWidth,oprnds);

	// send the RAM upload instruction and the RAM address and upload length  
	payload = cnt + addr + instr;
	jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
	jtagportFPGA->SendRcvBitstream(payload,null);

	// now upload the data words from RAM
	data.Resize(8*numBytes);
	jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
	TIME_IT(jtagportFPGA->SendRcvBitstream(null,data))
	DEBUG_STMT("Time to upload " << dec << 8*numBytes << " bits = " << prec_timer << " seconds")
	DEBUG_STMT("Transfer rate = " << dec << 8*numBytes / prec_timer << " bps")

	// convert uploaded data from bitstream into a byte array
	unsigned char *bytes = data.ToCharString();
	unsigned byteIndex = 0;		// index to next byte in the array

	// divide the byte array into 16-byte hex records and store them in the hex file
	HexRecord hx;
	hx.Setup(format);
	unsigned long a;
	for(a=loAddr; a<=hiAddr; a=(a+16)&~0xF)
	{
		unsigned long endAddr = (a|0xF);
		if(endAddr>hiAddr)
			endAddr = hiAddr;
		numBytes = endAddr-a+1;
		hx.SetAddress(a);
		hx.SetLength(numBytes);
//		for(i=0; i<numBytes; i++)
//			hx[i] = bytes[byteIndex++];	// PROBLEM: LITTLE ENDIAN BYTES! Least-significant byte is stored at lower address!!
		for(i=0; i<numBytes; i+=stride)
		{
			if(bigEndianBytes)
			{
				for(int j=stride-1; j>= 0; j--)
				{
					hx[i+j] = bytes[byteIndex++];
					if(bigEndianBits)
						hx[i+j] = reverseByteBits[hx[i+j]];
				}
			}
			else // little-endian bytes
			{
				for(int j=0; j<stride; j++)
				{
					hx[i+j] = bytes[byteIndex++];
					if(bigEndianBits)
						hx[i+j] = reverseByteBits[hx[i+j]];
				}
			}
		}
		hx.CalcCheckSum();
		os << hx;		// send hex record to hex file
		progressGauge->Report(a);	// report progress
	}
	delete [] bytes;

	delete progressGauge;

	os.close();  // close-up the hex file

	return true;
}


//##########################################################################################################
// Look at xsboard.h for a description of the interface.
bool XSCBoard::ReadRAM(unsigned int addr, unsigned int* data, bool bigEndianBytes, bool bigEndianBits)	
{
	return false;
}

			
//##########################################################################################################
// Look at xsboard.h for a description of the interface.
bool XSCBoard::WriteRAM(unsigned int addr, unsigned int data, bool bigEndianBytes, bool bigEndianBits)	
{
	return false;
}


//##########################################################################################################
// Look at xsboard.h for a description of the interface.
bool XSCBoard::DownloadFlash(const char *fileName, bool bigEndianBytes, bool bigEndianBits, bool doStart, bool doEnd)
{

	// if there is no Flash interface bitstream file, then this board model does not support Flash downloads
	string msg;
	if(strlen(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].flashBitstreamFile)==0)
	{
		msg = brdModel + (string)" does not support Flash downloads!!\n";
		brdErr->SimpleMsg(XSErrorMajor,msg);
		return false;
	}

	// stop if no hex file was given
    if(strlen(fileName)==0)
	{
		brdErr->SimpleMsg(XSErrorMajor,"no download file was given!\n");
		return false;
	}

	// check the file suffix to see if it is appropriate for downloading to Flash
	string suffix = GetSuffix(fileName);
	if(suffix!="HEX" && suffix!="EXO" && suffix!="XES" && suffix!="MCS")
	{
        string Msg;

        Msg = "Only .HEX, .MCS, .EXO or .XES files can be downloaded into the Flash on the ";
        Msg += brdModel;
        Msg += "!!\n";
        brdErr->SimpleMsg(XSErrorMajor, Msg.c_str());
		return false;
	}

	// otherwise open the hex file	
    ifstream is(fileName, ios::binary);
	if(!is || is.fail() || is.eof()!=0)
	{ // error - couldn't open hex file
		brdErr->SetSeverity(XSErrorMajor);
        *brdErr << "Could not open " << fileName << "\n";
		brdErr->EndMsg();
		return false;
	}

	Bitstream null(0), instr(0), addr(0), cnt(0), data(0), payload(0);

	// download the USER instruction to the FPGA to enable the JTAG circuitry
	jtagportFPGA->InitTAP();
	jtagportFPGA->GotoTAPState(ShiftIR);
	instr.FromString(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].userInstruction);
	jtagportFPGA->SendRcvBitstream(instr,null);
	// go to the SHIFT-DR state where the Flash interface circuitry can be controlled
	jtagportFPGA->GoThruTAPSequence(UpdateIR,SelectDRScan,CaptureDR,ShiftDR,-1);
	// download the instruction that gets the interface capabilities from the FPGA
	instr.FromString(INSTR_CAPABILITIES);
	jtagportFPGA->SendRcvBitstream(instr,null);
	// readback the capabilities of the interface
	data.Resize(TDO_LENGTH);
	jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
	jtagportFPGA->SendRcvBitstream(null,data);
	// check the capabilities to see if Flash writes are supported
	bool flashIntfcAlreadyLoaded = hasCapability(data,CAPABLE_FLASH_WRITE_BIT);

	// only download the Flash interface if this is the first download of data to the Flash.
	// otherwise the interface should already be in place.
	if(doStart && !flashIntfcAlreadyLoaded)
	{
		// configure the FPGA with the Flash interface circuit.
        if(Configure(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].flashBitstreamFile) == false)
		{
			brdErr->SimpleMsg(XSErrorMajor,"error downloading Flash interface circuit!!\n");
			return false;
		}
	}

	// download the USER instruction to the FPGA to enable the JTAG circuitry
	// that will be used to access the Flash
	jtagportFPGA->InitTAP();
	jtagportFPGA->GotoTAPState(ShiftIR);
	instr.FromString(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].userInstruction);
	jtagportFPGA->SendRcvBitstream(instr,null);

	// go to the SHIFT-DR state where the Flash interface circuitry can be controlled
	jtagportFPGA->GoThruTAPSequence(UpdateIR,SelectDRScan,CaptureDR,ShiftDR,-1);

	// download the instruction that gets the Flash organization from the FPGA
	instr.FromString(INSTR_FLASH_SIZE);
	jtagportFPGA->SendRcvBitstream(instr,null);

	// readback the widths of the Flash address and data buses
	data.Resize(24);
	jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
	jtagportFPGA->SendRcvBitstream(null,data);
	unsigned char *sizes = data.ToCharString();
	unsigned dataWidth = sizes[0];
	unsigned addrWidth = sizes[1];
	unsigned blockAddrWidth = sizes[2]; // address width of the block RAM that holds data to be written to Flash 
	unsigned blockSize = 1<<blockAddrWidth;
	unsigned blockAddrMask = ~(blockSize-1);
	delete [] sizes;

	// stride is the number of byte addresses that are contained in each Flash word address
	unsigned stride = dataWidth / 8; // stride is 1,2,4 for data bus width of 8, 16 or 32
	// address mask zeroes the lower bits of the byte address for alignment to the Flash word size
	unsigned long addrMask = ~(stride - 1);

	DEBUG_STMT(" ");
	DEBUG_STMT("dataWidth = " << dataWidth)
	DEBUG_STMT("addrWidth = " << addrWidth)
	DEBUG_STMT("stride    = " << stride)
	DEBUG_STMT("addrMask  = " << addrMask)
	DEBUG_STMT("blockSize = " << blockSize)

	Bitstream opInProgress(0), opPassed(0), opFailed(0);
	opInProgress.FromHexString(TDO_LENGTH,OP_INPROGRESS);
	opPassed.FromHexString(TDO_LENGTH,OP_PASSED);
	opFailed.FromHexString(TDO_LENGTH,OP_FAILED);

	if(doStart)
	{
		// create a progress indicator to track the progress of the Flash erase
		string desc("Flash Erase"), subdesc("Erasing Flash");
		Progress *progressGauge = new Progress(brdErr,desc,subdesc,0,50);  // max erase time = 50 sec
		assert(progressGauge != NULL);		// make sure progress indicator is initialized
		int eraseTime = 0;
		progressGauge->Report(eraseTime);

		// erase the Flash chip
		instr.FromString(INSTR_FLASH_ERASE);
		jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
		jtagportFPGA->SendRcvBitstream(instr,null);
		data.Resize(TDO_LENGTH);
		do
		{
			jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
			jtagportFPGA->SendRcvBitstream(null,data);
			DEBUG_STMT("erase result = " << data)
			InsertDelay(1000,MILLISECONDS);
			eraseTime++;
			progressGauge->Report(eraseTime);
		}while(data == opInProgress);

		delete progressGauge;
	}

	// determine the size of the hex file
	is.seekg(0,ios::end);	// move pointer to end of file
	streampos streamEndPos = is.tellg();	// pointer position = position of end of file
	is.seekg(0,ios::beg);	// return pointer to beginning of file

	// create a progress indicator to track the progress of the download to Flash
	string desc("Flash Download"), subdesc("Downloading "+StripPrefix(fileName));
	Progress *progressGauge = new Progress(brdErr,desc,subdesc,0,streamEndPos);
	assert(progressGauge != NULL);		// make sure progress indicator is initialized
	progressGauge->Report(0);			// start progress indicator at zero

	instr.FromString(INSTR_FLASH_PGM);	// prepare instruction for downloading data to Flash

	// read hex records from the hex file, place contiguous records into a buffer,
	// and download the buffer to block RAM when it is filled or a non-contiguous record is encountered.
	unsigned char bytes[10000];			// buffer for data bytes from hex records
// TODO: handle dynamic allocation of bytes.
//	unsigned char *bytes = new unsigned char(blockSize);		// buffer for data bytes from hex records
//	if(bytes == NULL)
//	{
//		brdErr->SimpleMsg(XSErrorMajor,"unable to allocate byte buffer!\n");
//		return false;
//	}
	unsigned long blockBaseAddr = 0;	// base address of current block
	unsigned long blockTopAddr = blockBaseAddr+blockSize-1; // top address of current block 
	unsigned byteIndex = 0;				// index to next free byte in the buffer and offset into block
	unsigned long loAddr;				// starting byte address of the current hex record 
	unsigned numBytes;					// number of bytes in the current hex record
	unsigned long nextAddr = 0;			// next contiguous address for the next hex record
	unsigned wordAddr;					// address of word in Flash
	unsigned numWords;					// number of words to download
	unsigned char oprnds[16];			// general storage for various operands
	unsigned int i;						// general loop counter
	HexRecord hx;						// holds current hex record
	while(is.eof()==0)
	{
		is >> hx;	// get a hex record from the hex file
		if(is.eof()!=0)
			break; // terminate downloading loop when stream goes dry
		if(is.fail())
		{ // terminate downloading loop when the stream is interrupted
			brdErr->SetSeverity(XSErrorMajor);
			*brdErr << "error reading stream\n";
			brdErr->EndMsg();
			break;
		}
		if(hx.IsError())
		{ // some error in the hex record itself
			brdErr->SetSeverity(XSErrorMajor);
			*brdErr << "hex record: " << hx.GetErrMsg() << "\n";
			brdErr->EndMsg();
			break;
		}
		if(!hx.IsData()) // skip over non-data hex records
			continue;

		// check address and length of current hex record to make sure it aligns with Flash word size
		numBytes = hx.GetLength();
		if(numBytes % stride)
		{
			brdErr->SimpleMsg(XSErrorMajor,"Cannot download an odd number of bytes to multibyte-wide Flash!\n");
			return false;
		}
		loAddr = hx.GetAddress();
		if(loAddr & ~addrMask)
		{
			brdErr->SimpleMsg(XSErrorMajor,"Cannot download to multibyte-wide Flash using an odd byte-starting address!\n");
			return false;
		}
		
		// stop filling the buffer and download it if 1) the address of the current hex record is not
		// contiguous with the preceding hex records or 2) the buffer is full
		if(byteIndex > 0 && (
			loAddr != nextAddr ||
			loAddr > blockTopAddr ||
			(loAddr>=blockBaseAddr && loAddr<=blockTopAddr && loAddr+numBytes-1>blockTopAddr)
			))
		{
			if((loAddr>=blockBaseAddr && loAddr<=blockTopAddr && loAddr+numBytes-1>blockTopAddr))
			{
				int nBytes = blockTopAddr - loAddr + 1;
				int i, j;
				for(i=0; i<nBytes; i++)
					bytes[byteIndex++] = hx[i];
				for(j=0 ; i<numBytes; i++,j++)
					hx[j] = hx[i];
				hx.SetLength(numBytes-nBytes);
				hx.SetAddress(blockTopAddr+1);
				hx.CalcCheckSum();
				numBytes = hx.GetLength();
				loAddr = hx.GetAddress();
			}
			
			// store the number of words that will be downloaded to Flash into the download instruction operands
			numWords = byteIndex / stride;
			for(i=0; i<(addrWidth+7)/8; i++,numWords>>=8)
				oprnds[i] = numWords & 0xFF;
			cnt.FromCharString(addrWidth,oprnds);
			
			// send the Flash download instruction and the Flash address and download length  
			payload = cnt + addr + instr;
			jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
			jtagportFPGA->SendRcvBitstream(payload,null);
			
			// now download the data words to block RAM
			data.FromCharString(8*byteIndex,bytes);
			jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
			TIME_IT(jtagportFPGA->SendRcvBitstream(data,null))
				DEBUG_STMT("Time to download " << dec << 8*byteIndex << " bits = " << prec_timer << " seconds")
				DEBUG_STMT("Transfer rate = " << dec << 8*byteIndex / prec_timer << " bps")
				
				// wait until the block RAM contents are programmed into the Flash
				data.Resize(TDO_LENGTH);
			do
			{
				jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
				jtagportFPGA->SendRcvBitstream(null,data);
				DEBUG_STMT("block write result = " << data)
			}while(data == opInProgress);
			
			byteIndex = 0;	// buffer is now empty, so restart at the beginning
			
			// show how much of the hex file has been downloaded
			progressGauge->Report(is.tellg());
		}
		
		// if this is the first record to go in the buffer, then the starting download address
		// is the starting address for the data that will be stored in the buffer
		if(byteIndex==0)
		{
			wordAddr = loAddr / stride; // adjust the byte starting address for the Flash word size
			// partition the word address into bytes and store in the operand storage area
			for(i=0; i<(addrWidth+7)/8; i++)
				oprnds[i] = (wordAddr>>(8*i)) & 0xFF;
			addr.FromCharString(addrWidth,oprnds);
			blockBaseAddr = loAddr & blockAddrMask;
			blockTopAddr = blockBaseAddr + blockSize - 1;
		}
		
		// append the data in the current hex record to the end of the buffer 
		for(int i=0; i<numBytes; i++)
			bytes[byteIndex++] = hx[i];
		
		// compute the address for the next contiguous hex record
		nextAddr = loAddr + numBytes;
		
		if(brdErr->IsError())
			break; // terminate downloading loop if some error was reported
	}

	// handle any data remaining in the buffer after the hex file is empty
	if(byteIndex>0)
	{
		// store the number of words that will be downloaded to Flash into the download instruction operands
		numWords = byteIndex / stride;
		for(i=0; i<(addrWidth+7)/8; i++,numWords>>=8)
			oprnds[i] = numWords & 0xFF;
		cnt.FromCharString(addrWidth,oprnds);

		// send the Flash download instruction and the Flash address and download length  
		payload = cnt + addr + instr;
		jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
		jtagportFPGA->SendRcvBitstream(payload,null);

		// now download the data words to Flash
		data.FromCharString(8*byteIndex,bytes);
		jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
		TIME_IT(jtagportFPGA->SendRcvBitstream(data,null))
		DEBUG_STMT("Time to download " << dec << 8*byteIndex << " bits = " << prec_timer << " seconds")
		DEBUG_STMT("Transfer rate = " << dec << 8*byteIndex / prec_timer << " bps")
	
		// wait until the block RAM contents are programmed into the Flash
		data.Resize(TDO_LENGTH);
		do
		{
			jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
			jtagportFPGA->SendRcvBitstream(null,data);
			DEBUG_STMT("block write result = " << data)
		}while(data == opInProgress);

		// show how much of the hex file has been downloaded
		progressGauge->Report(is.tellg());
	}
//	delete [] bytes;

	delete progressGauge;
	
	is.close();  // close-up the hex file

	// if this is the last file to be downloaded to Flash, then reprogram the CPLD with a circuit
	// that will make the CPLD load the FPGA with the contents of the Flash upon power-up.
	if(doEnd)
	{
        if(Configure(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].flashConfigBitstreamFile) == false)
		{
			brdErr->SimpleMsg(XSErrorMajor,"Error configuring CPLD with Flash configuration circuit!\n");
			return false;
		}
	}

	return true;
}


//##########################################################################################################
// Look at xsboard.h for a description of the interface.
bool XSCBoard::UploadFlash(const char *fileName, const char* format, unsigned int loAddr, unsigned int hiAddr,
			bool bigEndianBytes, bool bigEndianBits, bool doStart, bool doEnd)				
{

	// if there is no Flash interface bitstream file, then this board model does not support Flash uploads
	string msg;
	if(strlen(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].flashBitstreamFile)==0)
	{
		msg = brdModel + (string)" does not support Flash uploads!!\n";
		brdErr->SimpleMsg(XSErrorMajor,msg);
		return false;
	}

	// stop if no hex file was given
    if(strlen(fileName)==0)
	{
		brdErr->SimpleMsg(XSErrorMajor,"No upload file was given!\n");
		return false;  // stop if no hex file was given
	}

	// otherwise open the hex file	
    ofstream os(fileName);
	if(os.fail() || os.eof()!=0)
	{ // error - couldn't open hex file
		brdErr->SetSeverity(XSErrorMajor);
        *brdErr << "could not open " << fileName << "\n";
		brdErr->EndMsg();
		return false;
	}

	Bitstream null(0), instr(0), addr(0), cnt(0), data(0), payload(0);

	// download the USER instruction to the FPGA to enable the JTAG circuitry
	jtagportFPGA->InitTAP();
	jtagportFPGA->GotoTAPState(ShiftIR);
	instr.FromString(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].userInstruction);
	jtagportFPGA->SendRcvBitstream(instr,null);
	// go to the SHIFT-DR state where the Flash interface circuitry can be controlled
	jtagportFPGA->GoThruTAPSequence(UpdateIR,SelectDRScan,CaptureDR,ShiftDR,-1);
	// download the instruction that gets the interface capabilities from the FPGA
	instr.FromString(INSTR_CAPABILITIES);
	jtagportFPGA->SendRcvBitstream(instr,null);
	// readback the capabilities of the interface
	data.Resize(TDO_LENGTH);
	jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
	jtagportFPGA->SendRcvBitstream(null,data);
	// check the capabilities to see if Flash reads are supported
	bool flashIntfcAlreadyLoaded = hasCapability(data,CAPABLE_FLASH_READ_BIT);

	DEBUG_STMT(" ")
	DEBUG_STMT("CAPABILITIES = " << data)

	// only download the Flash interface if this is the first upload of data from the Flash.
	// Otherwise the interface should already be in place.
	if(doStart && !flashIntfcAlreadyLoaded)
	{
		// configure the FPGA with the Flash interface circuit.
        if(Configure(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].flashBitstreamFile) == false)
		{
			brdErr->SimpleMsg(XSErrorMajor,"Error configuring FPGA with Flash programming circuit!\n");
			return false;
		}
	}

	// download the USER instruction to the FPGA to enable the JTAG circuitry
	// that will be used to access the Flash
	jtagportFPGA->InitTAP();
	jtagportFPGA->GotoTAPState(ShiftIR);
	instr.FromString(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].userInstruction);
	jtagportFPGA->SendRcvBitstream(instr,null);

	// go to the SHIFT-DR state where the Flash interface circuitry can be controlled
	jtagportFPGA->GoThruTAPSequence(UpdateIR,SelectDRScan,CaptureDR,ShiftDR,-1);

	// download the instruction that gets the Flash organization from the FPGA
	instr.FromString(INSTR_FLASH_SIZE);
	jtagportFPGA->SendRcvBitstream(instr,null);

	// readback the widths of the Flash address and data buses
	data.Resize(24);
	jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
	jtagportFPGA->SendRcvBitstream(null,data);
	unsigned char *sizes = data.ToCharString();
	unsigned dataWidth = sizes[0];
	unsigned addrWidth = sizes[1];
	unsigned blockAddrWidth = sizes[2]; // address width of the block RAM that holds data to be written to Flash 
	delete [] sizes;

	// stride is the number of byte addresses that are contained in each Flash word address
	unsigned stride = dataWidth / 8; // stride is 1,2,4 for data bus width of 8, 16 or 32
	// address mask zeroes the lower bits of the byte address for alignment to the Flash word size
	unsigned long addrMask = ~(stride - 1);

	DEBUG_STMT(" ")
	DEBUG_STMT("dataWidth      = " << dataWidth)
	DEBUG_STMT("addrWidth      = " << addrWidth)
	DEBUG_STMT("stride         = " << stride)
	DEBUG_STMT("addrMask       = " << addrMask)
	DEBUG_STMT("blockAddrWidth = " << blockAddrWidth)

	// check address and length of the upload byte address range to make sure it aligns with Flash word size
	if((hiAddr-loAddr+1) % stride)
	{
		brdErr->SimpleMsg(XSErrorMajor,"Cannot upload an odd number of bytes from multibyte-wide Flash!\n");
		return false;
	}
	if(loAddr & ~addrMask)
	{
		brdErr->SimpleMsg(XSErrorMajor,"Cannot upload from multibyte-wide Flash using an odd byte-starting address!\n");
		return false;
	}

	// create a progress indicator to track the progress of the upload from Flash
	string desc("Flash Upload"), subdesc("Uploading "+StripPrefix(fileName));
	Progress *progressGauge = new Progress(brdErr,desc,subdesc,loAddr,hiAddr);
	assert(progressGauge != NULL);	// make sure progress indicator is initialized
	progressGauge->Report(loAddr);	// start progress indicator at zero

	instr.FromString(INSTR_FLASH_READ);	// prepare instruction for uploading data from Flash

	// read hex records from Flash, divide it into hex records, and place the records in the hex file
	unsigned wordAddr = loAddr / stride;	// address of word in Flash
	unsigned numBytes = hiAddr-loAddr+1;	// number of bytes to upload to the hex file
	unsigned numWords = numBytes / stride;	// number of Flash words to upload
	unsigned char oprnds[16];				// general storage for various operands
	unsigned i;								// general loop counter

	DEBUG_STMT(" ")
	DEBUG_STMT("wordAddr   = " << hex << wordAddr)
	DEBUG_STMT("numBytes   = " << numBytes)
	DEBUG_STMT("numWords   = " << numWords)

	// partition the word address into bytes and store in the operand storage area
	for(i=0; i<(addrWidth+7)/8; i++)
		oprnds[i] = (wordAddr>>(8*i)) & 0xFF;
	addr.FromCharString(addrWidth,oprnds);
	// store the number of words that will be uploaded from Flash into the upload instruction operands
	for(i=0; i<(addrWidth+7)/8; i++)
		oprnds[i] = (numWords>>(8*i)) & 0xFF;
	cnt.FromCharString(addrWidth,oprnds);

	// send the Flash upload instruction and the Flash address and upload length  
	payload = cnt + addr + instr;
	jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
	jtagportFPGA->SendRcvBitstream(payload,null);

	// now upload the data words from Flash
	data.Resize(8*numBytes);
	jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
	TIME_IT(jtagportFPGA->SendRcvBitstream(null,data))
	DEBUG_STMT("Time to upload " << dec << 8*numBytes << " bits = " << prec_timer << " seconds")
	DEBUG_STMT("Transfer rate = " << dec << 8*numBytes / prec_timer << " bps")

	// convert uploaded data from bitstream into a byte array
	unsigned char *bytes = data.ToCharString();
	unsigned byteIndex = 0;		// index to next byte in the array

	// divide the byte array into 16-byte hex records and store them in the hex file
	HexRecord hx;
	hx.Setup(format);
	unsigned long address;
	for(address=loAddr; address<=hiAddr; address=(address+16)&~0xF)
	{
		unsigned long endAddr = (address|0xF);
		if(endAddr>hiAddr)
			endAddr = hiAddr;
		numBytes = endAddr-address+1;
		hx.SetAddress(address);
		hx.SetLength(numBytes);
		for(i=0; i<numBytes; i++)
			hx[i] = bytes[byteIndex++];
		hx.CalcCheckSum();
		os << hx;		// send hex record to hex file
		progressGauge->Report(address);	// report progress

	}
	delete [] bytes;

	delete progressGauge;

	os.close();  // close-up the hex file

	// you can only upload a single file from Flash, so reprogram the CPLD with a circuit
	// that will make the CPLD load the FPGA with the contents of the Flash upon power-up.
	// (We are assuming this was the circuit programmed into the CPLD before uploading from the Flash.)
    if(Configure(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].flashConfigBitstreamFile) == false)
	{
		brdErr->SimpleMsg(XSErrorMajor,"Error configuring CPLD with Flash configuration circuit!\n");
		return false;
	}

	return true;
}


//##########################################################################################################
// Look at xsboard.h for a description of the interface.
bool XSCBoard::SetFreq(int div, bool extOscPresent)
{
	string msg = brdModel + (string)" does not have a programmable oscillator!!\n";
	msg += "You will need to divide the clock in your FPGA design.";
	brdErr->SimpleMsg(XSErrorMajor,msg);
	return false;
}


//##########################################################################################################
// Look at xsboard.h for a description of the interface.
bool XSCBoard::SetupAudio(int *reg)
{
	// use general-purpose flags to turn ON or OFF the configuration of the interface to the audio codec.
	// this is useful if we want to do interactive setting of codec registers without incurring bitstream download delays.
	if(!(flags & 1))
	{
		if(strlen(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].oscBitstreamFile)==0)
		{
			string msg = brdModel + (string)" does not support audio codec programming!!\n";
			brdErr->SimpleMsg(XSErrorMajor,msg);
			return false;
		}
		
		// Get the name of the file that contains a bitstream that will configure the CPLD to provide an interface
		// between the parallel port and the audio codec. The audio codec interface is in the same directory as the osc interface file.
		string bitFileName = GetPrefix(string(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].oscBitstreamFile));
		bitFileName += "/";
		bitFileName += "cfgcodec.bit"; // codec interface file name is hard-coded.
		
		// download the codec interface into the FPGA
        if(Configure(bitFileName.c_str())==false)
		{
			brdErr->SimpleMsg(XSErrorMajor,"Error downloading codec setup circuit!!\n");
			return false;
		}
	}

	// download register values into the codec
//	bool status = codec.Configure(reg);
	bool status = false;

	if(!(flags & 1))
	{
		// give the user feedback on the results of configuring the codec interface
		if(status == true)
		{
			string instructions = "The codec of your XSC Board has been configured!!\n";
			PromptUser(instructions,PROMPT_OK);
			return true;
		}
		else
		{
			string instructions = "An error occured while configuring the XSC codec!!\n";
			PromptUser(instructions,PROMPT_OK);
			return false;
		}
	}
	return status;
}


//##########################################################################################################
// Look at xsboard.h for a description of the interface.
bool XSCBoard::SetupVideoIn(string& fileName)
{
	// Get the name of the file that contains a bitstream that will configure the CPLD to provide an interface
	// between the parallel port and the video codec. The video codec interface is in the same directory as the osc interface file.
	string bitFileName = GetPrefix(string(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].oscBitstreamFile));
	bitFileName += "/";
	bitFileName += "cfgi2c.bit";

	// download the codec interface into the FPGA
    if(Configure(bitFileName.c_str())==false)
	{
		brdErr->SimpleMsg(XSErrorMajor,"Error downloading video input setup circuit!!\n");
		return false;
	}


	// download register values into the video codec
//	bool status = videoin.Configure(fileName);
	bool status = false;

	// give the user feedback on the results of configuring the codec interface
	if(status == true)
	{
		string instructions = "The video input of your XSC Board has been configured!!\n";
		PromptUser(instructions,PROMPT_OK);
		return true;
	}
	else
	{
		string instructions = "An error occured while configuring the video input!!\n";
		PromptUser(instructions,PROMPT_OK);
		return false;
	}
}


//##########################################################################################################
// Look at xsboard.h for a description of the interface.
bool XSCBoard::Test(void)
{

	// if there is no diagnostic bitstream file, then this board model does not support diagnostics
	string msg;
	if(strlen(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].testBitstreamFile)==0)
	{
		msg = brdModel + (string)" does not support self-test!!\n";
		brdErr->SimpleMsg(XSErrorMajor,msg);
		return false;
	}

	int try_test_counter = 5;	// number of times to try testing the board if it fails

try_test_loop:

	Bitstream null(0), instr(0), data(0);

	// download the USER instruction to the FPGA to enable the JTAG circuitry
	jtagportFPGA->InitTAP();
	jtagportFPGA->GotoTAPState(ShiftIR);
	instr.FromString(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].userInstruction);
	jtagportFPGA->SendRcvBitstream(instr,null);
	// go to the SHIFT-DR state where the test interface circuitry can be controlled
	jtagportFPGA->GoThruTAPSequence(UpdateIR,SelectDRScan,CaptureDR,ShiftDR,-1);
	// download the instruction that gets the test capabilities from the FPGA
	instr.FromString(INSTR_CAPABILITIES);
	jtagportFPGA->SendRcvBitstream(instr,null);
	// readback the capabilities of the interface
	data.Resize(TDO_LENGTH);
	jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
	jtagportFPGA->SendRcvBitstream(null,data);
	// check the capabilities to see if it supports testing
	bool testIntfcAlreadyLoaded = hasCapability(data,CAPABLE_RUN_DIAG_BIT);

	if(!testIntfcAlreadyLoaded)
	{
		// erase the FPGA in case it is already configured
		//jtagportFPGA->SetPROG(1);
		//jtagportFPGA->SetPROG(0);
		//jtagportFPGA->SetPROG(1);
		//InsertDelay(30,MILLISECONDS);
		
		// download the diagnostic circuit to the board
        if(Configure(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].testBitstreamFile) == false)
		{
			brdErr->SetSeverity(XSErrorMajor);
			*brdErr << "Error downloading " << brdModel << " Board self-test circuit!\n";
			brdErr->EndMsg();
			return false;
		}
	}

	// download the USER instruction to the FPGA to enable the JTAG circuitry
	// that will be used to monitor the progress of the diagnostic
	jtagportFPGA->InitTAP();
	jtagportFPGA->GotoTAPState(ShiftIR);
	instr.FromString(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].userInstruction);
	jtagportFPGA->SendRcvBitstream(instr,null);

	// go to the SHIFT-DR state where the diagnostic circuitry can be controlled
	jtagportFPGA->GoThruTAPSequence(UpdateIR,SelectDRScan,CaptureDR,ShiftDR,-1);

	// download the instruction that initiates the operation of the diagnostic
	instr.FromString(INSTR_RUN_DIAG);
	jtagportFPGA->SendRcvBitstream(instr,null);

	// repeatedly monitor the status of the diagnostic while it sends the "in-progress" code
	Bitstream opInProgress(0), opPassed(0), opFailed(0);
	opInProgress.FromHexString(TDO_LENGTH,OP_INPROGRESS);
	opPassed.FromHexString(TDO_LENGTH,OP_PASSED);
	opFailed.FromHexString(TDO_LENGTH,OP_FAILED);
	data.Resize(TDO_LENGTH);
	do
	{
		jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
		jtagportFPGA->SendRcvBitstream(null,data);
		DEBUG_STMT("diagnostic result = " << data)
		InsertDelay(100,MILLISECONDS);
	}while(data == opInProgress);

	// report whether the diagnostic completed successfully or not
	if(data == opPassed)
	{
		msg = (string)"\nYour " + brdModel + (string)" passed the test!\n";
		PromptUser(msg,PROMPT_OK);
		return true;
	}
	else if(data == opFailed)
	{
		msg = (string)"\nYour " + brdModel + (string)" failed the test!\n\n";
		msg += "Check the following:\n";
		msg += "1) Is your board connected to a USB port?\n";
		msg += "2) Is your board getting power?\n";
		msg += "3) Is your board sitting on a non-conductive surface?\n";
		brdErr->SimpleMsg(XSErrorMajor,msg);
		return false;
	}

	if(try_test_counter > 0)
	{
		try_test_counter--;
		goto try_test_loop;
	}

	// got some strange code from the diagnostic...
	msg = (string)"\nYour " + brdModel + (string)" sent an unknown test diagnostic code!\n";
	brdErr->SimpleMsg(XSErrorMajor,msg);
	return false;
}


//##########################################################################################################
// Look at xsboard.h for a description of the interface.
unsigned char XSCBoard::ApplyTestVectors(unsigned char singleVector, unsigned char mask, unsigned char *vector,
											unsigned char *response, unsigned int numVectors)
{
	return jtagportFPGA->ApplyTestVectors(singleVector,mask,vector,response,numVectors);
}


//##########################################################################################################
// Look at xsboard.h for a description of the interface.
unsigned char XSCBoard::GetTestVector(void)
{
	return jtagportFPGA->GetTestVector();
}


//##########################################################################################################
//##########################################################################################################


//##########################################################################################################
// Look at xsboard.h for a description of the interface.
bool XSCBoard::DownloadRAMFromIntArray(unsigned *intArray, unsigned address, unsigned numInts)			
{
	Bitstream null(0), instr(0), addr(0), cnt(0), data(0), payload(0);

	// download the USER instruction to the FPGA to enable the JTAG circuitry
	// that will be used to access the RAM
	jtagportFPGA->InitTAP();
	jtagportFPGA->GotoTAPState(ShiftIR);
	instr.FromString(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].userInstruction);
	jtagportFPGA->SendRcvBitstream(instr,null);

	// go to the SHIFT-DR state where the RAM interface circuitry can be controlled
	jtagportFPGA->GoThruTAPSequence(UpdateIR,SelectDRScan,CaptureDR,ShiftDR,-1);

	const unsigned addrWidth = 32;
	const unsigned dataWidth = 16;

	unsigned i;
	unsigned long n;
	unsigned char oprnds[5];

	// store the number of words that will be downloaded to RAM into the download instruction operands
	n = numInts;
	for(i=0; i<(addrWidth+7)/8; i++,n>>=8)
		oprnds[i] = n & 0xFF;
	cnt.FromCharString(addrWidth,oprnds);
	
	// store the RAM address into the download instruction operands
	n = address;
	for(i=0; i<(addrWidth+7)/8; i++,n>>=8)
		oprnds[i] = n & 0xFF;
	addr.FromCharString(addrWidth,oprnds);

	instr.FromString(INSTR_RAM_WRITE);	// prepare instruction for downloading data to RAM

	// send the RAM download instruction and the RAM address and download length  
	payload = cnt + addr + instr;
	jtagportFPGA->SendRcvBitstream(payload,null);

	// now download the data words to RAM
	data.FromWordString(16*numInts,intArray);
	jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
	TIME_IT(jtagportFPGA->SendRcvBitstream(data,null))
	DEBUG_STMT("Time to download " << dec << 16*numInts << " bits = " << prec_timer << " seconds")
	DEBUG_STMT("\tTransfer rate = " << dec << 16*numInts / prec_timer << " bps")
	
	return true;
}


//##########################################################################################################
// Look at xsboard.h for a description of the interface.
bool XSCBoard::UploadRAMToIntArray(unsigned *intArray, unsigned address, unsigned numInts)			
{
	Bitstream null(0), instr(0), addr(0), cnt(0), data(0), payload(0);

	// download the USER instruction to the FPGA to enable the JTAG circuitry
	// that will be used to access the RAM
	jtagportFPGA->InitTAP();
	jtagportFPGA->GotoTAPState(ShiftIR);
	instr.FromString(brdInfo[brdIndex].port[PORTTYPE_USBJTAG].userInstruction);
	jtagportFPGA->SendRcvBitstream(instr,null);

	// go to the SHIFT-DR state where the RAM interface circuitry can be controlled
	jtagportFPGA->GoThruTAPSequence(UpdateIR,SelectDRScan,CaptureDR,ShiftDR,-1);

	const unsigned addrWidth = 32;
	const unsigned dataWidth = 16;

	unsigned i;
	unsigned long n;
	unsigned char oprnds[5];

	// store the number of words that will be downloaded to RAM into the upload instruction operands
	n = numInts;
	for(i=0; i<(addrWidth+7)/8; i++,n>>=8)
		oprnds[i] = n & 0xFF;
	cnt.FromCharString(addrWidth,oprnds);
	
	// store the RAM address into the download instruction operands
	n = address;
	for(i=0; i<(addrWidth+7)/8; i++,n>>=8)
		oprnds[i] = n & 0xFF;
	addr.FromCharString(addrWidth,oprnds);

	instr.FromString(INSTR_RAM_READ);	// prepare instruction for uploading data from RAM

	// send the RAM upload instruction and the RAM address and upload length  
	payload = cnt + addr + instr;
	jtagportFPGA->SendRcvBitstream(payload,null);

	// now upload the data words from RAM
	data.Resize(16*numInts);
	jtagportFPGA->GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
	TIME_IT(jtagportFPGA->SendRcvBitstream(null,data))
	DEBUG_STMT("Time to upload " << dec << 16*numInts << " bits = " << prec_timer << " seconds")
	DEBUG_STMT("\tTransfer rate = " << dec << 16*numInts / prec_timer << " bps")

	// convert uploaded data from bitstream into a word array
	data.ToWordString(intArray);

	return true;
}

