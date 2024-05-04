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
#include "flashprt.h"


/// Create a Flash upload/download port.
FlashPort::FlashPort(void)
{
	progressGauge = NULL;
}


/// Create a Flash upload/download port
FlashPort::FlashPort(XSError* e,	///< pointer to error reporting object
				   unsigned int portNum,	///< parallel port number
				   unsigned int invMask,	///< inversion mask for the parallel port
				   unsigned int pos_reset,	///< bit position in parallel port of Flash RESET pin
				   unsigned int pos_clk,	///< bit position in parallel port of Flash CLK pin
				   unsigned int pos_dolsb,	///< bit position in parallel port of LSB of Flash data-out pin
				   unsigned int pos_domsb,	///< bit position in parallel port of MSB of Flash data-out pin
				   unsigned int pos_dilsb,	///< bit position in parallel port of LSB of Flash data-in pin
				   unsigned int pos_dimsb,	///< bit position in parallel port of MSB of Flash data-in pin
				   unsigned int pos_stlsb,	///< bit position in parallel port of LSB of Flash status pin
				   unsigned int pos_stmsb)	///< bit position in parallel port of MSB of Flash status pin
{
	progressGauge = NULL;
	Setup(e,portNum,invMask,pos_reset,pos_clk,
		pos_dolsb,pos_domsb,
		pos_dilsb,pos_dimsb,
		pos_stlsb,pos_stmsb);
}


/// Setup a Flash upload/download port.
int FlashPort::Setup(XSError* e,	///< pointer to error reporting object
				   unsigned int portNum,	///< parallel port number
				   unsigned int invMask,	///< inversion mask for the parallel port
				   unsigned int pos_reset,	///< bit position in parallel port of Flash RESET pin
				   unsigned int pos_clk,	///< bit position in parallel port of Flash CLK pin
				   unsigned int pos_dolsb,	///< bit position in parallel port of LSB of Flash data-out pin
				   unsigned int pos_domsb,	///< bit position in parallel port of MSB of Flash data-out pin
				   unsigned int pos_dilsb,	///< bit position in parallel port of LSB of Flash data-in pin
				   unsigned int pos_dimsb,	///< bit position in parallel port of MSB of Flash data-in pin
				   unsigned int pos_stlsb,	///< bit position in parallel port of LSB of Flash status pin
				   unsigned int pos_stmsb)	///< bit position in parallel port of MSB of Flash status pin
{
	posRESET = pos_reset;
	posCLK   = pos_clk;
	posDOLSB = pos_dolsb;
	posDOMSB = pos_domsb;
	posDILSB = pos_dilsb;
	posDIMSB = pos_dimsb;
	posSTLSB = pos_stlsb;
	posSTMSB = pos_stmsb;
	return PPort::Setup(e,portNum,invMask);
}


/// Download the Flash with the contents of a HEX file.
///\return true if the operation was successful, false otherwise
bool FlashPort::DownloadFlash( string& hexfileName,	///< name of file containing hex data to be programmed into the Flash
					bool bigEndianBytes,	///< if true, data is stored in Flash with most-significant byte at lower address
					bool bigEndianBits,		///< if true, data is stored in Flash with most-significant bit in position 0
					bool doErase)			///< if true, then erase Flash before downloading data from file
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

	string desc("Flash Download"), subdesc("Downloading "+StripPrefix(hexfileName));
	progressGauge = new Progress(&err,desc,subdesc,0,streamEndPos);

	bool status = DownloadFlash(is,bigEndianBytes,bigEndianBits,doErase);
	
	delete progressGauge;
	
	is.close();  // close-up the hex file
	
	return status;
}


/// Download the XS Board Flash with the contents arriving through a stream.
///\return true if the operation was successful, false otherwise
bool FlashPort::DownloadFlash( istream& is,	///< stream through which hex data is received
					bool bigEndianBytes,	///< if true, data is stored in Flash with most-significant byte at lower address
					bool bigEndianBits,		///< if true, data is stored in Flash with most-significant bit in position 0
					bool doErase)			///< if true, then erase Flash before downloading data from file
{
	XSError& err = GetErr();

	if(doErase)
		EraseFlash();	// erase the entire Flash

	assert(progressGauge != NULL);	//	make sure progress indicator is initialized
	progressGauge->Report(0);	// start progress indicator at zero

	// read hex records from file and place data in the Flash of the board
	HexRecord hx;
	while(is.eof()==0)
	{
		is >> hx;
		if(is.eof()!=0)
			break; // terminate loading loop when stream goes dry
		if(is.fail() || err.IsError())
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
			if(DownloadHexRecordToFlash(hx,bigEndianBytes,bigEndianBits) == false)
				return false;
		}
	}
	progressGauge->Report(is.tellg());	// should set gauge to 100%
	
	return err.IsError() ? false:true;
}


