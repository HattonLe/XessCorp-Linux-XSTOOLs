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


#ifndef OSCCYPRT_H
#define OSCCYPRT_H

#include <cassert>
#include <cstdarg>
#include <string>
using namespace std;

#include "i2cportlpt.h"


/**
Set of the divisor on the Cypress programmable oscillator, thus determining its output frequency.

This object provides a method for programming the Cypress CY22393FC
oscillator chip. This chip provides six clock outputs, but this object
only sets the divisor for a 100 MHz master clock to generate a slower clock on the CLKD output.

*/
class OscCyPort : I2CPortLPT
{
	public:

	OscCyPort(void);

	OscCyPort(XSError* e, unsigned int portNum, unsigned int invMask,
		unsigned int pos_oscSCLW, unsigned int pos_oscSDAW,
		unsigned int pos_oscSCLR, unsigned int pos_oscSDAR);

	int Setup(XSError* e, unsigned int portNum, unsigned int invMask,
		unsigned int pos_oscSCLW, unsigned int pos_oscSDAW,
		unsigned int pos_oscSCLR, unsigned int pos_oscSDAR);

	void ResetOsc(string jedecFileName);

	bool WriteReg(unsigned regAddress, unsigned value);

	bool SetOscFrequency(int div, string jedecFileName);

	
	private:

	unsigned int posOscSCLW; ///< position of prog. osc. clock write pin
	unsigned int posOscSCLR; ///< position of prog. osc. clock read pin
	unsigned int posOscSDAW; ///< position of prog. osc. data write pin
	unsigned int posOscSDAR; ///< position of prog. osc. data read pin
};

#endif
