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
#include <iostream>
#include <cassert>

#include "utils.h"
#include "hexrecrd.h"
#include "at17prt.h"


/// Create a Flash upload/download port.
AT17Port::AT17Port(void)
{
	progressGauge = NULL;
}

/// Create a serial EEPROM download port.
AT17Port::AT17Port( XSError* e,	///< pointer to error reporting object
				   unsigned int portNum,	///< parallel port number
				   unsigned int invMask,	///< inversion mask for the parallel port
				   unsigned int pos_clk,	///< bit position in parallel port of EEPROM clock pin
				   unsigned int pos_eece,	///< bit position in parallel port of EEPROM chip-enable
				   unsigned int pos_eeoe,	///< bit position in parallel port of EEPROM output-enable
				   unsigned int pos_din		///< bit position in parallel port of EEPROM serial data input
				   )
{
	progressGauge = NULL;
	Setup(e,portNum,invMask,pos_clk,pos_eece,pos_eeoe,pos_din);
}

/// Setup a serial EEPROM download port.
int AT17Port::Setup( XSError* e,	///< pointer to error reporting object
				   unsigned int portNum,	///< parallel port number
				   unsigned int invMask,	///< inversion mask for the parallel port
				   unsigned int pos_clk,	///< bit position in parallel port of EEPROM clock pin
				   unsigned int pos_eece,	///< bit position in parallel port of EEPROM chip-enable
				   unsigned int pos_eeoe,	///< bit position in parallel port of EEPROM output-enable
				   unsigned int pos_din		///< bit position in parallel port of EEPROM serial data input
				   )
{
	posCLK   = pos_clk;
	posEECE = pos_eece;
	posEEOE = pos_eeoe;
	posDIN = pos_din;
	return PPort::Setup(e,portNum,invMask);
}

/// Program a serial EEPROM with an FPGA bitstream stored in a .bit file.
///\return true if operation is successful, false if not.
bool AT17Port::ProgramEEPROM(string& bitfileName) ///< name of file containing bitstream
{
	XSError& err = GetErr();
	
	if(bitfileName.length()==0)
		return false;  // stop if no bit file was given
	
	ifstream is(bitfileName.c_str(),ios::binary);  // otherwise open bit file
	if(!is || is.fail() || is.eof()!=0)
	{ // error - couldn't open bitstream file
		err.SetSeverity(XSErrorMajor);
		err << "could not open " << bitfileName.c_str() << "\n";
		err.EndMsg();
		return false;
	}
	
	// determine the size of the file
	is.seekg(0,ios::end);	// move pointer to end of file
	streampos streamEndPos = is.tellg();	// pointer position = position of end of file
	is.seekg(0,ios::beg);	// return pointer to beginning of file

	string desc("Serial EEPROM Download"), subdesc("Downloading "+StripPrefix(bitfileName));
	progressGauge = new Progress(&err,desc,subdesc,0,streamEndPos);

	bool status = ProgramEEPROM(is);
	
	delete progressGauge;
	
	is.close();  // close-up the bitstream file
	
	return status;
}


