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


#ifndef OSCPORT_H
#define OSCPORT_H

#include <cassert>
#include <cstdarg>
#include "pport.h"


/**
Set of the divisor on the Dallas/Maxim programmable oscillator, thus determining its output frequency.
 
This object provides a method for programming the Dallas DS1075 
oscillator chip. The specified divisor for the master frequency is 
partitioned into a divisor of 513 or less and a frequency prescaler of 2 
or 4 (if necessary). The divisor value is programmed into the divisor 
register of the oscillator by sending a precisely timed waveform through 
the parallel port pin that connects to the oscillator chip output (which 
operates as an input during the DS1075 programming mode.) Then the 
prescaler value and the bit that selects the source for the master 
oscillator are loaded into the multiplexer register. 

*/
class OscPort : PPort
{
	public:

	OscPort(void);

	OscPort(XSError* e,
		unsigned int portNum,
		unsigned int invMask,
		unsigned int pos_osc);

	int Setup(XSError* e,
		unsigned int portNum,
		unsigned int invMask,
		unsigned int pos_osc);

	void ResetOsc(void);

	void SendOscBit(unsigned char b);

	void IssueOscCmd(unsigned int cmd);

	void SendOscData(unsigned int data);

	bool SetOscFrequency(int div, bool extOscPresent);

	
	private:

	unsigned int posOsc; ///< position of prog. osc. clock pin
};

#endif
