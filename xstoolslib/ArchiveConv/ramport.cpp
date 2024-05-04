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
#include "ramport.h"


/// Create a RAM upload/download port.
RAMPort::RAMPort(void)
{
	;
}


/// Create a RAM upload/download port.
RAMPort::RAMPort(XSError* e,	///< pointer to error reporting object
				   unsigned int portNum,	///< parallel port number
				   unsigned int invMask,	///< inversion mask for the parallel port
				   unsigned int pos_reset,	///< bit position in parallel port of RAM RESET pin
				   unsigned int pos_clk,	///< bit position in parallel port of RAM CLK pin
				   unsigned int pos_dolsb,	///< bit position in parallel port of LSB of RAM data-out pin
				   unsigned int pos_domsb,	///< bit position in parallel port of MSB of RAM data-out pin
				   unsigned int pos_dilsb,	///< bit position in parallel port of LSB of RAM data-in pin
				   unsigned int pos_dimsb,	///< bit position in parallel port of MSB of RAM data-in pin
				   unsigned int pos_stlsb,	///< bit position in parallel port of LSB of RAM status pin
				   unsigned int pos_stmsb,	///< bit position in parallel port of MSB of RAM status pin
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
int RAMPort::Setup(XSError* e,	///< pointer to error reporting object
				   unsigned int portNum,	///< parallel port number
				   unsigned int invMask,	///< inversion mask for the parallel port
				   unsigned int pos_reset,	///< bit position in parallel port of RAM RESET pin
				   unsigned int pos_clk,	///< bit position in parallel port of RAM CLK pin
				   unsigned int pos_dolsb,	///< bit position in parallel port of LSB of RAM data-out pin
				   unsigned int pos_domsb,	///< bit position in parallel port of MSB of RAM data-out pin
				   unsigned int pos_dilsb,	///< bit position in parallel port of LSB of RAM data-in pin
				   unsigned int pos_dimsb,	///< bit position in parallel port of MSB of RAM data-in pin
				   unsigned int pos_stlsb,	///< bit position in parallel port of LSB of RAM status pin
				   unsigned int pos_stmsb,	///< bit position in parallel port of MSB of RAM status pin
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


/// Download the RAM with the contents of a HEX file.
///\return true if the operation was successful, false if not
bool RAMPort::DownloadRAM(string& hexfileName,	///< hex file containing the data to be downloaded
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
///\return true if the operation was successful, false if not
bool RAMPort::DownloadRAM(istream& is,	///< stream that delivers hex data to be downloaded
				bool bigEndianBytes,	///< if true, data is stored in RAM with most-significant byte at lower address
				bool bigEndianBits)		///< if true, data is stored in RAM with most-significant bit in position 0
{
	XSError& err = GetErr();

	assert(progressGauge != NULL);	//	make sure progress indicator is initialized
	progressGauge->Report(0);	// start progress indicator at zero

	// read hex records from file and place data in the RAM of the board
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
			if(DownloadHexRecordToRAM(hx,bigEndianBytes,bigEndianBits) == false) // download hex record
				return false;	// an error occurred while downloading the hex record
		}
	}
	progressGauge->Report(is.tellg());	// should set gauge to 100%
	
	return err.IsError() ? false:true;
}


/// Upload the XS Board RAM to a file.
///\return true if the operation was successful, false if not
bool RAMPort::UploadRAM(string& hexfileName,	///< dump uploaded data to this file
				const char* format,		///< hex file format
				unsigned long loAddr,	///< start fetching data from this address
				unsigned long hiAddr,	///< stop at this address
				bool bigEndianBytes,	///< if true, data is stored in RAM with most-significant byte at lower address
				bool bigEndianBits)		///< if true, data is stored in RAM with most-significant bit in position 0
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
	
	bool status = UploadRAM(os,format,loAddr,hiAddr,bigEndianBytes,bigEndianBits);
	
	delete progressGauge;

	os.close();  // close-up the hex file

	return status;
}


/// Upload the XS Board RAM to a stream.
///\return true if the operation was successful, false if not
bool RAMPort::UploadRAM(ostream& os,	///< dump uploaded data to this stream
				const char* format,		///< hex file format
				unsigned long loAddr,	///< start fetching data from this address
				unsigned long hiAddr,	///< stop at this address
				bool bigEndianBytes,	///< if true, data is stored in RAM with most-significant byte at lower address
				bool bigEndianBits)		///< if true, data is stored in RAM with most-significant bit in position 0
{
	XSError& err = GetErr();

	assert(progressGauge != NULL);	//	make sure progress indicator is initialized
	progressGauge->Report(0);	// start progress indicator at zero
	
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
		if(UploadHexRecordFromRAM(hx,addr,hiAddr,bigEndianBytes,bigEndianBits) == false) // get the last few bytes from RAM
			return false;	// an error occurred while uploading the hex record
		os << hx;		// send hex record to output stream
	}
	progressGauge->Report(hiAddr);	// should set gauge to 100%
	
	return true;
}


