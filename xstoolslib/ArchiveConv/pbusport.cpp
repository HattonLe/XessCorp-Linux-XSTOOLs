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


#include <cassert>
#include <fstream>

#include "utils.h"
#include "hexrecrd.h"
#include "pbusport.h"


/// Create a peripheral bus upload/download port.
PBusPort::PBusPort(void)
{
	;
}


/// Create a peripheral bus upload/download port.
PBusPort::PBusPort(XSError* e,	///< pointer to error reporting object
				   unsigned int portNum,	///< parallel port number
				   unsigned int invMask,	///< inversion mask for the parallel port
				   unsigned int pos_reset,	///< bit position in parallel port of RESET pin
				   unsigned int pos_clk,	///< bit position in parallel port of CLK pin
				   unsigned int pos_dolsb,	///< bit position in parallel port of LSB of data-out pin
				   unsigned int pos_domsb,	///< bit position in parallel port of MSB of data-out pin
				   unsigned int pos_dilsb,	///< bit position in parallel port of LSB of data-in pin
				   unsigned int pos_dimsb,	///< bit position in parallel port of MSB of data-in pin
				   unsigned int pos_stlsb,	///< bit position in parallel port of LSB of status pin
				   unsigned int pos_stmsb,	///< bit position in parallel port of MSB of status pin
				   unsigned int dWidth,		///< width of data word
				   unsigned int aWidth)		///< width of address
{
	Setup(e,portNum,invMask,pos_reset,pos_clk,
		pos_dolsb,pos_domsb,
		pos_dilsb,pos_dimsb,
		pos_stlsb,pos_stmsb,
		dWidth,aWidth);
}


/// Initialize the object.
int PBusPort::Setup(XSError* e,	///< pointer to error reporting object
				   unsigned int portNum,	///< parallel port number
				   unsigned int invMask,	///< inversion mask for the parallel port
				   unsigned int pos_reset,	///< bit position in parallel port of RESET pin
				   unsigned int pos_clk,	///< bit position in parallel port of CLK pin
				   unsigned int pos_dolsb,	///< bit position in parallel port of LSB of data-out pin
				   unsigned int pos_domsb,	///< bit position in parallel port of MSB of data-out pin
				   unsigned int pos_dilsb,	///< bit position in parallel port of LSB of data-in pin
				   unsigned int pos_dimsb,	///< bit position in parallel port of MSB of data-in pin
				   unsigned int pos_stlsb,	///< bit position in parallel port of LSB of status pin
				   unsigned int pos_stmsb,	///< bit position in parallel port of MSB of status pin
				   unsigned int dWidth,		///< width of data word
				   unsigned int aWidth)		///< width of address
{
	progressGauge = NULL;
	posRESET = pos_reset;
	posCLK   = pos_clk;
	posDOLSB = pos_dolsb;
	posDOMSB = pos_domsb;
	posDILSB = pos_dilsb;
	posDIMSB = pos_dimsb;
	posSTLSB = pos_stlsb;
	posSTMSB = pos_stmsb;
	dataWidth = dWidth;
	addrWidth = aWidth;
	return PPort::Setup(e,portNum,invMask);
}


/// Download the peripheral bus with the contents of a file.
///\return true if operation was successful, false if not
bool PBusPort::Download(string& hexfileName,	///< file containing data for the device on the peripheral bus
				bool bigEndianBytes,	///< if true, data is stored in Flash with most-significant byte at lower address
				bool bigEndianBits)		///< if true, data is stored in Flash with most-significant bit in position 0
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

	bool status = Download(is,bigEndianBytes,bigEndianBits);
	
	delete progressGauge;
	
	is.close();  // close-up the hex file
	
	return status;
}


