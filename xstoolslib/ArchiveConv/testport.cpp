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

#include "testport.h"


/// Create a test vector port.
TestPort::TestPort(void)
{
	;
}


/// Create a test vector port.
TestPort::TestPort(XSError* e,	///< pointer to error reporting object
				   unsigned int portNum,	///< parallel port number
				   unsigned int invMask,	///< inversion mask for the parallel port
				   unsigned int posClk,		///< bit position of clock for entering test vectors
				   unsigned int posDolo,	///< lower bit position in test vector parallel port data pins
				   unsigned int posDohi,	///< lower bit position in test vector parallel port data pins
				   unsigned int posDilo,	///< lower bit position in test vector parallel port status pins
				   unsigned int posDihi)	///< lower bit position in test vector parallel port status pins
{
	Setup(e,portNum,invMask,posClk,posDolo,posDohi,posDilo,posDihi);
}


/// Setup a test vector port.
bool TestPort::Setup(XSError* e,	///< pointer to error reporting object
				   unsigned int portNum,	///< parallel port number
				   unsigned int invMask,	///< inversion mask for the parallel port
				   unsigned int posClk,		///< bit position of clock for entering test vectors
				   unsigned int posDolo,	///< lower bit position in test vector parallel port data pins
				   unsigned int posDohi,	///< lower bit position in test vector parallel port data pins
				   unsigned int posDilo,	///< lower bit position in test vector parallel port status pins
				   unsigned int posDihi)	///< lower bit position in test vector parallel port status pins
{
	posCLK = posClk;
	posDOLO = posDolo;
	posDOHI = posDohi;
	posDILO = posDilo;
	posDIHI = posDihi;
	return PPort::Setup(e,portNum,invMask);
}


/// Send a test vector thru the parallel port data pins and get the values on the status pins.
///\return the value on the parallel port status pins.
unsigned char TestPort::ApplyTestVector(unsigned char v,	///< value to output on parallel port data pins
						unsigned char mask)					///< mask for output value
{
	char in = In(posDOLO,posDOHI);					// get current output value on parallel port data pins
	Out((v & mask) | (in & ~mask),posDOLO,posDOHI);	// output vector onto masked data pins
	return In(posDILO,posDIHI);						// return levels on status pins
}


/// Get the current test vector being output on the parallel port data pins.
///\return the value on the parallel port data pins
unsigned char TestPort::GetTestVector(void)
{
	return In(posDOLO,posDOHI);
}
