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


#ifndef REGIOJTAG_H
#define REGIOJTAG_H

#include "usbjtag.h"


/**
Performs reads and writes of registers in devices through the JTAG port.

This object performs reads and writes of registers through a JTAG port.

*/
class RegioJTAG : public USBJTAG
{
	public:

	RegioJTAG();

	RegioJTAG(XSError* e, unsigned int portNum, unsigned int endptNum, string brdModel);

	bool Setup(XSError* e, unsigned int portNum, unsigned int endptNum, string brdModel);

	void Write(unsigned long addr, unsigned long data, bool startup=true, bool teardown=true);

	unsigned long Read(unsigned long addr, bool startup=true, bool teardown=true);


	private:
	
	string userInstruction;		/// JTAG USER instruction opcode
	unsigned addrWidth;			/// width of address field in opcode for R/W of registers
	unsigned dataWidth;			/// width of data field in opcode for R/W of registers
};

#endif
