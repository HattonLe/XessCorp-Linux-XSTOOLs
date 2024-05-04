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


#include <dos.h>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <ctime>
#include <string>
using namespace std;

#ifdef WIN32
#include <afxwin.h>
#endif

#include "utils.h"
#include "xc4kprt.h"


/// Instantiate an XC4KPort object on a given parallel port.
XC4KPort::XC4KPort(void)
{
	progressGauge = NULL;
}


/// Instantiate an XC4KPort object on a given parallel port.
XC4KPort::XC4KPort(XSError* e, ///< pointer to error reporting object
				   unsigned int portNum, ///< parallel port number
				   unsigned int invMask, ///< inversion mask for the parallel port
				   unsigned int posCCLK, ///< bit position in parallel port of CCLK pin
				   unsigned int posPROG, ///< bit position in parallel port of PROGRAM pin
				   unsigned int posDIN, ///< bit position in parallel port of DIN pin
				   unsigned int posDONE) ///< bit position in parallel port of DONE pin
{
	progressGauge = NULL;
	Setup(e,portNum,invMask,posCCLK,posPROG,posDIN,posDONE);
}


/// Setup an XC4KPort object on a given parallel port.
///\return true if the operation was a success, false otherwise
bool XC4KPort::Setup(XSError* e, ///< pointer to error reporting object
				   unsigned int portNum, ///< parallel port number
				   unsigned int invMask, ///< inversion mask for the parallel port
				   unsigned int posCCLK, ///< bit position in parallel port of CCLK pin
				   unsigned int posPROG, ///< bit position in parallel port of PROGRAM pin
				   unsigned int posDIN, ///< bit position in parallel port of DIN pin
				   unsigned int posDONE) ///< bit position in parallel port of DONE pin
{
	return CnfgPort::Setup(e,portNum,invMask,posCCLK,posPROG,posDIN,posDONE);
}


static const int DeviceFieldType = 0x62;		// field type for the FPGA device identifier

/// Get the chip identifier from the FPGA bitstream.
///\return the chip identifier
string XC4KPort::GetChipType(istream& is) ///< stream that delivers the bitstream
{
	XSError& err = GetErr();
	
	if(is.eof()!=0)
		return (string)""; // exit if no BIT stream exists
	
	// jump over the first field of the BIT file
	long unsigned int fieldLength = GetInteger(is);
	assert(fieldLength!=0);
	is.ignore(fieldLength);
	
	// process the second field
	fieldLength = GetInteger(is);
	assert(fieldLength==1);
	
	// now look for the field with the chip identifier
	bool status = ScanForField(is,DeviceFieldType);
	assert(status==true);
	fieldLength = GetInteger(is);
	assert(fieldLength>0 && fieldLength<100);
	char chipType[100];
	unsigned int i;
	for(i=0; i<fieldLength; i++)
		is >> chipType[i];
	chipType[i] = 0;		// terminate string
	return chipType;		// return the chip type
}



/// Get the chip identifier from the FPGA bitstream.
///\return the chip identifier
string XC4KPort::GetChipType(string& bitfileName)	///< file containing the FPGA bitstream
{
	XSError& err = GetErr();
	
	if(bitfileName.compare("")==0)
		return (string)""; // exit if no BIT file was given
	
	ifstream is(bitfileName.c_str(),ios::binary);
	if(!is || is.fail()!=0)
	{ // error if given file could not be opened
		err.SetSeverity(XSErrorMajor);
		err << "could not open " << bitfileName.c_str() << "\n";
		err.EndMsg();
		return (string)"";
	}
	
	// now get the chip identifier from the bitstream file
	string chipType = GetChipType(is);
	
	is.close();
	
	return chipType;
}


/// Initialize FPGA configuration pins.
void XC4KPort::InitConfigureFPGA(void)
{
	SetPROG(1);
	SetCCLK(0);
	Out(0x3,6,7);	// set XC4000 mode pins to "slave serial" config. mode
}


