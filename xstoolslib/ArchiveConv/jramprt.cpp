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

	©1997-2010 - X Engineering Software Systems Corp.
----------------------------------------------------------------------------------*/


#include <fstream>
#include <cassert>

#include "utils.h"
#include "hexrecrd.h"
#include "jramprt.h"


// length of JTAG boundary scan instruction register in XC9572,XC95108 CPLDs
static const unsigned int bsirLength[] = {8,8};

// length of JTAG boundary scan data register in XC9572 CPLD
static const unsigned int bsdrLength[] = {216,324};

// position of cells in the BSDR which affect the address bits of the RAM on the XS95 board
static unsigned int ramAddressBitPos[][numRAMAddressBits] =
{
	// A0 ... A16 for the XC9572
	{132,126,114,111,210,198,117,201,18,12,36,33,21,24,15,90,135},
	// A0 ... A16 for the XC95108
	{252,237,225,219,318,309,222,315,153,3,12,9,18,156,147,93,255},
};

// position of the cells in the BSDR which affect the data bits of the RAM on the XS95 board
static unsigned int ramDataBitPos[][numRAMDataBits] =
{
	// D0 ... D7 for the XC9572
	{48,69,72,78,57,63,66,87},
	// D0 ... D7 for the XC95108
	{57,63,66,72,75,81,84,90},
};

// position of the cells in the BSDR which affect the CS, OE, and WE
// bits of the RAM on the XS95 board
static unsigned int ramControlBitPos[][numRAMControlBits] =
{
	// CS, OE, WE for the XC9572
	{  9,  6,  159},
	// CS, OE, WE for the XC95108
	{135, 144, 138},
};


/// Create a RAM upload/download port.
JTAGRAMPort::JTAGRAMPort(void)
{
	progressGauge = NULL;
	bsirPtr = bsdrPtr = NULL;
}


/// Create a RAM upload/download port.
JTAGRAMPort::JTAGRAMPort(int xc95Type,		///< type of XC9500 CPLD
					XSError* e,				///< pointer to error reporting object
					unsigned int portNum,	///< parallel port number
					unsigned int invMask,	///< inversion mask for the parallel port
					unsigned int pos_tck,	///< bit position in parallel port of TCK pin
					unsigned int pos_tms,	///< bit position in parallel port of TMS pin
					unsigned int pos_tdi,	///< bit position in parallel port of TDI pin
					unsigned int pos_tdo)	///< bit position in parallel port of TDO pin
{
	progressGauge = NULL;
	bsirPtr = bsdrPtr = NULL;
	Setup(xc95Type,e,portNum,invMask,pos_tck,pos_tms,pos_tdi,pos_tdo);
}


/// Initialize the members of the object.
bool JTAGRAMPort::Setup(int xc95Type,		///< type of XC9500 CPLD
					XSError* e,				///< pointer to error reporting object
					unsigned int portNum,	///< parallel port number
					unsigned int invMask,	///< inversion mask for the parallel port
					unsigned int pos_tck,	///< bit position in parallel port of TCK pin
					unsigned int pos_tms,	///< bit position in parallel port of TMS pin
					unsigned int pos_tdi,	///< bit position in parallel port of TDI pin
					unsigned int pos_tdo)	///< bit position in parallel port of TDO pin
{
	if(bsirPtr != NULL)
		delete bsirPtr;
	if(bsdrPtr != NULL)
		delete bsdrPtr;
	bsirPtr = new Bitstream(bsirLength[xc95Type]);
	assert(bsirPtr != NULL);
	bsdrPtr = new XCBSDR(bsdrLength[xc95Type],
						ramAddressBitPos[xc95Type],
						ramDataBitPos[xc95Type],
						ramControlBitPos[xc95Type]);
	assert(bsdrPtr != NULL);
	return LPTJTAG::Setup(e,portNum,invMask,pos_tck,pos_tms,pos_tdi,pos_tdo,0);
}


