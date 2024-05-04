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


#ifndef XCBSDR_H
#define XCBSDR_H

#include "bitstrm.h"

const int numRAMAddressBits = 17;
const int numRAMDataBits = 8;
const int numRAMControlBits = 3;


/**
 Interface to the JTAG boundary scan data register.
 
 This object adds methods to the Bitstream object that make it easier to access the address, data, and control bits of the XC9500 BSDR.
*/
class XCBSDR : public Bitstream
{
public:
	
	XCBSDR(unsigned int length, unsigned int *addressBitPos,
		unsigned int *dataBitPos, unsigned int *controlBitPos);
	
	void SetRAMAddress(unsigned int address);
	
	void SetRAMData(unsigned int data);
	
	void ReadRAMData(void);
	
	unsigned int GetRAMData(void);
	
	unsigned long GetRAMAddress(void);
	
	void SetRAMControls(unsigned int cs_, unsigned int oe_, unsigned int we_);


private:

	// The RAM of the XS board is controlled through a JTAG interface,
	// and these arrays hold the positions of the RAM pins in the data register
	unsigned int ramAddressBitPos[numRAMAddressBits];	///< position of RAM address bits in bsdr
	unsigned int ramDataBitPos[numRAMDataBits];			///< position of RAM data bits in bsdr
	unsigned int ramControlBitPos[numRAMControlBits];	///< position of RAM control bits in bsdr
};

#endif
