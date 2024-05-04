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


#ifndef PPORT_H
#define PPORT_H

#include <wtypes.h>
#include "ioport.hpp"
using namespace Uniio;

#include "xserror.h"


/**
Low-level interface to the PC parallel port.

This object handles the parallel port. It lets you set the value of bit 
fields in the data and control registers of the parallel port, and it 
lets you get values from bit fields of the status register. These three 
byte-wide registers are concatenated into a single 24-bit register from 
which bit fields are extracted using upper and lower indices in the 
range [0,23]. The PPort object also lets you specify a 24-bit inversion 
mask to counter the effect of any inverters found in the PC parallel 
port and/or XS Board hardware. 

The PPort object uses lower-level I/O routines from either the TVICHW32, UNIIO or 
DLPORTIO DLL libraries and drivers. The TVICHW32 interface is used by 
default, but the user can select the UNIIO or DLPORTIO routines by setting the 
appropriate value in the XSTOOLS parameter file. 

The PPort object can be initialized with a parallel port number in the 
range [1,4] in which case it will fetch the actual hardware address for 
the parallel port registers from the PC RAM. This address can be 
overridden by specifying the hardware address in the XSTOOLS parameter 
file. 

The PPort object will check its operation by verifying that any levels 
on the parallel port pins match the values it has placed on them. This 
alerts the PPort object to any problems accessing the parallel port 
hardware. These checks cease after a set number of I/O operations in 
order to increase the port throughput. 
*/
class PPort
{
	public:

	PPort(void);

	PPort(XSError* e, unsigned int n, unsigned int invMask);

	~PPort(void);

	bool Setup(XSError* e, unsigned int n, unsigned int invMask);

	void SetErr(XSError* e);

	XSError& GetErr(void);

	PPort& operator=(PPort& src);

	bool SetLPTNum(unsigned int n);

	unsigned int GetLPTNum(void) const;

	void Out(unsigned int v, unsigned int loPos, unsigned int hiPos);

	unsigned int In(unsigned int loPos, unsigned int hiPos);

	void SetInvMask(unsigned int mask);

	void EnableJTAG(bool value);

	int StartBuffer(void);

	int FlushBuffer(void);

	bool IsBufferOn(void);


	private:

	unsigned char Inp(unsigned short regOffset);

	void Outp(unsigned short regOffset, unsigned char byte);

	IOPort *dataPort;				///< IO port object for the parallel port data register
	IOPort *controlPort;			///< IO port object for the parallel port control register
	IOPort *statusPort;				///< IO port object for the parallel port status register
	static HANDLE HW32;				///< TVICHW32 IO port handle
	XSError* err;					///< error reporting object
	unsigned int num;				///< parallel port num 1,2,3,4
	unsigned int address;			///< I/O address
	unsigned int invMask;			///< concatenated inversion mask for data, status, and control
	unsigned int chkCounter;		///< counts the number of initial checks to make on parallel port
	unsigned int IODriverIndex;		///< UNIIO, DRIVERLINX, TVICHW32
	unsigned int regvals;			///< store the values of the parallel port registers
	unsigned int updateCounter;		///< update regvals whenever this counter hits zero
	bool enableJTAG;				///< true if the pin should be set to enable JTAG ops in the parallel port interface
	bool enableJTAGPinSet;			///< true if the pin to enable JTAG ops has been set
};

int ScanPPort(XSError *err, bool *portNumExists);

#endif
