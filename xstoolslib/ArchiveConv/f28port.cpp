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
#include <string>
using namespace std;

#include "utils.h"
#include "progress.h"
#include "f28port.h"


/// Create a Flash upload/download port.
F28Port::F28Port(void)
{
	;
}


/// Create a Flash upload/download port.
F28Port::F28Port(XSError* e,	///< pointer to error reporting object
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
	: FlashPort(e,portNum,invMask,pos_reset,pos_clk,pos_dolsb,pos_domsb,pos_dilsb,pos_dimsb,pos_stlsb,pos_stmsb)
{
	;
}


static const unsigned int rdyBitMask = 1<<7;

struct flashBlock
{
	unsigned int address; // address of block
	unsigned int length; // length of block
	bool erased; // true if block is erase
} static flashBlocks[] = 
{
	{0x00000000, 0x00010000, false},
	{0x00010000, 0x00010000, false},
	{0x00020000, 0x00010000, false},
	{0x00030000, 0x00010000, false},
	{0x00040000, 0x00010000, false},
	{0x00050000, 0x00010000, false},
	{0x00060000, 0x00010000, false},
	{0x00070000, 0x00010000, false},
	{0x00080000, 0x00010000, false},
	{0x00090000, 0x00010000, false},
	{0x000A0000, 0x00010000, false},
	{0x000B0000, 0x00010000, false},
	{0x000C0000, 0x00010000, false},
	{0x000D0000, 0x00010000, false},
	{0x000E0000, 0x00010000, false},
	{0x000F0000, 0x00010000, false},
	{0x00100000, 0x00010000, false},
	{0x00110000, 0x00010000, false},
	{0x00120000, 0x00010000, false},
	{0x00130000, 0x00010000, false},
	{0x00140000, 0x00010000, false},
	{0x00150000, 0x00010000, false},
	{0x00160000, 0x00010000, false},
	{0x00170000, 0x00010000, false},
	{0x00180000, 0x00010000, false},
	{0x00190000, 0x00010000, false},
	{0x001A0000, 0x00010000, false},
	{0x001B0000, 0x00010000, false},
	{0x001C0000, 0x00010000, false},
	{0x001D0000, 0x00010000, false},
	{0x001E0000, 0x00010000, false},
	{0x001F0000, 0x00010000, false},
};


/// Program a byte in the Flash.
///\return true if successful, false if not.
bool F28Port::ProgramFlash( unsigned int address,	///< address at which to store data
					unsigned int data,		///< data byte to be stored
					bool bigEndianBytes,	///< data is stored in RAM with most-significant byte at lower address
					bool bigEndianBits)		///< data is stored in RAM with most-significant bit in position 0
{
	if(WriteFlashByte(address,0x40,ENDIAN_DEFAULTS) == false)	return false;
	if(WriteFlashByte(address,data,bigEndianBytes,bigEndianBits) == false)	return false;
	unsigned int status;
	if(ReadFlashStatus(&status) == false)	return false;
	while((status & rdyBitMask) == 0)
	{
		if(ReadFlashStatus(&status) == false)
			return false;
	}

	XSError& err = GetErr();
	const unsigned int programErrorMask = (1<<4)|(1<<3)|(1<<1);
	if((status & programErrorMask) != 0)
	{
		err.SetSeverity(XSErrorNone);
		err << "error programming byte at " << (long)address << ": " << (long)data << "\n";
		err.EndMsg();
		ClearFlashStatus();
		return false;
	}
#if 0
	unsigned int d = ReadFlashByte(address,bigEndianBytes,bigEndianBits);
	if(d != data)
	{
		err.SetSeverity(XSErrorNone);
		err << "data mismatch at " << address << ": " << data << " != " << d << "\n";
		err.EndMsg();
	}
#endif
	return true;
}


/// Erase the entire contents of the Flash RAM.
///\return true if successful, false if not.
bool F28Port::EraseFlash(void)
{
	string desc("Flash Erase"), subdesc("Erasing flash device");
	Progress* eraseProgressGauge = new Progress(NULL,desc,subdesc,0,sizeof(flashBlocks)/sizeof(struct flashBlock)-1);
	for(int i=0; i<sizeof(flashBlocks)/sizeof(struct flashBlock); i++)
	{
		if(EraseFlashBlock(i) == false)
		{
			delete eraseProgressGauge;
			return false;
		}
		eraseProgressGauge->Report(i);
	}
	delete eraseProgressGauge;
	return true;
}


/// Erase the contents of a Flash block.
///\return true if successful, false if not.
bool F28Port::EraseFlashBlock(unsigned int blockIndex) ///< index of Flash block
{
	assert(blockIndex < sizeof(flashBlocks)/sizeof(struct flashBlock));

	static const unsigned int eraseErrorMask = (1<<5)|(1<<4)|(1<<3)|(1<<1);

	unsigned int address = flashBlocks[blockIndex].address;
	if(WriteFlashByte(address,0x20,ENDIAN_DEFAULTS) == false)	return false;
	if(WriteFlashByte(address,0xD0,ENDIAN_DEFAULTS) == false)	return false;
	unsigned int status;
	if(ReadFlashStatus(&status) == false)	return false;
	while((status & rdyBitMask) == 0)
	{
		if(ReadFlashStatus(&status) == false)
			return false;
	}
	if((status & eraseErrorMask) != 0)
	{
		XSError& err = GetErr();
		err.SetSeverity(XSErrorNone);
		err << "error erasing block at " << (long)address << "\n";
		err.EndMsg();
		ClearFlashStatus();
		return false;
	}
	flashBlocks[blockIndex].erased = true;
	return true;
}


/// Reset the Flash so data can be read from it.
///\return true if successful, false if not.
bool F28Port::ResetFlash(void)
{
	return WriteFlashByte(0,0xFF,ENDIAN_DEFAULTS);
}


/// Read the ID from the Flash.
///\return true if successful, false if not.
bool F28Port::ReadFlashID(unsigned int* id) ///< pointer to location to store ID
{
	if(WriteFlashByte(0,0x90,ENDIAN_DEFAULTS) == false)		return false;
	unsigned int id1,id2;
	if(ReadFlashByte(0x0,&id1,ENDIAN_DEFAULTS) == false)	return false;
	if(ReadFlashByte(0x1,&id2,ENDIAN_DEFAULTS) == false)	return false;
	*id = id1 | (id2<<8);
	return true;
}


/// Get the status of the Flash.
///\return true if successful, false if not.
bool F28Port::ReadFlashStatus(unsigned int* status) ///< pointer to location to store Flash status
{
	if(WriteFlashByte(0,0x70,ENDIAN_DEFAULTS) == false)		return false;
	return ReadFlashByte(0,status,ENDIAN_DEFAULTS);
}


/// Clear any error bits in the Flash status register.
///\return true if successful, false if not.
bool F28Port::ClearFlashStatus(void)
{
	return WriteFlashByte(0,0x50,ENDIAN_DEFAULTS);
}


/// Lock a Flash block so it can't be altered.
///\return true if successful, false if not.
bool F28Port::LockFlashBlock(unsigned int blockIndex) ///< index of Flash block
{
	assert(blockIndex < sizeof(flashBlocks)/sizeof(struct flashBlock));

	const unsigned int lockBlockErrorMask = (1<<5)|(1<<4)|(1<<3)|(1<<1);

	unsigned int address = flashBlocks[blockIndex].address;
	if(WriteFlashByte(address,0x60,ENDIAN_DEFAULTS) == false)	return false;
	if(WriteFlashByte(address,0x01,ENDIAN_DEFAULTS) == false)	return false;
	unsigned int status;
	if(ReadFlashStatus(&status) == false)	return false;
	while((status & rdyBitMask) == 0)
	{
		if(ReadFlashStatus(&status) == false)
			return false;
	}
	if((status & lockBlockErrorMask) != 0)
	{
		XSError& err = GetErr();
		err.SetSeverity(XSErrorNone);
		err << "error locking block at " << (long)address << "\n";
		err.EndMsg();
		ClearFlashStatus();
		return false;
	}
	return true;
}


/// Lock all the block lock bits so they cannot be altered.
///\return true if successful, false if not.
bool F28Port::MasterLockFlash(void)
{
 	const unsigned int lockBlockErrorMask = (1<<5)|(1<<4)|(1<<3)|(1<<1);

	if(WriteFlashByte(0,0x60,ENDIAN_DEFAULTS) == false)		return false;
	if(WriteFlashByte(0,0xF1,ENDIAN_DEFAULTS) == false)		return false;
	unsigned int status;
	if(ReadFlashStatus(&status) == false)	return false;
	while((status & rdyBitMask) == 0)
	{
		if(ReadFlashStatus(&status) == false)
			return false;
	}
	if((status & lockBlockErrorMask) != 0)
	{
		XSError& err = GetErr();
		err.SetSeverity(XSErrorNone);
		err << "error unlocking block lock bits" << "\n";
		err.EndMsg();
		ClearFlashStatus();
		return false;
	}
	return true;
}


/// Unlock all the Flash blocks.
///\return true if successful, false if not.
bool F28Port::UnlockFlashBlocks(void)
{
	const unsigned int unlockBlockErrorMask = (1<<5)|(1<<4)|(1<<3)|(1<<1);
	if(WriteFlashByte(0,0x60,ENDIAN_DEFAULTS) == false)		return false;
	if(WriteFlashByte(0,0xD0,ENDIAN_DEFAULTS) == false)		return false;
	unsigned int status;
	if(ReadFlashStatus(&status) == false)	return false;
	while((status & rdyBitMask) == 0)
	{
		if(ReadFlashStatus(&status) == false)
			return false;
	}
	if((status & unlockBlockErrorMask) != 0)
	{
		XSError& err = GetErr();
		err.SetSeverity(XSErrorNone);
		err << "error unlocking blocks" << "\n";
		err.EndMsg();
		ClearFlashStatus();
		return false;
	}
	return true;
}