/// Load RAM data at the addresses given in a hex record.
///\return true if the operation was successful, false if not
bool RAMPort::DownloadHexRecordToRAM(HexRecord& hx,	///< hex record containing data to download to RAM
					bool bigEndianBytes,	///< if true, data is stored in RAM with most-significant byte at lower address
					bool bigEndianBits)		///< if true, data is stored in RAM with most-significant bit in position 0
{
	XSError& err = GetErr();

	if(!hx.IsData()) // don't download unless this is RAM data
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
		string msg("Cannot download an odd number of bytes to word-wide RAM!\n");
		err.SimpleMsg(XSErrorMajor,msg);
		return false;
	}
	unsigned int loAddr = hx.GetAddress();
	if(loAddr & ~addrMask)
	{
		string msg("Cannot download to word-wide RAM using an odd byte-starting address!\n");
		err.SimpleMsg(XSErrorMajor,msg);
		return false;
	}
	
	Out(1,posRESET,posRESET);	// reset the downloading state machine
	Out(0,posCLK,posCLK);		// force clock low
	Out(0,posRESET,posRESET);	// release the reset
	unsigned int statusChk = 0;	// set status check to reset state id
	unsigned int status = In(posSTLSB,posSTMSB);
	assert(status==statusChk);	// make sure we are in reset state
	if(status != statusChk)
		return false;	// error - couldn't reset RAM interface state machine
	
	unsigned int address = loAddr >> addrScale;	// get starting address for hex record
	int j;
	for(j=addrWidth-4; j>=0; j-=4)
	{
		Out((address>>j)&0xf,posDOLSB,posDOMSB);
		Out(1,posCLK,posCLK);	// latch 4 address bits into downloading circuit
		statusChk++;			// increment status check but don't check it
		Out(0,posCLK,posCLK);
	}
	status = In(posSTLSB,posSTMSB);
	assert(status==statusChk);
	if(status != statusChk)
		return false;	// error - RAM interface state machine not in the right state
	
	// process the bytes from a data-type hex record
	unsigned int statusChkSave = statusChk;
	for(unsigned int i=0; i<hx.GetLength(); i+=stride)
	{
		unsigned int data = Hex2Data(hx,i,stride,bigEndianBytes,bigEndianBits);	// assemble next data word from hex record
		for(j=dataWidth-4; j>0; j-=4)
		{
			Out((data>>j)&0xf,posDOLSB,posDOMSB);
			Out(1,posCLK,posCLK);	// latch 4 data bits into downloading circuit
			// get the last 4 bits out while the clock is high because the
			// write to RAM will occur when the clock goes low in a few statements 
			if(j==4)
				Out(data&0xf,posDOLSB,posDOMSB);
			statusChk++;				// increment to next state id
			Out(0,posCLK,posCLK);
			status = In(posSTLSB,posSTMSB);
			assert(status==statusChk);
			if(status != statusChk)
				return false;	// error - RAM interface state machine not in the right state
		}
		Out(1,posCLK,posCLK);		// terminate RAM write pulse
		statusChk = statusChkSave;	// loop status check once data word is downloaded
		Out(0,posCLK,posCLK);
		status = In(posSTLSB,posSTMSB);
		assert(status==statusChk);
		if(status != statusChk)
			return false;	// error - RAM interface state machine not in the right state
	}
	
	Out(1,posRESET,posRESET);	// reset the downloading state machine
	Out(0,posRESET,posRESET);	// release the reset
	statusChk = 0;				// set status check to reset state id
	status = In(posSTLSB,posSTMSB);
	assert(status==statusChk);	// make sure we are in reset state
	if(status != statusChk)
		return false;	// error - RAM interface state machine not in the right state
	else
		return true;
}


