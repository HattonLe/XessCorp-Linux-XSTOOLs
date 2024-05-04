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
#include "am29fprt.h"


/// Create a Flash upload/download port.
AM29FPort::AM29FPort(void)
{
	;
}


/// Create a Flash upload/download port.
AM29FPort::AM29FPort(XSError* e,	///< pointer to error reporting object
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
	{0x001F0000, 0x00008000, false},
	{0x001F8000, 0x00002000, false},
	{0x001FA000, 0x00002000, false},
	{0x001FC000, 0x00004000, false},
};


/// Program a byte in the Flash.
///\return true if successful, false if not.
bool AM29FPort::ProgramFlash( unsigned int address,	///< address at which to store data
					unsigned int data,		///< data byte to be stored
					bool bigEndianBytes,	///< data is stored in RAM with most-significant byte at lower address
					bool bigEndianBits)		///< data is stored in RAM with most-significant bit in position 0
{
//	fprintf(stdout,"%02x => (%06x)\n",data,address);
	if(WriteFlashByte(0xAAA,  0xAA,ENDIAN_DEFAULTS)	== false)	return false;
	if(WriteFlashByte(0x555,  0x55,ENDIAN_DEFAULTS)	== false)	return false;
	if(WriteFlashByte(0xAAA,  0xA0,ENDIAN_DEFAULTS)	== false)	return false;
	if(WriteFlashByte(address,data,bigEndianBytes,bigEndianBits)	== false)	return false;

	unsigned int d;
	if(ReadFlashByte(address,&d,ENDIAN_DEFAULTS) == false)
		return false;
	while((data & 0x80) != (d & 0x80))
	{
		if(ReadFlashByte(address,&d,ENDIAN_DEFAULTS) == false)
			return false;
	}

	ReadFlashByte(address,&d,bigEndianBytes,bigEndianBits);
	if(data != d)
	{
		XSError& err = GetErr();
		err.SetSeverity(XSErrorNone);
		err << "data mismatch at " << (long)address << ": " << (long)data << " != " << (long)d << "\n";
		err.EndMsg();
		return false;
	}
	return true;
}


/// Erase the entire contents of the Flash RAM.
///\return true if successful, false if not.
bool AM29FPort::EraseFlash(void)
{
	int i;
	
	if(WriteFlashByte(0xAAA,0xAA,ENDIAN_DEFAULTS) == false)	return false;
	if(WriteFlashByte(0x555,0x55,ENDIAN_DEFAULTS) == false)	return false;
	if(WriteFlashByte(0xAAA,0x80,ENDIAN_DEFAULTS) == false)	return false;
	if(WriteFlashByte(0xAAA,0xAA,ENDIAN_DEFAULTS) == false)	return false;
	if(WriteFlashByte(0x555,0x55,ENDIAN_DEFAULTS) == false)	return false;
	if(WriteFlashByte(0xAAA,0x10,ENDIAN_DEFAULTS) == false)	return false;

	string desc("Flash Erase");
	string subdesc("Erasing flash device");
	Progress* eraseProgressGauge = new Progress(NULL,desc,subdesc,0,30);
	const unsigned int estimated_erase_time = 60;  // units = seconds
	for(i=0; ; i++)
	{
		InsertDelay((estimated_erase_time * 1000)/30,MILLISECONDS);
		eraseProgressGauge->Report(i%30);
		unsigned int d;
		if(ReadFlashByte(0,&d,ENDIAN_DEFAULTS) == false)
		{
			delete eraseProgressGauge;
			return false;
		}
		if(d & 0x80)
			break;	// erase is done when MSB is set
	}
	delete eraseProgressGauge;

	// initialize the block erasure flags
	for(i=sizeof(flashBlocks)/sizeof(struct flashBlock)-1; i>=0; i--)
		flashBlocks[i].erased = true;
	return true;
}


/// Erase the contents of a Flash block.
///\return true if successful, false if not.
bool AM29FPort::EraseFlashBlock(unsigned int blockIndex)
{
	assert(blockIndex < sizeof(flashBlocks)/sizeof(struct flashBlock));
	unsigned int address = flashBlocks[blockIndex].address;
	if(WriteFlashByte(0xAAA,  0xAA,ENDIAN_DEFAULTS)	== false)	return false;
	if(WriteFlashByte(0x555,  0x55,ENDIAN_DEFAULTS)	== false)	return false;
	if(WriteFlashByte(0xAAA,  0x80,ENDIAN_DEFAULTS)	== false)	return false;
	if(WriteFlashByte(0xAAA,  0xAA,ENDIAN_DEFAULTS)	== false)	return false;
	if(WriteFlashByte(0x555,  0x55,ENDIAN_DEFAULTS)	== false)	return false;
	if(WriteFlashByte(address,0x30,ENDIAN_DEFAULTS)	== false)	return false;

	unsigned int d;
	if(ReadFlashByte(address,&d,ENDIAN_DEFAULTS) == false)
		return false;
	while((d & 0x80) == 0)
	{
		if(ReadFlashByte(address,&d,ENDIAN_DEFAULTS) == false)
			return false;
	}

	flashBlocks[blockIndex].erased = true;
	return true;
}


/// Reset the Flash so data can be read from it.
///\return true if successful, false if not.
bool AM29FPort::ResetFlash(void)
{
	return true;
}
