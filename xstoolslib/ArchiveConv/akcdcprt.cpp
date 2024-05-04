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
#include <fstream>
#include <sstream>
#include <ctime>
#include <string>
using namespace std;

#ifdef WIN32
#include <afxwin.h>
#endif

#include "akcdcprt.h"
#include "utils.h"


const unsigned int BitDuration = 3;	// programming bit duration


/// Create a codec controller port.
AKCodecPort::AKCodecPort(void)
{
	;
}

/// Create a codec controller port.
AKCodecPort::AKCodecPort(XSError* e,	///< pointer to error reporting object
				   unsigned int portNum,	///< parallel port number
				   unsigned int invMask,	///< inversion mask for the parallel port
				   unsigned int pos_cdcCCLK, ///< bit position in parallel port of clock pin
				   unsigned int pos_cdcCSNN, ///< bit position in parallel port of chip-select pin
				   unsigned int pos_cdcCDTI, ///< bit position in parallel port of data input pin
				   unsigned int pos_cdcCDTO) ///< bit position in parallel port of data output pin
{
	Setup(e,portNum,invMask,pos_cdcCCLK,pos_cdcCSNN,pos_cdcCDTI,pos_cdcCDTO);
}



/// Setup a codec controller port.
int AKCodecPort::Setup(XSError* e,	///< pointer to error reporting object
				   unsigned int portNum,	///< parallel port number
				   unsigned int invMask,	///< inversion mask for the parallel port
				   unsigned int pos_cdcCCLK, ///< bit position in parallel port of clock pin
				   unsigned int pos_cdcCSNN, ///< bit position in parallel port of chip-select pin
				   unsigned int pos_cdcCDTI, ///< bit position in parallel port of data input pin
				   unsigned int pos_cdcCDTO) ///< bit position in parallel port of data output pin
{
	posCdcCCLK = pos_cdcCCLK;
	posCdcCSNN = pos_cdcCSNN;
	posCdcCDTI = pos_cdcCDTI;
	posCdcCDTO = pos_cdcCDTO;
	return PPort::Setup(e,portNum,invMask);
}


/// Pulse the codec clock.
void AKCodecPort::PulseCCLK(void)
{
	Out(~In(posCdcCCLK,posCdcCCLK),posCdcCCLK,posCdcCCLK);
	Out(~In(posCdcCCLK,posCdcCCLK),posCdcCCLK,posCdcCCLK);
}


/// Write a value to a codec register.
bool AKCodecPort::WriteReg(int regAddr,	///< register address
						int regData)		///< data to write to register
{
	int i;
	
	Out(1,posCdcCSNN,posCdcCSNN);	// disable codec
	Out(0,posCdcCCLK,posCdcCCLK);	// lower codec clock

	Out(0,posCdcCSNN,posCdcCSNN);	// enable codec

	// output the 3-bit write command "111"
	Out(1,posCdcCDTI,posCdcCDTI);
	PulseCCLK();
	PulseCCLK();
	PulseCCLK();

	// output the 5-bit register address starting with the LSB
	for(i=0; i<5; i++)
	{
		Out((regAddr>>i)&1,posCdcCDTI,posCdcCDTI);
		PulseCCLK();
	}

	// output the 8-bit register value starting with the LSB
	for(i=0; i<8; i++)
	{
		Out((regData>>i)&1,posCdcCDTI,posCdcCDTI);
		PulseCCLK();
	}

	Out(1,posCdcCSNN,posCdcCSNN);	// disable codec

	return true;
}


/// Configure the codec.
bool AKCodecPort::Configure(int *reg) ///< pointer to 8 values to be written to registers 0..7
{
	XSError& err = GetErr();

	for( int i=0; i<8; i++)
		WriteReg(i,reg[i]);
				
	return true;
}