/// Write contents of a single location in RAM.
///\return true if the operation was successful, false if not
bool RAMPort::WriteRAM(unsigned int addr,	///< address of RAM location
			unsigned int data,		///< data to be written into RAM location
			bool bigEndianBytes,	///< if true, data is stored in RAM with most-significant byte at lower address
			bool bigEndianBits)		///< if true, data is stored in RAM with most-significant bit in position 0
{
	XSError& err = GetErr();
	
	Out(1,posRESET,posRESET);	// reset the downloading state machine
	Out(0,posCLK,posCLK);		// force clock low
	Out(0,posRESET,posRESET);	// release the reset
	unsigned int statusChk = 0;	// set status check to reset state id
	unsigned int status = In(posSTLSB,posSTMSB);
	assert(status==statusChk);	// make sure we are in reset state
	if(status != statusChk)
		return false;	// error - couldn't reset RAM interface state machine
	
	int j;
	for(j=addrWidth-4; j>=0; j-=4)
	{
		Out((addr>>j)&0xf,posDOLSB,posDOMSB);
		Out(1,posCLK,posCLK);	// latch 4 address bits into downloading circuit
		statusChk++;			// increment status check but don't check it
		Out(0,posCLK,posCLK);
	}
	status = In(posSTLSB,posSTMSB);
	assert(status==statusChk);
	if(status != statusChk)
		return false;	// error - RAM interface state machine not in the right state
	
	// process the bytes from a data-type hex record
	unsigned int statusChkSave = statusChk;
	data = RearrangeData(data,dataWidth,bigEndianBytes,bigEndianBits);
	for(j=dataWidth-4; j>0; j-=4)
	{
		Out((data>>j)&0xf,posDOLSB,posDOMSB);
		Out(1,posCLK,posCLK);	// latch 4 data bits into downloading circuit
		// get the last 4 bits out while the clock is high because the
		// write to RAM will occur when the clock goes low in a few statements 
		if(j==4)
			Out(data&0xf,posDOLSB,posDOMSB);
		statusChk++;				// increment to next state id
		Out(0,posCLK,posCLK);
		status = In(posSTLSB,posSTMSB);
		assert(status==statusChk);
		if(status != statusChk)
			return false;	// error - RAM interface state machine not in the right state
	}
	Out(1,posCLK,posCLK);		// terminate RAM write pulse
	statusChk = statusChkSave;	// loop status check once data word is downloaded
	Out(0,posCLK,posCLK);
	status = In(posSTLSB,posSTMSB);
	assert(status==statusChk);
	if(status != statusChk)
		return false;	// error - RAM interface state machine not in the right state
	
	Out(1,posRESET,posRESET);	// reset the downloading state machine
	Out(0,posRESET,posRESET);	// release the reset
	statusChk = 0;				// set status check to reset state id
	status = In(posSTLSB,posSTMSB);
	assert(status==statusChk);	// make sure we are in reset state
	if(status != statusChk)
		return false;	// error - RAM interface state machine not in the right state
	else
		return true;
}


/// Get RAM data at the addresses given and store it in a hex record.
///\return true if the operation was successful, false if not
bool RAMPort::UploadHexRecordFromRAM(HexRecord& hx,	///< hex record to store data in
				unsigned long loAddr,	///< begin upload from this address
				unsigned long hiAddr,	///< end upload at this address
				bool bigEndianBytes,	///< if true, data is stored in RAM with most-significant byte at lower address
				bool bigEndianBits)		///< if true, data is stored in RAM with most-significant bit in position 0
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
		string msg("Cannot upload an odd number of bytes from word-wide RAM!\n");
		err.SimpleMsg(XSErrorMajor,msg);
		return false;
	}
	if(loAddr & ~addrMask)
	{
		string msg("Cannot upload from word-wide RAM using an odd byte-starting address!\n");
		err.SimpleMsg(XSErrorMajor,msg);
		return false;
	}

	// clear the lower bits of the address bounds
	loAddr = loAddr & addrMask;
	hiAddr = hiAddr & addrMask;

	hx.SetAddress(loAddr);	// set beginning and ending addresses for the hex record
	hx.SetLength(hiAddr-loAddr+stride);
	
	Out(1,posRESET,posRESET);	// reset the downloading state machine
	Out(0,posCLK,posCLK);		// force clock low
	Out(0,posRESET,posRESET);	// release the reset
	unsigned int statusChk = 0;	// set status check to reset state id
	unsigned int status = In(posSTLSB,posSTMSB);
	assert(status==statusChk);	// make sure we are in reset state
	if(status != statusChk)
		return false;	// error - RAM interface state machine not in the right state

	// set initial address into the downloading state machine
	int j;
	for(j=addrWidth-4; j>=0; j-=4)
	{
		Out((loAddr>>(addrScale+j))&0xf,posDOLSB,posDOMSB);
		Out(1,posCLK,posCLK);	// latch 4 address bits into downloading circuit
		statusChk++;			// increment status check but don't check it
		Out(0,posCLK,posCLK);
	}
	status = In(posSTLSB,posSTMSB);
	assert(status==statusChk);
	if(status != statusChk)
		return false;	// error - RAM interface state machine not in the right state

	Out(1,posRESET,posRESET);	// reset the downloading state machine
	Out(0,posRESET,posRESET);	// release the reset
	statusChk = 0;				// set status check to reset state id
	status = In(posSTLSB,posSTMSB);
	assert(status==statusChk);	// make sure we are in reset state
	if(status != statusChk)
		return false;	// error - RAM interface state machine not in the right state

	unsigned int data;	// holds data read from RAM

	// process the bytes from a data-type hex record.
	// we have to read up to hiAddr+1 in order to get the data from hiAddr.
	for(unsigned long i=loAddr+stride; i<=hiAddr+stride; i+=stride)
	{
		int k;
		data = 0;
		for(j=addrWidth-4, k=dataWidth; j>=0; j-=4, k-=4)
		{
			Out((i>>(addrScale+j))&0xf,posDOLSB,posDOMSB);
			Out(1,posCLK,posCLK);	// latch 4 address bits into downloading circuit
			statusChk++;			// increment status check
			Out(0,posCLK,posCLK);
			if(k>0)
			{ // read 4 bits of data
				data = (data<<4) | (In(posDILSB,posDIMSB) & 0xf);
			}
			else
			{ // check status if not reading data
				status = In(posSTLSB,posSTMSB);
				assert(status==statusChk);	// make sure we are in the correct state
				if(status != statusChk)
					return false;	// error - RAM interface state machine not in the right state
			}
		}
		Data2Hex(data,hx,i-loAddr-stride,dataWidth>>3,bigEndianBytes,bigEndianBits);

		Out(1,posRESET,posRESET);	// reset the downloading state machine
		Out(0,posRESET,posRESET);	// release the reset
		statusChk = 0;				// set status check to reset state id
		status = In(posSTLSB,posSTMSB);
		assert(status==statusChk);	// make sure we are in reset state
		if(status != statusChk)
			return false;	// error - RAM interface state machine not in the right state
	}
	
	hx.CalcCheckSum();	// put the checksum into the hex record
	return true;
}