/// Load Flash data at the addresses given in a hex record.
///\return true if the operation was successful, false otherwise
bool FlashPort::DownloadHexRecordToFlash(
					HexRecord& hx,			///< hex record with data to be programmed into the Flash
					bool bigEndianBytes,	///< if true, data is stored in Flash with most-significant byte at lower address
					bool bigEndianBits)		///< if true, data is stored in Flash with most-significant bit in position 0
{
	if(hx.IsData()) // don't download unless this is Flash data
	{
		unsigned int i;
		unsigned int address = hx.GetAddress();	// get starting address for hex record
//		for(int j=sizeof(flashBlocks)/sizeof(struct flashBlock)-1; j>=0; j--)
//		{
//			unsigned int addresshi = address + hx.GetLength()-1;
//			if( ((flashBlocks[j].address <= address  ) && (address   < (flashBlocks[j].address+flashBlocks[j].length))) ||
//				((flashBlocks[j].address <= addresshi) && (addresshi < (flashBlocks[j].address+flashBlocks[j].length))) )
//			{
//				if(flashBlocks[j].erased==false)
//					EraseFlashBlock(j);
//			}
//		}
		for(i=0; i<hx.GetLength(); i++)
		{
			if(ProgramFlash(address+i,hx[i],bigEndianBytes,bigEndianBits) == false)
			{
				int testStatus = 0;
				int j;
				for(j=0; j<100; j++)
					testStatus += Test();
				XSError& err = GetErr();
				err.SetSeverity(XSErrorMajor);
				err << "Failure in DownloadHexRecordToFlash at address " << (long int)(address+i) << "\n";
				err << "Parallel port response factor = " << (float)testStatus/j << "\n";
				err.EndMsg();
				return false;
			}
		}
	}
	return true;
}


/// Write a byte to the Flash through the parallel port.
///\return true if the operation was successful, false otherwise
bool FlashPort::WriteFlashByte(unsigned int address,	///< address at which to store data
					unsigned int data,		///< data byte to be stored
					bool bigEndianBytes,	///< if true, data is stored in Flash with most-significant byte at lower address
					bool bigEndianBits)		///< if true, data is stored in Flash with most-significant bit in position 0
{
	Out(1,posRESET,posRESET);	// reset the downloading state machine
	Out(1,posCLK,posCLK);		// force clock high
	Out(0,posRESET,posRESET);	// release the reset

	int status = In(posDILSB,posDIMSB);
//	assert(status==0);
	if(status != 0)
		return false;

	Out((address>>20)&0xf,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);	// latch address bits A23-A20 into downloading circuit
	Out(1,posCLK,posCLK);
	
	Out((address>>16)&0xf,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);	// latch address bits A19-A16 into downloading circuit
	Out(1,posCLK,posCLK);
	
	Out((address>>12)&0xf,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);	// latch address bits A15-A12 into downloading circuit
	Out(1,posCLK,posCLK);
	
	Out((address>> 8)&0xf,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);	// latch address bits A11-A8 into downloading circuit
	Out(1,posCLK,posCLK);
	
	status = In(posDILSB,posDIMSB);
//	assert(status == 1);
	if(status != 1)
		return false;
	
	Out((address>> 4)&0xf,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);	// latch address bits A7-A4 into downloading circuit
	Out(1,posCLK,posCLK);
	
	status = In(posDILSB,posDIMSB);
//	assert(status == 2);
	if(status != 2)
		return false;
	
	Out( address     &0xf,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);		// latch address bits A3-A0 into downloading circuit
	Out(1,posCLK,posCLK);

	status = In(posDILSB,posDIMSB);
//	assert(status == 3);
	if(status != 3)
		return false;

	data = RearrangeData(data,8,bigEndianBytes,bigEndianBits);

	Out((data>>4)&0xf,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);	// latch data bits D7-D4 into downloading circuit
	Out(1,posCLK,posCLK);

	status = In(posDILSB,posDIMSB);
//	assert(status == 4);
	if(status != 4)
		return false;

	Out( data    &0xf,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);	// latch data bits D3-D0 into downloading circuit
	Out(1,posCLK,posCLK);

	status = In(posDILSB,posDIMSB);
//	assert(status == 0);
	if(status != 0)
		return false;
	else
		return true;
}