/// Download the peripheral bus with the contents arriving through a stream.
///\return true if operation was successful, false if not
bool PBusPort::Download(istream& is,	///< input stream that delivers data to be sent to a device on the peripheral bus
				bool bigEndianBytes,	///< if true, data is stored in Flash with most-significant byte at lower address
				bool bigEndianBits)		///< if true, data is stored in Flash with most-significant bit in position 0
{
	XSError& err = GetErr();

	assert(progressGauge != NULL);	//	make sure progress indicator is initialized
	progressGauge->Report(0);		// start progress indicator at zero

	// read hex records from file and place data on the peripheral bus of the board
	HexRecord hx;
	while(is.eof()==0)
	{
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
			if(DownloadHexRecord(hx,bigEndianBytes,bigEndianBits) == false) // download hex record
				return false;	// an error occurred while downloading the hex record
		}
	}
	progressGauge->Report(is.tellg());	// should set gauge to 100%
	
	return err.IsError() ? false:true;
}


/// Upload the peripheral bus to a file.
///\return true if operation was successful, false if not
bool PBusPort::Upload(string& hexfileName,	///< dump uploaded data to this file
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
	
	string desc("RAM Upload"), subdesc("Uploading "+StripPrefix(hexfileName));
	progressGauge = new Progress(&err,desc,subdesc,loAddr,hiAddr);
	
	bool status = Upload(os,format,loAddr,hiAddr,bigEndianBytes,bigEndianBits);
	
	delete progressGauge;

	os.close();  // close-up the hex file

	return status;
}


/// Upload the peripheral bus to a stream.
///\return true if operation was successful, false if not
bool PBusPort::Upload(ostream& os,		///< dump uploaded data to this stream
				const char* format,		///< hex file format
				unsigned long loAddr,	///< start fetching data from this address
				unsigned long hiAddr,	///< stop at this address
				bool bigEndianBytes,	///< if true, data is stored in Flash with most-significant byte at lower address
				bool bigEndianBits)		///< if true, data is stored in Flash with most-significant bit in position 0
{
	XSError& err = GetErr();

	assert(progressGauge != NULL);	//	make sure progress indicator is initialized
	progressGauge->Report(0);		// start progress indicator at zero
	
	// read hex records from the peripheral bus and place data in the hex file
	HexRecord hx;
	hx.Setup(format);
	unsigned long addr;
	for(addr=loAddr; (addr|0xF)<=hiAddr; addr=(addr+16)&~0xF)
	{
		if(UploadHexRecord(hx,addr,addr|0xF,bigEndianBytes,bigEndianBits) == false) // get 16 bytes
			return false;	// an error occurred while uploading the hex record
		os << hx;		// send hex record to output stream
		progressGauge->Report(addr);	// give feedback on progress
	}
	if(addr<=hiAddr)
	{ // handle the last few bytes of an upload
		if(UploadHexRecord(hx,addr,hiAddr,bigEndianBytes,bigEndianBits) == false) // get the last few bytes
			return false;	// an error occurred while uploading the hex record
		os << hx;		// send hex record to output stream
	}
	progressGauge->Report(hiAddr);	// should set gauge to 100%
	
	return true;
}


/// Load peripheral bus at the addresses given in a hex record.
///\return true if operation was successful, false if not
bool PBusPort::DownloadHexRecord(HexRecord& hx,	///< hex record containing data to load into device on peripheral bus
				bool bigEndianBytes,	///< if true, data is stored in Flash with most-significant byte at lower address
				bool bigEndianBits)		///< if true, data is stored in Flash with most-significant bit in position 0
{
	XSError& err = GetErr();

	if(!hx.IsData())	// don't download unless this is data
		return true;	// no error, just didn't do anything

	// the address has to be right-shifted by one bit for each doubling of the data width.
	unsigned int addrScale;
	if(dataWidth<=8)
		addrScale = 0;	// addresses for byte-wide data need no scaling because each byte fits in a hex record byte
	else if(dataWidth<=16)
		addrScale = 1;	// addresses for 16-bit data are halved because each word takes two bytes in the hex record
	else if(dataWidth<=32)
		addrScale = 2;	// addresses for 32-bit data are quartered because each word takes four bytes in the hex record
	else
		assert(1==0);	// we can't handle data widths > 32 bits
	unsigned int stride = 1 << addrScale;

	unsigned int addrMask = ~((1 << addrScale) - 1);

	if(hx.GetLength() % stride)
	{
		string msg("Cannot download an odd number of bytes to word-wide bus!\n");
		err.SimpleMsg(XSErrorMajor,msg);
		return false;
	}
	unsigned int loAddr = hx.GetAddress();
	if(loAddr & ~addrMask)
	{
		string msg("Cannot download to word-wide bus using an odd byte-starting address!\n");
		err.SimpleMsg(XSErrorMajor,msg);
		return false;
	}
	
	// load the bytes from a data-type hex record onto the peripheral bus
	for(unsigned int i=0; i<hx.GetLength(); i+=stride)
	{
		unsigned int data = 0;	// assemble next data word from hex record
		for(int j=0; j<stride; j++)
			data = (data<<8) | (hx[i+j] & 0xFF);
		if(Write((loAddr+i)>>addrScale,data,bigEndianBytes,bigEndianBits,i==0) == false)
			return false;
	}

	return true;
}