/// Send out a byte of configuration information.
void XC4KPort::ConfigureFPGA(unsigned char b)	///< configuration byte from FPGA bitstream
{
	// configuration bytes for the XC4000 programming pins only contain
	// values for the DIN pin, so eight clock pulses are needed per byte.
	for(int i=0; i<8; i++)
	{
		SetDIN(b&0x80 ? 1:0);  // byte is sent MSB first
		b <<= 1;
		PulseCCLK();
	}
}


static const int BitstreamFieldType = 0x65;	// field type for the bitstream data

/// Process a stream and send the configuration bitstream to the board.
///\return true if the operation was a success, false otherwise
bool XC4KPort::ConfigureFPGA(istream& is)	///< stream that delivers the FPGA bitstream
{
	XSError& err = GetErr();
	
	if(is.eof()!=0)
		return false; // exit if no BIT stream exists

	assert(progressGauge != NULL);	// make sure progress indicator is initialized

	// show the progress indicator
	progressGauge->Report(0);
	
	// jump over the first field of the BIT file
	long unsigned int fieldLength = GetInteger(is);
	assert(fieldLength!=0);
	is.ignore(fieldLength);
	
	// process the second field
	fieldLength = GetInteger(is);
	assert(fieldLength==1);
	
	// now look for the field with the bitstream data
	bool status = ScanForField(is,BitstreamFieldType);
	assert(status==true);
	
	// now we are at the start of the configuration bits
	fieldLength = GetInteger(is,4); // get the length of the bit stream
	assert(fieldLength>0);
	DEBUG_STMT("bit stream field length = " << fieldLength)

	// start the configuration process
	PulsePROG();					// pulse the PROGRAM pin to start the configuration of the FPGA
	InsertDelay(300,MILLISECONDS);	// insert 300 ms delay after PROGRAM pin goes high

	for(long unsigned int i=0; i<fieldLength; i++)
	{ // read the bytes and send the config. bits to the board
		unsigned char byte;
		is.read((char*)&byte,1);
		assert(is.eof()==0);	// should not hit end-of-file
		ConfigureFPGA(byte);
		// output some feedback as the configuration proceeds
		if((i%1000)==0)
			progressGauge->Report(is.tellg());
	}
	progressGauge->Report(is.tellg());	// this should set gauge to 100%
	
	// clock a few more times at the end of the configuration process
	// to clean things up...
	for(int ii=0; ii<4; ii++)
		PulseCCLK();

	// insert a delay to let the chip initialize
	InsertDelay(100,MILLISECONDS);
	
	return true;
}



/// Process an FPGA bitstream file and send the configuration bitstream to the board.
///\return true if the operation was a success, false otherwise
bool XC4KPort::ConfigureFPGA(string& fileName)	///< file with FPGA bitstream
{
	XSError& err = GetErr();
	
	if(fileName.length()==0)
		return false;  // stop if no SVF or BIT file was given
	
	ifstream is(fileName.c_str(),ios::binary);  // otherwise open BIT or SVF file
	if(!is || is.fail() || is.eof())
	{ // error - couldn't open file
		err.SetSeverity(XSErrorMajor);
		err << "could not open " << fileName.c_str() << "\n";
		err.EndMsg();
		return false;
	}
	
	// determine the size of the file
	is.seekg(0,ios::end);	// move pointer to end of file
	streampos streamEndPos = is.tellg();	// pointer position = position of end of file

	is.seekg(0,ios::beg);	// return pointer to beginning of file

	string desc("Configure FPGA"), subdesc("Downloading "+StripPrefix(fileName));
	progressGauge = new Progress(&err,desc,subdesc,0,streamEndPos);
	
	bool status = ConfigureFPGA(is);
	
	delete progressGauge;
	
	is.close();  // close-up the hex file
	
	return status;
}