/// Upload data from Flash and store in a file
///\return true if the operation was successful, false otherwise
bool FlashPort::UploadFlash( string& hexfileName,	///< dump uploaded data to this file
					const char* format,		///< hex file format
					unsigned long loAddr,	///< start fetching data from this address
					unsigned long hiAddr,	///< stop at this address
					bool bigEndianBytes,	///< if true, data is stored in Flash with most-significant byte at lower address
					bool bigEndianBits)		///< if true, data is stored in Flash with most-significant bit in position 0
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
	
	string desc("Flash Upload"), subdesc("Uploading "+(hexfileName));
	progressGauge = new Progress(&err,desc,subdesc,loAddr,hiAddr);
	
	bool status = UploadFlash(os,format,loAddr,hiAddr,bigEndianBytes,bigEndianBits);
	
	delete progressGauge;

	os.close();  // close-up the hex file
	
	return status;
}


/// Upload the XS Board Flash to a stream.
///\return true if the operation was successful, false otherwise
bool FlashPort::UploadFlash( ostream& os,	///< dump uploaded data to this stream
					const char* format,		///< hex file format
					unsigned long loAddr,	///< start fetching data from this address
					unsigned long hiAddr,	///< stop at this address
					bool bigEndianBytes,	///< if true, data is stored in Flash with most-significant byte at lower address
					bool bigEndianBits)		///< if true, data is stored in Flash with most-significant bit in position 0
{
	XSError& err = GetErr();

	assert(progressGauge != NULL);	//	make sure progress indicator is initialized
	progressGauge->Report(0);	// start progress indicator at zero
	
	// read hex records from Flash and place data in the hex file
	HexRecord hx;
	hx.Setup(format);
	unsigned long addr;
	for(addr=loAddr; (addr|0xF)<=hiAddr; addr=(addr+16)&~0xF)
	{
		if(UploadHexRecordFromFlash(hx,addr,addr|0xF,bigEndianBytes,bigEndianBits) == false) // get 16 bytes from RAM
			return false;	// an error occurred while uploading the hex record
		os << hx;		// send hex record to output stream
		progressGauge->Report(addr);	// give feedback on progress
	}

	if(addr<=hiAddr)
	{ // handle the last few bytes of an upload
		if(UploadHexRecordFromFlash(hx,addr,hiAddr,bigEndianBytes,bigEndianBits) == false)
			return false;	// an error occurred while uploading the hex record
		os << hx;		// send hex record to output stream
	}
	progressGauge->Report(hiAddr);	// should set gauge to 100%
	
	return true;
}


/// Get Flash data at the addresses given and store it in a hex record.
///\return true if the operation was successful, false otherwise
bool FlashPort::UploadHexRecordFromFlash( HexRecord& hx,	///< hex record to store data in
					unsigned long loAddr,	///< begin upload from this address
					unsigned long hiAddr,	///< end upload at this address
					bool bigEndianBytes,	///< if true, data is stored in Flash with most-significant byte at lower address
					bool bigEndianBits)		///< if true, data is stored in Flash with most-significant bit in position 0
{
	XSError& err = GetErr();

	hx.SetAddress(loAddr);	// set beginning and ending addresses for the hex record
	hx.SetLength(hiAddr-loAddr+1);

	// process the bytes from a data-type hex record
	ResetFlash();
	unsigned int b;
	for(unsigned long i=loAddr; i<=hiAddr; i++)
	{
		if(ReadFlashByte(i,&b,bigEndianBytes,bigEndianBits) == false)
		{
			int testStatus = 0;
			int j;
			for(j=0; j<100; j++)
				testStatus += Test();
			XSError& err = GetErr();
			err.SetSeverity(XSErrorMajor);
			err << "Failure in UploadHexRecordToFlash at address " << (long int)i << "\n";
			err << "Parallel port response factor = " << (float)testStatus/j << "\n";
			err.EndMsg();
			return false;
		}
		hx[i-loAddr] = RearrangeData(b,8,bigEndianBytes,bigEndianBits);
	}
	
	hx.CalcCheckSum();	// put the checksum into the hex record
	return true;
}


