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

#include <dos.h>

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

#include "oscport.h"
#include "utils.h"


// Some values for the DS1075 programmable oscillator
static const unsigned int OscCmdLength		= 8;	// length of oscillator commands 
static const unsigned int OscDataLength		= 9;	// length of oscillator data
static const unsigned int OscCmdWriteDIV	= 1;	// command for writing clock divisor 
static const unsigned int OscCmdWriteMUX	= 2;	// command for writing clock multiplexer 

static const unsigned int OneDuration		= 5L;	// duration of a logic 1 in the osc. data stream
static const unsigned int ZeroDuration		= 112L;	// duration of a logic 0
static const unsigned int BitDuration		= 150L;	// total bit duration


/// Create a DS1075 prog. oscillator controller port.
OscPort::OscPort(void)
{
	;
}


/// Create a DS1075 prog. oscillator controller port.
OscPort::OscPort(XSError* e,	///< pointer to error reporting object
				   unsigned int portNum,	///< parallel port number
				   unsigned int invMask,	///< inversion mask for the parallel port
				   unsigned int pos_osc)	///< bit position in parallel port of osc. pin
{
	Setup(e,portNum,invMask,pos_osc);
}


/// Setup a DS1075 prog. oscillator controller port.
int OscPort::Setup(XSError* e,	///< pointer to error reporting object
				   unsigned int portNum,	///< parallel port number
				   unsigned int invMask,	///< inversion mask for the parallel port
				   unsigned int pos_osc)	///< bit position in parallel port of osc. pin
{
	posOsc = pos_osc;
	return PPort::Setup(e,portNum,invMask);
}


/// Reset the programmable oscillator.
void OscPort::ResetOsc(void)
{
	Out(0,posOsc,posOsc);				// lower data bit 0
	InsertDelay(1200L,MICROSECONDS);	// wait for DS1075 to see reset level
	Out(1,posOsc,posOsc);				// now remove reset level
	InsertDelay(1200L,MICROSECONDS);	// wait for presence pulse from DS1075
}


/// Send a bit of data to the programmable oscillator.
void OscPort::SendOscBit(unsigned char b)
{
	if(b)	// send a logic 1
	{
		Out(0,posOsc,posOsc);
		InsertDelay(OneDuration,MICROSECONDS);
		Out(1,posOsc,posOsc);
		InsertDelay(BitDuration-OneDuration,MICROSECONDS);
	}
	else	// send a logic 0
	{
		Out(0,posOsc,posOsc);
		InsertDelay(ZeroDuration,MICROSECONDS);
		Out(1,posOsc,posOsc);
		InsertDelay(BitDuration-ZeroDuration,MICROSECONDS);
	}
}


/// Send a command word to the programmable oscillator.
void OscPort::IssueOscCmd(unsigned int cmd)
{
	for(int i=0; i<OscCmdLength; i++, cmd>>=1)
		SendOscBit(cmd&1);
}


/// Send a data word to the programmable oscillator.
void OscPort::SendOscData(unsigned int data)
{
	for(int i=0; i<OscDataLength; i++, data>>=1)
		SendOscBit(data&1);
}


/// Set the programmable oscillator frequency.
bool OscPort::SetOscFrequency(int div,	///< divisor for master frequency
					bool extOscPresent)	///< if true, master frequency arrives from external source, otherwise use internal 100 MHz source in DS1075
{
	XSError& err = GetErr();
	if(div<1)
	{
		string msg("Clock divisor must be greater than zero!!\n");
		err.SimpleMsg(XSErrorMajor,msg);
		return false;
	}
	if(div>2052)
	{
		string msg("Clock divisor must be less than 2053!!\n");
		err.SimpleMsg(XSErrorMajor,msg);
		return false;
	}

	int div1 = div==1 ? 1:0;	// set divide-by-1 bit if clock is not divided down
	int prescaleOff = 1;		// turn off prescalar
	int prescaleByTwo = 0;
	if(!extOscPresent)		// setup prescalar circuitry if DS1075 internal oscillator is used
	{
		if(div>1026)
		{
			prescaleOff = 0;	// turn on the prescalar
			prescaleByTwo = 0;	// enable divide-by-four by turning off the divide-by-two prescalar
			div /= 4;			// reduce divisor
		}
		else if(div>513)
		{
			prescaleOff = 0;	// turn on the prescalar
			prescaleByTwo = 1;	// turn on the divide-by-two prescalar
			div /= 2;
		}
	}
	div = div>513 ? 513:div;	// divisor saturates at 513 if external oscillator is used
	// The divisor must be adjusted to get the frequency right as follows:
	//		original	adjusted	resulting
	//		div			div			frequency
	//		----------------------------------
	//		1			 -1			100 MHz (no division)
	//		2			  0			 50 MHz (divide by 2)
	//		3			  1			 33 MHz (divide by 3)
	//		4			  2			 25 MHz (divide by 4)
	//		...			...			...
	//		511			509			
	//		512			510			
	//		513			511
	div -= 2;

	int powerUp = 1;		// keep the oscillator powered up
	int disable0 = 1;		// disable the OUT0 output of the DS1075 (it's not connected on the XS Boards)
	int mux = (disable0<<5) | (powerUp<<4) | (prescaleByTwo<<3) |(prescaleOff<<2) | (div1<<1) | (extOscPresent ? 1:0);

	Out(1,posOsc,posOsc);	// make sure the osc. data pin starts out high

	// program divisor register of the DS1075
	ResetOsc();
	IssueOscCmd(OscCmdWriteDIV);
	SendOscData(div);
	InsertDelay(240,MICROSECONDS);

	// program multiplexor register of the DS1075
	ResetOsc();
	IssueOscCmd(OscCmdWriteMUX);
	SendOscData(mux);
	InsertDelay(240,MICROSECONDS);

	return true;
}
