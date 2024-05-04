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


//#include <dos.h>

#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <ctime>
#include <string>
using namespace std;

#ifdef WIN32
#include <afxwin.h>
#endif

#include "osccyprt.h"
#include "utils.h"


static const unsigned int BitDuration = 3;	// programming bit duration


/// Create a Cypress prog. oscillator controller port.
OscCyPort::OscCyPort(void)
{
	;
}

/// Create a Cypress prog. oscillator controller port.
OscCyPort::OscCyPort(XSError* e,	///< pointer to error reporting object
				   unsigned int portNum,	///< parallel port number
				   unsigned int invMask,	///< inversion mask for the parallel port
				   unsigned int pos_oscSCLW, ///< bit position in parallel port of osc. clock write pin
				   unsigned int pos_oscSDAW, ///< bit position in parallel port of osc. data write pin
				   unsigned int pos_oscSCLR, ///< bit position in parallel port of osc. clock read pin
				   unsigned int pos_oscSDAR) ///< bit position in parallel port of osc. data read pin
{
	Setup(e,portNum,invMask,pos_oscSCLW,pos_oscSCLR,pos_oscSDAW,pos_oscSDAR);
}


/// Initialize members of the object.
int OscCyPort::Setup(XSError* e,	///< pointer to error reporting object
				   unsigned int portNum,	///< parallel port number
				   unsigned int invMask,	///< inversion mask for the parallel port
				   unsigned int pos_oscSCLW, ///< bit position in parallel port of osc. clock write pin
				   unsigned int pos_oscSDAW, ///< bit position in parallel port of osc. data write pin
				   unsigned int pos_oscSCLR, ///< bit position in parallel port of osc. clock read pin
				   unsigned int pos_oscSDAR) ///< bit position in parallel port of osc. data read pin
{
	posOscSCLW = pos_oscSCLW;
	posOscSCLR = pos_oscSCLR;
	posOscSDAW = pos_oscSDAW;
	posOscSDAR = pos_oscSDAR;
	return I2CPortLPT::Setup(e,portNum,invMask,pos_oscSCLW,pos_oscSDAW,pos_oscSCLR,pos_oscSDAR,BitDuration);
}


/// Reset the programmable oscillator.
void OscCyPort::ResetOsc(string jedecFileName) ///< JEDEC file containing the programming for the Cypress oscillator
{
	assert(jedecFileName.length()>0);
	ifstream is(jedecFileName.c_str());  // open JEDEC file
	assert(!(is.fail() || is.eof()));

	// char num[10];
	// string instructions;
	char c;
	unsigned cnt = 0;
	unsigned char byte = 0;
	int address;
	bool enable = false;
	while(!is.eof())
	{
		is >> c;
		if(isspace(c))
			continue;
		if(c=='*')
		{
			// PromptUser((string)"Found *\n",PROMPT_OKCANCEL);
			enable = true;
			continue;
		}
		if(enable==false)
			continue;
		switch(c)
		{
		case '0':
		case '1':
			byte = byte*2 + (c-'0');
			cnt = (cnt+1) % 8;
			if(cnt==0)
			{
				// instructions = "writing ";
				// sprintf(num,"%02x",byte); instructions += (string)num;
				// instructions += " to address ";
				// sprintf(num,"%02x",address); instructions += (string)num;
				// instructions += "\n";
				// PromptUser(instructions,PROMPT_OKCANCEL);
				unsigned char wrBuf[3];
				unsigned char numWr;
				wrBuf[0] = (0x69<<1);	// device address with /write-enable bit appended
				wrBuf[1] = address;		// register address
				wrBuf[2] = byte;		// data to be written into register
				numWr = 3;
				WriteBuffer(wrBuf,numWr,0);
				address++;
				byte = 0;
				// PromptUser((string)"Write successful!\n",PROMPT_OKCANCEL);
			}
			break;
		case 'L':
			is >> dec >> address;
			address /= 8;
			cnt = 0;
			byte = 0;
			// instructions = "L Address = ";
			// sprintf(num,"%d",address); instructions += (string)num;
			// instructions += "\n";
			// PromptUser(instructions,PROMPT_OKCANCEL);
			break;
		default:
			enable = false;
			break;
		}
	}
}


/// Write a value into a register of the programmable oscillator.
///\return true if the operation was successful, false if not
bool OscCyPort::WriteReg(unsigned regAddress, ///< address of register in the oscillator 
					unsigned value)	///< value to be loaded into the register
{
	unsigned char wrBuf[3];
	wrBuf[0] = (0x69<<1);	// I2C device address with /write-enable bit appended
	wrBuf[1] = regAddress;	// register address
	wrBuf[2] = value;		// data to be written into register
	WriteBuffer(wrBuf,3,0);
	return true;
}


/// Set the programmable oscillator frequency.
///\return true if the operation was successful, false if not
bool OscCyPort::SetOscFrequency(int div, ///< value to store in the divisor for the master frequency
						string jedecFileName) ///< JEDEC file with default programming for the oscillator
{
	XSError& err = GetErr();
	if(div<1)
	{
		err.SimpleMsg(XSErrorMajor,"Clock divisor must be greater than zero!!\n");
		return false;
	}
	if(div>128)
	{
		err.SimpleMsg(XSErrorMajor,"Clock divisor must be less than 128!!\n");
		return false;
	}
	
	ResetOsc(jedecFileName);
	WriteReg(0x0d,div);	// register address for CLKD divider
	return true;
}