/// Read a byte from the Flash through the parallel port.
///\return true if the operation was successful, false otherwise
bool FlashPort::ReadFlashByte( unsigned int address, ///< address of Flash to be read
					unsigned int* b,		///< pointer to location where data retrieved fromFlash will be stored
					bool bigEndianBytes,	///< if true, data is stored in Flash with most-significant byte at lower address
					bool bigEndianBits)		///< if true, data is stored in Flash with most-significant bit in position 0
{
	Out(1,posRESET,posRESET);	// reset the downloading state machine
	Out(1,posCLK,posCLK);		// force clock high
	Out(0,posRESET,posRESET);	// release the reset

	int status = In(posDILSB,posDIMSB);
//	assert(status==0);
	if(status != 0)
		return false;

	Out((address>>20)&0xf,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);	// latch address bits A23-A20 into downloading circuit
	Out(1,posCLK,posCLK);
	
	Out((address>>16)&0xf,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);	// latch address bits A19-A16 into downloading circuit
	Out(1,posCLK,posCLK);
	
	Out((address>>12)&0xf,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);	// latch address bits A15-A12 into downloading circuit
	Out(1,posCLK,posCLK);
	
	Out((address>> 8)&0xf,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);	// latch address bits A11-A8 into downloading circuit
	Out(1,posCLK,posCLK);

	status = In(posDILSB,posDIMSB);
//	assert(status==1);
	if(status != 1)
		return false;
	
	Out((address>> 4)&0xf,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);	// latch address bits A7-A4 into downloading circuit
	Out(1,posCLK,posCLK);

	status = In(posDILSB,posDIMSB);
//	assert(status==2);
	if(status != 2)
		return false;
	
	Out( address     &0xf,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);		// latch address bits A3-A0 into downloading circuit
	Out(1,posCLK,posCLK);
	Out(0,posCLK,posCLK);

	status = In(posDILSB,posDIMSB);
//	assert(status==3);
	if(status != 3)
		return false;

	Out(1,posRESET,posRESET);	// reset the downloading state machine
	Out(0,posRESET,posRESET);	// release the reset

	status = In(posDILSB,posDIMSB);
//	assert(status==0);
	if(status != 0)
		return false;

	Out((address>>20)&0xf,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);	// latch address bits A23-A20 into downloading circuit
	Out(1,posCLK,posCLK);
	// read upper 3 bits of RAM data from previously loaded address
	unsigned int data = In(posDILSB,posDIMSB);	

	Out((address>>16)&0xf,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);	// latch address bits A19-A16 into downloading circuit
	Out(1,posCLK,posCLK);
	// read middle 3 bits of RAM data from previously loaded address
	data = (data<<3) | (In(posDILSB,posDIMSB) & 0x7);

	Out((address>>12)&0xf,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);	// latch address bits A15-A12 into downloading circuit
	Out(1,posCLK,posCLK);
	// read lowest 2 bits of RAM data from previously loaded address
	data = (data<<2) | (In(posDILSB,posDIMSB) & 0x3);

	*b = RearrangeData(data,8,bigEndianBytes,bigEndianBits);
	return true;
}


/// Test the port that reads/writes the flash to see if it is responding.
///\return zero if the operation was successful, non-zero otherwise
int FlashPort::Test(void)
{
	Out(1,posRESET,posRESET);	// reset the downloading state machine
	Out(1,posCLK,posCLK);		// force clock high
	Out(0,posRESET,posRESET);	// release the reset

	int status = In(posDILSB,posDIMSB);
	if(status != 0)
		return 1;

	Out(0,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);	// latch address bits A23-A20 into downloading circuit
	Out(1,posCLK,posCLK);
	
	Out(0,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);	// latch address bits A19-A16 into downloading circuit
	Out(1,posCLK,posCLK);
	
	Out(0,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);	// latch address bits A15-A12 into downloading circuit
	Out(1,posCLK,posCLK);
	
	Out(0,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);	// latch address bits A11-A8 into downloading circuit
	Out(1,posCLK,posCLK);
	
	status = In(posDILSB,posDIMSB);
	if(status != 1)
		return 2;
	
	Out(0,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);	// latch address bits A7-A4 into downloading circuit
	Out(1,posCLK,posCLK);
	
	status = In(posDILSB,posDIMSB);
	if(status != 2)
		return 3;
	
	Out(0,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);		// latch address bits A3-A0 into downloading circuit
	Out(1,posCLK,posCLK);

	status = In(posDILSB,posDIMSB);
	if(status != 3)
		return 4;

	Out(0,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);	// latch data bits D7-D4 into downloading circuit
	Out(1,posCLK,posCLK);

	status = In(posDILSB,posDIMSB);
	if(status != 4)
		return 5;

	Out(0,posDOLSB,posDOMSB);
	Out(0,posCLK,posCLK);	// latch data bits D3-D0 into downloading circuit
	Out(1,posCLK,posCLK);

	status = In(posDILSB,posDIMSB);
	if(status != 0)
		return 6;
	else
		return 0;  // test successful!!
}