/// Program a serial EEPROM with a bitstream received through a stream.
///\return true if the operation is successful, false if not
bool AT17Port::ProgramEEPROM(istream& is) ///< bitstream arrives through this stream
{
	XSError& err = GetErr();

	if(is.eof()!=0)
		return false; // exit if no BIT stream exists

	// Setup the pins of the Atmel EEPROM for programming.
	// The SER_EN_ should already be held low by a shunt on Jumper J6.
	// This holds the EEPROM CE_ low.
	Out(0,posEECE,posEECE);
	Out(0,posEEOE,posEEOE);
	
	// jump over the first field of the BIT file
	long unsigned int fieldLength = GetInteger(is,2);
	assert(fieldLength!=0);
	is.ignore(fieldLength);
	
	// process the second field
	fieldLength = GetInteger(is,2);
	assert(fieldLength==1);
	
	// now look for the field with the bitstream data
	const int BitstreamFieldType = 0x65;	// field type for the bitstream data
	bool status = ScanForField(is,BitstreamFieldType);
	assert(status==true);
	
	// now we are at the start of the configuration bits
	fieldLength = GetInteger(is,4); // get the length of the bit stream
	assert(fieldLength>0);
	
	// now load the EEPROM memory
	long unsigned int i;
	unsigned char page[64];
	for(i=0; i<fieldLength; i++)
	{ 
		is.read((char*)&page[i&0x3f],1);	// read the bytes and into a 64-byte page
		if(is.eof())
		{
			break;
		}
		assert(is.eof()==0);	// shouldn't reach end-of-file
		if((i%1000)==0)		// after every 1000 bytes,
			progressGauge->Report(is.tellg());	// give some feedback during the programming process
		if((i&0x3f)==0x3f)
		{ // got a full 64-byte page, so program the EEPROM
			ProgramEEPROMPage(page,i&~0x3F,64);
		}
	}
	if((i&0x3F)!=0)
	{ // BIT file ended before a full 64-byte page was assembled
		ProgramEEPROMPage(page,i&~0x3F,(i&0x3F)+1);
	}
	progressGauge->Report(is.tellg());	// should set gauge to 100%

	// now set the output-enable polarity for the EEPROM to active-high
	Out(1,posEECE,posEECE);	// raise chip-enable high
	Out(1,posEEOE,posEEOE);	// raise output-enable high
	// write 0xFF to address 0x3FFF to set the output-enable polarity
	unsigned char allOne = 0xFF;
	ProgramEEPROMPage(&allOne,0x3FFF,1);
		
	// programming done, so disable the EEPROM
	Out(1,posEECE,posEECE);

	return true;
}


/// Send a byte into an Atmel EEPROM.
void AT17Port::SendEEPROMByte(unsigned char byte) ///< data byte
{
	for(int i=0; i<8; i++)
	{
		// configuration data pin of parallel port connects to data pin of Atmel EEPROM
		Out(byte&0x80 ? 1:0,posDIN,posDIN);  // byte is sent MSB first
		byte <<= 1;
		Out(1,posCLK,posCLK);
		Out(0,posCLK,posCLK);
	}
	Out(0,posDIN,posDIN);	// this is for the acknowledge bit from the EEPROM
	Out(1,posCLK,posCLK);
	Out(0,posCLK,posCLK);
}


/// Program a page of an Atmel EEPROM.
void AT17Port::ProgramEEPROMPage(unsigned char* buf,	///< buffer with data bytes
							 unsigned int pageAddr,	///< address where data will be stored
							 unsigned int len		///< number of bytes in buffer
							 )
{
	assert(len<=64);	// an Atmel page is 64 bytes or less
	assert(pageAddr<0x20000L);	// Atmel byte addresses are less than 128K
	
	// initialize clock and data pins
	Out(0,posCLK,posCLK);
	Out(1,posDIN,posDIN);
	// give start signal (data falls while clock is high)
	Out(1,posCLK,posCLK);
	Out(0,posDIN,posDIN);
	Out(0,posCLK,posCLK);
	// send device-address byte with A2=0 and R/W=0 (write enabled)
	SendEEPROMByte(0xA6);
	// send two-byte address of EEPROM page
	SendEEPROMByte(pageAddr>>8);	// send upper byte
	SendEEPROMByte(pageAddr&0xFF);	// send lower bytes
	// send data bytes in page
	for(unsigned int i=0; i<len; i++)
		SendEEPROMByte(buf[i]);
	// give stop signal (data rises while clock is high)
	Out(0,posDIN,posDIN);
	Out(1,posCLK,posCLK);
	Out(1,posDIN,posDIN);
	// wait 10 ms
	InsertDelay(10,MILLISECONDS);
}