/// Read contents of a single location in RAM.
///\return true if the operation was successful, false if not
bool RAMPort::ReadRAM(unsigned int addr,	///< address of RAM location
		unsigned int* data,		///< data returned from RAM location
		bool bigEndianBytes,	///< if true, data is stored in RAM with most-significant byte at lower address
		bool bigEndianBits)		///< if true, data is stored in RAM with most-significant bit in position 0
{
	XSError& err = GetErr();

	Out(1,posRESET,posRESET);	// reset the downloading state machine
	Out(0,posCLK,posCLK);		// force clock low
	Out(0,posRESET,posRESET);	// release the reset
	unsigned int statusChk = 0;	// set status check to reset state id
	unsigned int status = In(posSTLSB,posSTMSB);
	assert(status==statusChk);	// make sure we are in reset state
	if(status != statusChk)
		return false;	// error - RAM interface state machine not in the right state

	// set initial address into the downloading state machine
	int j;
	for(j=addrWidth-4; j>=0; j-=4)
	{
		Out((addr>>j)&0xf,posDOLSB,posDOMSB);
		Out(1,posCLK,posCLK);	// latch 4 address bits into downloading circuit
		statusChk++;			// increment status check but don't check it
		Out(0,posCLK,posCLK);
	}
	status = In(posSTLSB,posSTMSB);
	assert(status==statusChk);
	if(status != statusChk)
		return false;	// error - RAM interface state machine not in the right state
	
	Out(1,posRESET,posRESET);	// reset the downloading state machine
	Out(0,posRESET,posRESET);	// release the reset
	statusChk = 0;				// set status check to reset state id
	status = In(posSTLSB,posSTMSB);
	assert(status==statusChk);	// make sure we are in reset state
	if(status != statusChk)
		return false;	// error - RAM interface state machine not in the right state
	
	unsigned int d;	// holds data read from RAM
	
	int k;
	d = 0;
	for(j=addrWidth-4, k=dataWidth; j>=0; j-=4, k-=4)
	{
		Out((addr>>j)&0xf,posDOLSB,posDOMSB);
		Out(1,posCLK,posCLK);	// latch 4 address bits into downloading circuit
		statusChk++;			// increment status check
		Out(0,posCLK,posCLK);
		if(k>0)
		{ // read 4 bits of data
			d = (d<<4) | (In(posDILSB,posDIMSB) & 0xf);
		}
		else
		{ // check status if not reading data
			status = In(posSTLSB,posSTMSB);
			assert(status==statusChk);	// make sure we are in the correct state
			if(status != statusChk)
				return false;	// error - RAM interface state machine not in the right state
		}
	}
	Out(1,posRESET,posRESET);	// reset the downloading state machine
	Out(0,posRESET,posRESET);	// release the reset
	statusChk = 0;				// set status check to reset state id
	status = In(posSTLSB,posSTMSB);
	assert(status==statusChk);	// make sure we are in reset state
	if(status != statusChk)
		return false;	// error - RAM interface state machine not in the right state

	*data = RearrangeData(d,dataWidth,bigEndianBytes,bigEndianBits);
	return true;
}