/// Download the RAM with the contents of a HEX file.
///\return true if operation was successul, false otherwise
bool JTAGRAMPort::DownloadRAM(string& hexfileName,	///< receive data to download from this file
						bool bigEndianBytes,	///< if true, data is stored in RAM with most-significant byte at lower address
						bool bigEndianBits)		///< if true, data is stored in RAM with most-significant bit in position 0
{
	XSError& err = GetErr();
	
	if(hexfileName.length()==0)
		return false;  // stop if no hex file was given
	
	ifstream is(hexfileName.c_str(), ios::binary);  // otherwise open hex file
	if(!is || is.fail() || is.eof()!=0)
	{ // error - couldn't open hex file
		err.SetSeverity(XSErrorMajor);
		err << "could not open " << hexfileName.c_str() << "\n";
		err.EndMsg();
		return false;
	}
	
	// determine the size of the file
	is.seekg(0,ios::end);	// move pointer to end of file
	streampos streamEndPos = is.tellg();	// pointer position = position of end of file
	is.seekg(0,ios::beg);	// return pointer to beginning of file

	string desc("RAM Download"), subdesc("Downloading "+StripPrefix(hexfileName));
	progressGauge = new Progress(&err,desc,subdesc,0,streamEndPos);

	bool status = DownloadRAM(is,bigEndianBytes,bigEndianBits);
	
	delete progressGauge;
	
	is.close();  // close-up the hex file
	
	return status;
}


/// Download the XS Board RAM with the contents arriving through a stream.
///\return true if operation was successul, false otherwise
bool JTAGRAMPort::DownloadRAM(istream& is,	///< receive data to download through this stream
					bool bigEndianBytes,	///< if true, data is stored in RAM with most-significant byte at lower address
					bool bigEndianBits)		///< if true, data is stored in RAM with most-significant bit in position 0
{
	XSError& err = GetErr();

	assert(progressGauge != NULL);	//	make sure progress indicator is initialized
	progressGauge->Report(0);	// start progress indicator at zero

	XCBSDR& bsdr = *bsdrPtr;
	Bitstream& bsir = *bsirPtr;
	Bitstream null(0);
	InitTAP();  // reset the TAP state controller
	// get to a state where the controller will accept an instruction
	GoThruTAPSequence(RunTestIdle,SelectDRScan,SelectIRScan,CaptureIR,ShiftIR, -1);
	// first load an EXTEST instruction into the BSIR
	bsir.Clear();	// all-zeroes is the EXTEST opcode
	SendRcvBitstream(bsir, null);  // after this, TAP state is Exit1-IR
	// activate the EXTEST instruction and get ready to send data to the DR
	GoThruTAPSequence(UpdateIR, SelectDRScan, CaptureDR, ShiftDR, -1);
	
	// enable the RAM
	bsdr.Clear();
	bsdr.SetRAMControls(0,1,1); // enable RAM, disable writing
	SendRcvBitstream(bsdr,null);
	GoThruTAPSequence(UpdateDR,SelectDRScan,CaptureDR,ShiftDR,-1);

	// read hex records from file and place data in the RAM of the board
	while(is.eof()==0)
	{
		HexRecord hx;
		is >> hx;
		if(is.eof()!=0)
			break; // terminate loading loop when stream goes dry
		if(err.IsError())
			break;
		if(is.fail())
		{
			err.SetSeverity(XSErrorMajor);
			err << "error reading stream\n";
			err.EndMsg();
			break;
		}
		if(hx.IsError())
		{ // some error in the hex record itself
			err.SetSeverity(XSErrorMajor);
			err << "hex record: " << hx.GetErrMsg() << "\n";
			err.EndMsg();
			break;
		}
		else
		{ // send out an indication for each hex record that's loaded
			progressGauge->Report(is.tellg());
			DownloadHexRecordToRAM(hx,bigEndianBytes,bigEndianBits);
		}
	}
	progressGauge->Report(is.tellg());	// should set gauge to 100%
	
	bsdr.SetRAMControls(1,1,1); // disable RAM
	SendRcvBitstream(bsdr,null);
	GoThruTAPSequence(UpdateDR,-1);
	
	InitTAP();  // reset the TAP state controller
	
	return err.IsError() ? false:true;
}


