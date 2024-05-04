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


#ifndef AT49FPORT_H
#define AT49FPORT_H

#include "flashprt.h"


/**
Uploading and downloading of hexadecimal records to the AT49* Flash memory on an XSA-50, XSA-100 or XSB Board.

This object specializes the FlashPort object by providing methods for erasing blocks and writing individual bytes of an Atmel AT49FXXX Flash memory.
*/
class AT49FPort : public FlashPort
{
	public:

	 AT49FPort(void);

	 AT49FPort(XSError* e, 
		unsigned int portNum, 
		unsigned int invMask, 
		unsigned int pos_reset, 
		unsigned int pos_clk, 
		unsigned int pos_dolsb, 
		unsigned int pos_domsb, 
		unsigned int pos_dilsb, 
		unsigned int pos_dimsb, 
		unsigned int pos_stlsb, 
		unsigned int pos_stmsb) 
		;

	bool ProgramFlash(
		unsigned int address,
		unsigned int data,
		bool bigEndianBytes,	
		bool bigEndianBits		
		);

	bool EraseFlash(void);

	bool EraseFlashBlock(unsigned int blockIndex);

	bool ResetFlash(void);
};

#endif
