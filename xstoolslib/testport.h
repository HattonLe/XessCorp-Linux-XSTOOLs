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


#ifndef TESTPORT_H
#define TESTPORT_H

#include <cassert>
#include "pport.h"


/**
Apply test vectors to the FPGA through the parallel port.

This object provides a method of sending test vectors to the FPGA
through the parallel port.  Two nybbles are clocked into the CPLD
that controls the parallel port interface to create a complete,
byte-wide vector.

*/
class TestPort : PPort
{
	public:

	TestPort(void);

	TestPort(XSError* e, unsigned int portNum, unsigned int invMask,
		unsigned int posClk, unsigned int posDolo, unsigned int posDohi,
		unsigned int posDilo, unsigned int posDihi);

	bool Setup(XSError* e, unsigned int portNum, unsigned int invMask,
		unsigned int posClk, unsigned int posDolo, unsigned int posDohi,
		unsigned int posDilo, unsigned int posDihi);

	unsigned char ApplyTestVector(unsigned char vector, unsigned char mask);

	unsigned char GetTestVector(void);
	
	private:

	unsigned int posCLK;		///< bit position of clock for entering test vectors
	unsigned int posDOLO;		///< lower bit position in test vector parallel port data pins
	unsigned int posDOHI;		///< upper bit position in test vector parallel port data pins
	unsigned int posDILO;		///< lower bit position in test vector parallel port status pins
	unsigned int posDIHI;		///< upper bit position in test vector parallel port status pins
};

#endif