/// Upload the XS Board RAM to a file.
///\return true if operation was successul, false otherwise
bool JTAGRAMPort::UploadRAM(string& hexfileName,	///< dump uploaded data to this file.
						const char* format,		///< hex file format.
						unsigned long loAddr,	///< start fetching data from this address.
						unsigned long hiAddr,	///< stop at this address.
						bool bigEndianBytes,	///< if true, data is stored in RAM with most-significant byte at lower address.
						bool bigEndianBits)		///< if true, data is stored in RAM with most-significant bit in position 0.
{
	XSError& err = GetErr();
	
	if(hexfileName.length()==0)
		return false;  // stop if no hex file was given
	
	ofstream os(hexfileName.c_str());  // otherwise open hex file
	if(os.fail() || os.eof()!=0)
	{ // error - couldn't open hex file
		err.SetSeverity(XSErrorMajor);
		err << "could not open " << hexfileName.c_str() << "\n";
		err.EndMsg();
		return false;
	}

	string desc("RAM Upload"), subdesc("Uploading "+StripPrefix(hexfileName));
	progressGauge = new Progress(&err,desc,subdesc,loAddr,hiAddr);
	
	UploadRAM(os,format,loAddr,hiAddr,bigEndianBytes,bigEndianBits);
	
	delete progressGauge;

	os.close();  // close-up the hex file
	
	return true;
}


/// Upload the XS Board RAM to a stream.
///\return true if operation was successul, false otherwise
bool JTAGRAMPort::UploadRAM(ostream& os,		///< dump uploaded data to this stream
						const char* format,		///< hex file format
						unsigned long loAddr,	///< start fetching data from this address
						unsigned long hiAddr,	///< stop at this address
						bool bigEndianBytes,	///< if true, data is stored in RAM with most-significant byte at lower address
						bool bigEndianBits)		///< if true, data is stored in RAM with most-significant bit in position 0
{
	XSError& err = GetErr();

	assert(progressGauge != NULL);	//	make sure progress indicator is initialized
	progressGauge->Report(0);	// start progress indicator at zero

	XCBSDR& bsdr = *bsdrPtr;
	Bitstream& bsir = *bsirPtr;
	Bitstream null(0);
	InitTAP();  // reset the TAP state controller
	// get to a state where the TAP controller will accept an instruction
	GoThruTAPSequence(RunTestIdle,SelectDRScan,SelectIRScan,CaptureIR,ShiftIR, -1);
	// first load an EXTEST instruction into the BSIR
	bsir.Clear();	// all-zeroes is the EXTEST opcode
	SendRcvBitstream(bsir, null);  // after this, TAP state is Exit1-IR
	// activate the EXTEST instruction and get ready to send data to the DR
	GoThruTAPSequence(UpdateIR, SelectDRScan, CaptureDR, ShiftDR, -1);
	
	// enable the RAM so data can be read out of it
	bsdr.Clear();
	bsdr.SetRAMControls(0,1,1); // enable RAM, disable writing
	SendRcvBitstream(bsdr,null);
	// transfer TAP to ShiftDR state so RAM data can be extracted via JTAG
	GoThruTAPSequence(UpdateDR,SelectDRScan,CaptureDR,ShiftDR,-1);
	
	// read hex records from RAM and place data in the hex file
	HexRecord hx;
	hx.Setup(format);
	unsigned long addr;
	for(addr=loAddr; (addr|0xF)<=hiAddr; addr=(addr+16)&~0xF)
	{
		if(UploadHexRecordFromRAM(hx,addr,addr|0xF,bigEndianBytes,bigEndianBits) == false) // get 16 bytes from RAM
			return false;	// an error occurred while uploading the hex record
		os << hx;		// send hex record to output stream
		progressGauge->Report(addr);	// give feedback on progress
	}
	if(addr<=hiAddr)
	{ // handle the last few bytes of an upload
		if(UploadHexRecordFromRAM(hx,addr,hiAddr,bigEndianBytes,bigEndianBits) == false)
			return false;	// an error occurred while uploading the hex record
		os << hx;		// send hex record to output stream
	}
	progressGauge->Report(hiAddr);	// should set gauge to 100%
	
	bsdr.SetRAMControls(1,1,1); // disable RAM
	SendRcvBitstream(bsdr,null);
	GoThruTAPSequence(UpdateDR,-1);
	
	InitTAP();  // reset the TAP state controller
	
	return true;
}