/// Write contents of a single location on the peripheral bus.
///\return true if operation was successful, false if not
bool PBusPort::Write(unsigned int addr,		///< address of RAM location
			unsigned int data,		///< data to be written into RAM location
			bool bigEndianBytes,	///< if true, data is stored in Flash with most-significant byte at lower address
			bool bigEndianBits,		///< if true, data is stored in Flash with most-significant bit in position 0
			bool loadAddress)		///< true if address has to be loaded first
{
	XSError& err = GetErr();

	unsigned int statusPortWidth = posSTMSB - posSTLSB + 1;
	unsigned int inDataPortWidth = posDIMSB - posDILSB + 1;
	unsigned int outDataPortWidth = posDOMSB - posDOLSB + 1;
	unsigned int statusMask = (1<<statusPortWidth) - 1;
	unsigned int inDataMask = (1<<inDataPortWidth) - 1;
	unsigned int outDataMask = (1<<outDataPortWidth) - 1;


	unsigned int statusChk = 0;	// set status check to reset state id
	if(loadAddress)
	{
		Out(1,posRESET,posRESET);	// reset the downloading state machine
		Out(0,posRESET,posRESET);	// release the reset
		
		// set initial address into the downloading state machine
		addr |= (1<<27);	// set upper bit of address to signal a write operation
		for(int j=0; j<addrWidth; j+=outDataPortWidth)
		{
			unsigned int status = In(posSTLSB,posSTMSB);
			assert(status==(statusChk & statusMask));
			if(status != (statusChk & statusMask))
				return false;	// error - peripheral bus interface state machine not in the right state
			Out((addr>>j)&outDataMask,posDOLSB,posDOMSB);
			Out(0,posCLK,posCLK);
			Out(1,posCLK,posCLK);	// latch address bits into downloading circuit
			statusChk++;			// increment status check
		}
	}

	statusChk = 5;
	data  = RearrangeData(data,dataWidth,bigEndianBytes,bigEndianBits);
	for(int j=0; j<dataWidth; j+=outDataPortWidth)
	{
		unsigned int status = In(posSTLSB,posSTMSB);
		assert(status==(statusChk & statusMask));
		if(status != (statusChk & statusMask))
			return false;	// error - peripheral bus interface state machine not in the right state
		Out((data>>j)&outDataMask,posDOLSB,posDOMSB);	// send data bits to register
		Out(0,posCLK,posCLK);
		Out(1,posCLK,posCLK);	// store data bits into data register
		statusChk--;			// decrement status check
	}

	Out(0,posCLK,posCLK);
	Out(1,posCLK,posCLK);	// latch data from register into RAM

	return true;
}


