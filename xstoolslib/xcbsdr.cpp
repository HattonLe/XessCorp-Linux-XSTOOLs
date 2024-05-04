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


#include "xcbsdr.h"


// The bsdr is partitioned into three-bit segments.  Each three bit segment
// interfaces to a pin of the FPLD. The bit functions are:
//		bit 0: controls the output driver of the pin
//				0 = tristate the output driver
//				1 = enable the driver
//		bit 1: stores the value to be output on the pin
//		bit 2: stores the value read from the pin   
//

static const int tristate = 0;		// disable pin driver
static const int enable = 1;		// enable pin driver
static const int bsdrPinEnable = 0;	// pin tristate control bit position offset
static const int bsdrPinOutput = 1;	// pin output bit position offset
static const int bsdrPinInput  = 2;	// pin input bit position offset


/// Create a boundary-scan data register.
XCBSDR::XCBSDR(unsigned int length,			///< length of bsdr
			unsigned int* addressBitPos,	///< positions of RAM address bits in bsdr
			unsigned int* dataBitPos,		///< positions of RAM databus bits in bsdr
			unsigned int* controlBitPos)	///< positions of RAM control bits in bsdr
			: Bitstream(length)
{
	int i;
	for(i=0; i<numRAMAddressBits; i++)
		ramAddressBitPos[i] = addressBitPos[i];
	for(i=0; i<numRAMDataBits; i++)
		ramDataBitPos[i] = dataBitPos[i];
	for(i=0; i<numRAMControlBits; i++)
		ramControlBitPos[i] = controlBitPos[i];
}


/// Place a RAM address into the appropriate position in bsdr bitstream.
void XCBSDR::SetRAMAddress(unsigned int address)	///< RAM address
{
	for(int i=0; i<numRAMAddressBits; i++,address>>=1)
	{
		// enable the output drivers of the FPLD pin connected to the RAM address pin
		SetBit(ramAddressBitPos[i]+bsdrPinEnable,enable);
		// set the address bit value on the FPLD pin
		SetBit(ramAddressBitPos[i]+bsdrPinOutput,address&1);
	}
}


/// Place RAM data into the appropriate position in bsdr bitstream.
void XCBSDR::SetRAMData(unsigned int data)	///< RAM data
{
	for(int i=0; i<numRAMDataBits; i++,data>>=1)
	{
		// enable the output driver of the FPLD pin connected to the RAM data pin
		SetBit(ramDataBitPos[i]+bsdrPinEnable,enable);
		// set the data bit value on the FPLD pin
		SetBit(ramDataBitPos[i]+bsdrPinOutput,data&1);
	}
}


/// Set the bits in the bsdr bitstream to tristate the CPLD pins connected to the RAM databus so the RAM can drive it.
void XCBSDR::ReadRAMData(void)
{
	for(int i=0; i<numRAMDataBits; i++)
	{
		// tristate the FPLD pin that drives the RAM data line
		SetBit(ramDataBitPos[i]+bsdrPinEnable,tristate);
	}
}


/// Extract the RAM data value from the bsdr bitstream.
///\return the data received from the RAM
unsigned int XCBSDR::GetRAMData()
{
	unsigned int data = 0;
	for(int i=numRAMDataBits-1; i>=0; i--)
		data = (data<<1) | ((*this)[ramDataBitPos[i]+bsdrPinInput] ? 1:0);
	return data;
}


/// Extract the RAM address value from the bsdr bitstream.
///\return the RAM address
unsigned long XCBSDR::GetRAMAddress(void)
{
	unsigned int address = 0;
	for(int i=numRAMAddressBits-1; i>=0; i--)
		address = (address<<1) | ((*this)[ramAddressBitPos[i]+bsdrPinInput] ? 1:0);
	return address;
}


/// Place RAM control pin levels into the bsdr bitstream.
void XCBSDR::SetRAMControls(unsigned int ce_,	///< RAM chip-enable
				unsigned int oe_,				///< RAM output-enable
				unsigned int we_)				///< RAM write-enable
{
	// set the output drivers of the FPLD pins which drive the RAM control pins
	SetBit(ramControlBitPos[0]+bsdrPinEnable,enable);
	SetBit(ramControlBitPos[1]+bsdrPinEnable,enable);
	SetBit(ramControlBitPos[2]+bsdrPinEnable,enable);
	// set the control signal levels on the FPLD pins
	SetBit(ramControlBitPos[0]+bsdrPinOutput,ce_);
	SetBit(ramControlBitPos[1]+bsdrPinOutput,oe_);
	SetBit(ramControlBitPos[2]+bsdrPinOutput,we_);
}