/// Load RAM data at the addresses given in a hex record.
///\return true if operation was successul, false otherwise
bool JTAGRAMPort::DownloadHexRecordToRAM(HexRecord& hx,	///< hex record with data to be downloaded
					bool bigEndianBytes,	///< if true, data is stored in RAM with most-significant byte at lower address
					bool bigEndianBits)		///< if true, data is stored in RAM with most-significant bit in position 0
{
	if(!hx.IsData()) // don't download unless this is RAM data
		return true;
	
	// process the bytes from a data-type hex record
	for(unsigned int i=0; i<hx.GetLength(); i++)
	{
		XCBSDR& bsdr = *bsdrPtr;
		Bitstream null(0);
		
		// load address and data onto the FPLD pins
		bsdr.Clear();	// set all the BSDR bits to zero
		bsdr.SetRAMAddress(hx.GetAddress()+i);
		bsdr.SetRAMData(RearrangeData(hx[i],8,bigEndianBytes,bigEndianBits));
		bsdr.SetRAMControls(0,1,1); // enable RAM, disable outputs, raise write line
		SendRcvBitstream(bsdr,null);
		GoThruTAPSequence(UpdateDR,SelectDRScan,CaptureDR,ShiftDR,-1);
		
		bsdr.SetRAMControls(0,1,0); // bring write enable low
		SendRcvBitstream(bsdr,null);
		GoThruTAPSequence(UpdateDR,SelectDRScan,CaptureDR,ShiftDR,-1);
		
		bsdr.SetRAMControls(0,1,1); // bring write enable high again
		SendRcvBitstream(bsdr,null);
		GoThruTAPSequence(UpdateDR,SelectDRScan,CaptureDR,ShiftDR,-1);
	}
	return true;
}


/// Write contents of a single location in RAM.
///\return true if operation was successul, false otherwise
bool JTAGRAMPort::WriteRAM(unsigned int addr,	///< address of RAM location
						unsigned int data,		///< data to be written into RAM location
						bool bigEndianBytes,	///< if true, data is stored in RAM with most-significant byte at lower address
						bool bigEndianBits)		///< if true, data is stored in RAM with most-significant bit in position 0
{
	XSError& err = GetErr();

	XCBSDR& bsdr = *bsdrPtr;
	Bitstream& bsir = *bsirPtr;
	Bitstream null(0);

	InitTAP();  // reset the TAP state controller
	// get to a state where the controller will accept an instruction
	GoThruTAPSequence(RunTestIdle,SelectDRScan,SelectIRScan,CaptureIR,ShiftIR, -1);
	// first load an EXTEST instruction into the BSIR
	bsir.Clear();	// all-zeroes is the EXTEST opcode
	SendRcvBitstream(bsir, null);  // after this, TAP state is Exit1-IR
	// activate the EXTEST instruction and get ready to send data to the DR
	GoThruTAPSequence(UpdateIR, SelectDRScan, CaptureDR, ShiftDR, -1);
	
	// load address and data onto the FPLD pins
	bsdr.Clear();	// set all the BSDR bits to zero
	bsdr.SetRAMAddress(addr);
	bsdr.SetRAMData(RearrangeData(data,8,bigEndianBytes,bigEndianBits));
	bsdr.SetRAMControls(0,1,1); // enable RAM, disable outputs, raise write line
	SendRcvBitstream(bsdr,null);
	GoThruTAPSequence(UpdateDR,SelectDRScan,CaptureDR,ShiftDR,-1);
	
	bsdr.SetRAMControls(0,1,0); // bring write enable low
	SendRcvBitstream(bsdr,null);
	GoThruTAPSequence(UpdateDR,SelectDRScan,CaptureDR,ShiftDR,-1);
	
	bsdr.SetRAMControls(0,1,1); // bring write enable high again
	SendRcvBitstream(bsdr,null);
	GoThruTAPSequence(UpdateDR,SelectDRScan,CaptureDR,ShiftDR,-1);

	bsdr.SetRAMControls(1,1,1); // disable RAM
	SendRcvBitstream(bsdr,null);
	GoThruTAPSequence(UpdateDR,-1);
	
	InitTAP();  // reset the TAP state controller

	return true;
}