/// Get data from the peripheral bus at the addresses given and store it in a hex record.
///\return true if operation was successful, false if not
bool PBusPort::UploadHexRecord(HexRecord& hx,	///< hex record to store data in
				unsigned long loAddr,	///< begin upload from this address
				unsigned long hiAddr,	///< end upload at this address
				bool bigEndianBytes,	///< if true, data is stored in Flash with most-significant byte at lower address
				bool bigEndianBits)		///< if true, data is stored in Flash with most-significant bit in position 0
{
	XSError& err = GetErr();

	// the address has to be right-shifted by one bit for each doubling of the data width.
	unsigned int addrScale;
	if(dataWidth<=8)
		addrScale = 0;	// addresses for byte-wide data need no scaling because each byte fits in a hex record byte
	else if(dataWidth<=16)
		addrScale = 1;	// addresses for 16-bit data are halved because each word takes two bytes in the hex record
	else if(dataWidth<=32)
		addrScale = 2;	// addresses for 32-bit data are quartered because each word takes four bytes in the hex record
	else
		assert(1==0);	// we can't handle data widths > 32 bits
	unsigned int stride = 1 << addrScale;

	unsigned int addrMask = ~((1 << addrScale) - 1);

	if((hiAddr-loAddr+1) % stride)
	{
		string msg("Cannot upload an odd number of bytes from word-wide bus!\n");
		err.SimpleMsg(XSErrorMajor,msg);
		return false;
	}
	if(loAddr & ~addrMask)
	{
		string msg("Cannot upload from word-wide bus using an odd byte-starting address!\n");
		err.SimpleMsg(XSErrorMajor,msg);
		return false;
	}

	// clear the lower bits of the address bounds
	loAddr = loAddr & addrMask;
	hiAddr = hiAddr & addrMask;

	hx.SetAddress(loAddr);	// set beginning and ending addresses for the hex record
	hx.SetLength(hiAddr-loAddr+stride);

	// process the bytes from a data-type hex record.
	for(unsigned long i=loAddr; i<=hiAddr; i+=stride)
	{
		unsigned int data = 0;
		if(Read(i>>addrScale,&data,bigEndianBytes,bigEndianBits,i==loAddr) == false)
			return false;
		Data2Hex(data,hx,i-loAddr,dataWidth>>3,BIG_ENDIAN_BYTES,LITTLE_ENDIAN_BITS);
	}
	
	hx.CalcCheckSum();	// put the checksum into the hex record

	return true;
}


/// Read contents of a single location on the peripheral bus.
///\return true if operation was successful, false if not
bool PBusPort::Read(unsigned int addr,		///< address of peripheral bus location
		unsigned int* data,		///< data returned from RAM location
		bool bigEndianBytes,	///< if true, data is stored in Flash with most-significant byte at lower address
		bool bigEndianBits,		///< if true, data is stored in Flash with most-significant bit in position 0
		bool loadAddress)		///< true if address has to be loaded first
{
	XSError& err = GetErr();

	unsigned int statusPortWidth = posSTMSB - posSTLSB + 1;
	unsigned int inDataPortWidth = posDIMSB - posDILSB + 1;
	unsigned int outDataPortWidth = posDOMSB - posDOLSB + 1;
	unsigned int statusMask = (1<<statusPortWidth) - 1;
	unsigned int inDataMask = (1<<inDataPortWidth) - 1;
	unsigned int outDataMask = (1<<outDataPortWidth) - 1;

	if(loadAddress)
	{
		Out(1,posRESET,posRESET);	// reset the downloading state machine
		Out(0,posRESET,posRESET);	// release the reset
		
		// set initial address into the downloading state machine
		addr &= ~(1<<27);	// clear upper bit of address to signal a read operation
		unsigned int statusChk = 0;	// set status check to reset state id
		for(int j=0; j<addrWidth; j+=outDataPortWidth)
		{
			unsigned int status = In(posSTLSB,posSTMSB);
			assert(status==(statusChk & statusMask));
			if(status != (statusChk & statusMask))
				return false;	// error - peripheral bus interface state machine not in the right state
			Out((addr>>j)&outDataMask,posDOLSB,posDOMSB);
			Out(0,posCLK,posCLK);
			Out(1,posCLK,posCLK);	// latch address bits into downloading circuit
			statusChk++;			// increment status check
		}
	}

	Out(0,posCLK,posCLK);
	Out(1,posCLK,posCLK);	// latch data into register

	unsigned int d=0;	// holds data read from peripheral bus
	for(int j=0; j<dataWidth; j+=inDataPortWidth)
	{
		d |= ((In(posDILSB,posDIMSB) & inDataMask) << j);	// read bits of data from register
		Out(0,posCLK,posCLK);
		Out(1,posCLK,posCLK);	// output next bits of data from register
	}

	*data = RearrangeData(d,dataWidth,bigEndianBytes,bigEndianBits);
	return true;
}