/// Get RAM data at the addresses given and store it in a hex record.
///\return true if operation was successul, false otherwise
bool JTAGRAMPort::UploadHexRecordFromRAM(HexRecord& hx,	///< hex record to store data in
						unsigned long loAddr,	///< begin upload from this address (inclusive)
						unsigned long hiAddr,	///< end upload at this address (inclusive)
						bool bigEndianBytes,	///< if true, data is stored in RAM with most-significant byte at lower address
						bool bigEndianBits)		///< if true, data is stored in RAM with most-significant bit in position 0
{
	XSError& err = GetErr();
	
	hx.SetAddress(loAddr);
	hx.SetLength(hiAddr-loAddr+1);
	
	XCBSDR& bsdrOut = *bsdrPtr;
	XCBSDR& bsdrIn = *bsdrPtr;
	Bitstream null(0);
	
	// process the bytes from a data-type hex record
	for(unsigned long i=loAddr; i<=hiAddr; i++)
	{
		// load address and data onto the FPLD pins
		bsdrOut.Clear();
		bsdrOut.SetRAMAddress(i);
		bsdrOut.ReadRAMData();  // tristate FPLD data lines so RAM has control
		bsdrOut.SetRAMControls(0,0,1); // enable RAM, enable outputs, keep write high
		SendRcvBitstream(bsdrOut,null);
		GoThruTAPSequence(UpdateDR,SelectDRScan,CaptureDR,ShiftDR,-1);
		
		bsdrOut.SetRAMControls(0,1,1); // disable RAM outputs
		SendRcvBitstream(bsdrOut,bsdrIn); // shift out RAM data while loading new data
		unsigned long drivenAddress = bsdrIn.GetRAMAddress();
		if(drivenAddress != i)
		{
			err.SetSeverity(XSErrorMajor);
			err << "specified memory address = " << (long int)i
				<< " but actual address = " << (long int)drivenAddress << "\n";
			err.EndMsg();
			return false;
		}
		else
			hx[i-loAddr] = RearrangeData(bsdrIn.GetRAMData(),8,bigEndianBytes,bigEndianBits);
		
		GoThruTAPSequence(UpdateDR,SelectDRScan,CaptureDR,ShiftDR,-1);
	}
	hx.CalcCheckSum();
	return true;
}


/// Read contents of a single location in RAM.
///\return true if operation was successul, false otherwise
bool JTAGRAMPort::ReadRAM(unsigned int addr,	///< address of RAM location
					unsigned int* data,		///< data returned from RAM location
					bool bigEndianBytes,	///< if true, data is stored in RAM with most-significant byte at lower address
					bool bigEndianBits)		///< if true, data is stored in RAM with most-significant bit in position 0
{
	XSError& err = GetErr();
	
	XCBSDR& bsdr = *bsdrPtr;
	Bitstream& bsir = *bsirPtr;
	Bitstream null(0);
	
	InitTAP();  // reset the TAP state controller
	// get to a state where the TAP controller will accept an instruction
	GoThruTAPSequence(RunTestIdle,SelectDRScan,SelectIRScan,CaptureIR,ShiftIR, -1);
	// first load an EXTEST instruction into the BSIR
	bsir.Clear();	// all-zeroes is the EXTEST opcode
	SendRcvBitstream(bsir, null);  // after this, TAP state is Exit1-IR
	// activate the EXTEST instruction and get ready to send data to the DR
	GoThruTAPSequence(UpdateIR, SelectDRScan, CaptureDR, ShiftDR, -1);

	// load address and data onto the FPLD pins
	bsdr.Clear();
	bsdr.SetRAMAddress(addr);
	bsdr.ReadRAMData();  // tristate FPLD data lines so RAM has control
	bsdr.SetRAMControls(0,0,1); // enable RAM, enable outputs, keep write high
	SendRcvBitstream(bsdr,null);
	GoThruTAPSequence(UpdateDR,SelectDRScan,CaptureDR,ShiftDR,-1);
	
	bsdr.SetRAMControls(0,1,1); // disable RAM outputs
	SendRcvBitstream(bsdr,bsdr); // shift out RAM data while loading new data
	unsigned long drivenAddress = bsdr.GetRAMAddress();
	if(drivenAddress != addr)
	{
		err.SetSeverity(XSErrorMajor);
		err << "specified memory address = " << (long int)addr
			<< " but actual address = " << (long int)drivenAddress << "\n";
		err.EndMsg();
		return false;
	}
	else
		*data = RearrangeData(bsdr.GetRAMData(),8,bigEndianBytes,bigEndianBits);
	GoThruTAPSequence(UpdateDR,SelectDRScan,CaptureDR,ShiftDR,-1);
	
	bsdr.SetRAMControls(1,1,1); // disable RAM
	SendRcvBitstream(bsdr,null);
	GoThruTAPSequence(UpdateDR,-1);
	
	InitTAP();  // reset the TAP state controller

	return true;
}
