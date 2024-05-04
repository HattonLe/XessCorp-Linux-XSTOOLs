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

	�1997-2010 - X Engineering Software Systems Corp.
----------------------------------------------------------------------------------*/


#include <cstdlib>
#include <conio.h>
#include <cassert>
#include <sstream>
#include <fstream>
#include <iostream>
using namespace std;

#include "physmem.hpp"
#include <stdafx.h>		// added to remove windows.h include problem
#include "osiface.hpp"
#include "btrace.hpp"
#include "DLPORTIO.H"
#include "tvichw32.h"

#include "pport.h"
#include "utils.h"


#define TVICHW_VERSION_6_0


enum {UNIIO, DRIVERLINX, TVICHW32, NO_LPT};		// indices for supported parallel port drivers

static const unsigned short DATAREG = 0;		// offset of data register from LPT port base address
static const unsigned short STATREG = 1;		// offset of status register from LPT port base address
static const unsigned short CTRLREG = 2;		// offset of control register from LPT port base address

// Global parameters for the parallel port
static const unsigned int minPortNum = 1;		// minimum parallel port #
static const unsigned int maxPortNum = 4;		// maximum parallel port #

HANDLE PPort::HW32;


// Some macros for handling bit fields
#define LOWFIELDMASK(lo,hi)	((1<<((hi)-(lo)+1))-1)
#define FIELDMASK(lo,hi)	(LOWFIELDMASK(lo,hi)<<(lo))

static const unsigned int minBitPos = 0;	// minimum index into parallel port register bits
static const unsigned int maxBitPos = 23;	// maximum index into parallel port register bits

static const unsigned int enableJTAGPinPos = 5;	// index of pin that enables/disables JTAG (pin D6)


// Get the value stored in a bit field.
static unsigned int GetField(unsigned int data,		// port data
							 unsigned int loPos,	// low bit position of field
							 unsigned int hiPos)		// high bit position of field
{
	assert(loPos<=maxBitPos);
	assert(hiPos<=maxBitPos);
	assert(hiPos>=loPos);
	return (FIELDMASK(loPos,hiPos) & data) >> loPos;
}

// Set the value in a bit field and return the entire data value after the field is updated.
static unsigned int SetField(unsigned int data,		// port data
							 unsigned int loPos,	// low bit position of field
							 unsigned int hiPos,	// high bit position of field
							 unsigned int newData)	// new data for field
{
	assert(loPos<=maxBitPos);
	assert(hiPos<=maxBitPos);
	assert(hiPos>=loPos);
	return (data & ~FIELDMASK(loPos,hiPos)) |
		((newData & FIELDMASK(0,hiPos-loPos))<<loPos);
}


/// Constructor for a parallel port object.
PPort::PPort(void)
{
	dataPort = statusPort = controlPort = NULL;
	PPort::HW32 = 0;
}


/// Constructor for a parallel port object.  
/// Inversion masks are used to correct or the effects of the various inverters in the PC and XS Board.
PPort::PPort(XSError* e,				///< error reporting channel 
			 unsigned int n,			///< parallel port number
			 unsigned int invMask)		///< inversion mask for data, status, and control output bits
{
	dataPort = statusPort = controlPort = NULL;
	PPort::HW32 = 0;
	Setup(e,n,invMask);
}


/// Destructor for parallel port object.
PPort::~PPort(void)
{
	if(dataPort != NULL) delete dataPort;
	if(statusPort != NULL) delete statusPort;
	if(controlPort != NULL) delete controlPort;
	if(PPort::HW32!= 0)
	{
		PPort::HW32 = CloseTVicHW32(PPort::HW32);
		assert(PPort::HW32==0);
	}
}


/// Initialize the object.
bool PPort::Setup(XSError* e,			///< error reporting channel 
			 unsigned int n,			///< parallel port number
			 unsigned int invMask)		///< inversion mask for data, status, and control output bits
{
	assert(sizeof(unsigned int)/sizeof(char) >= 3);	// value needs to be at least 24-bits wide

	// find out which parallel port driver to use
	string paramName("LPTDRIVER");
	string driverName = GetXSTOOLSParameter(paramName);
	if(driverName =="DRIVERLINX")
		IODriverIndex = DRIVERLINX;
	else if(driverName == "TVICHW32")
		IODriverIndex = TVICHW32;
	else if(driverName == "NO_LPT")
		IODriverIndex = NO_LPT;
	else // UNIIO is the default
		IODriverIndex = UNIIO;

	// see if the parallel port should be checked for correct operation
	char s[20];
	sprintf(s,"LPT%1dCHECK",n);	// get the parallel port check flag from the parameter file
	paramName = (string)s;
	if(GetXSTOOLSParameter(paramName) == "NO")
		chkCounter = 0;				// don't bother to check the port for correct operation
	else
		chkCounter = 100;			// number of times to initially check port for correct operation

	updateCounter = 0;

	enableJTAG = false;				// disable JTAG operations in the parallel port interface by default
	enableJTAGPinSet = false;		// the pin that enables/disables JTAG ops has not been set, yet

	SetErr(e);						// set error reporting channel
	SetInvMask(invMask);			// set read, write inversion masks
	return SetLPTNum(n);			// return false if could not assign to the given parallel port address
}


/// Sets the error reporting channel.
void PPort::SetErr(XSError* e)		///< error reporting channel
{
	err = e;
}


/// Provides access to the error reporting channel.
XSError& PPort::GetErr(void)
{
	return *err;
}


/// Assignment operator for parallel port objects.
PPort& PPort::operator=(PPort& src)
{
	dataPort		= src.dataPort;
	controlPort		= src.controlPort;
	statusPort		= src.statusPort;
	err				= src.err;
	num				= src.num;
	address			= src.address;
	invMask			= src.invMask;
	chkCounter		= src.chkCounter;
	IODriverIndex	= src.IODriverIndex;
	regvals			= src.regvals;
	updateCounter	= src.updateCounter;
	enableJTAG      = src.enableJTAG;
	enableJTAGPinSet = src.enableJTAGPinSet;

	return *this;
}


/// Sets up a port object for a given parallel port number.
///\return true if parallel port was setup correctly, false if some error occurred.
bool PPort::SetLPTNum(unsigned int n)	///< parallel port number or I/O address
{
	num = (n <= maxPortNum) ? n : 1;	// set parallel port number to 1 if n is a hardware address instead of port #
	
	// find the I/O address for the given parallel port
	if(n <= maxPortNum)
	{
		char s[20];
		sprintf(s,"LPT%1dADDRESS",n);
		string paramName(s);
		address = 0;
		sscanf(GetXSTOOLSParameter(paramName).c_str(),"%x",&address);
	}
	else
		address = n;
	
	switch(IODriverIndex)
	{
	case TVICHW32:
		if(PPort::HW32==0)
		{ // only activate the driver if it is currently inactive
#ifdef TVICHW_VERSION_6_0
			PPort::HW32 = OpenTVicHW();
#else // TVICHW versions 5.x and earlier
			PPort::HW32 = OpenTVicHW32(PPort::HW32, "TVICHW32","TVicDevice0");
			if(PPort::HW32==0 || !GetActiveHW(PPort::HW32))
				PPort::HW32 = OpenTVicHW32(PPort::HW32, "TVICHW32","TVicDevice1");
#endif
			// handle the case where the driver can't be started
			if(PPort::HW32==0)
				err->SimpleMsg(XSErrorMinor,"All TVicHW32 handles are in use!!\n");
			else if(!GetActiveHW(PPort::HW32))
			{
				err->SimpleMsg(XSErrorMinor,"All TVicHW32 handles are in use!!\n");
				PPort::HW32 = 0;
			}
		}
		if(PPort::HW32!=0)
		{
			if((n<=GetLPTNumPorts(PPort::HW32)) && (address==0))
			{
				SetLPTNumber(PPort::HW32,n);
				address = GetLPTBasePort(PPort::HW32);
				// Use soft access for higher speeds.  This is needed to generate
				// timed waveforms like those used to program Atmel serial EEPROMs
				// and Dallas programable oscillators.
				SetHardAccess(PPort::HW32,FALSE);
			}
			else
			{
				if(n <= maxPortNum)
					SetLPTNumber(PPort::HW32,n);
				else
					SetLPTNumber(PPort::HW32,1);
				SetHardAccess(PPort::HW32,FALSE);
			}
		}
		break;
	case DRIVERLINX:
		{
			if(address == 0)
			{
				char lptAddresses[8];
				PhysicalMemory lptAddrBlock(0x00000408,8,false);
				lptAddrBlock.readMemoryBlock((void*)lptAddresses,8, 0);
				address = (lptAddresses[(num-1)*2+1]<<8) + lptAddresses[(num-1)*2];
			}
			break;
		}
	case NO_LPT:
		{
			address = 0; // no LPT installed or used
			break;
		}
	case UNIIO:
	default:
		{
			if(address == 0)
			{
				char lptAddresses[8];
				PhysicalMemory lptAddrBlock(0x00000408,8,false);
				lptAddrBlock.readMemoryBlock((void*)lptAddresses,8, 0);
				address = (lptAddresses[(num-1)*2+1]<<8) + lptAddresses[(num-1)*2];
			}
			OSInterface& o = OSInterface::osinterface(); // this just gets the driver running on WinNT, 2000, XP
			dataPort = new IOPort(address,true);		// writeable data port
			statusPort = new IOPort(address+1,false);	// readable status port
			controlPort = new IOPort(address+2,true);	// writeable control 
			assert((dataPort!=NULL) && (statusPort!=NULL) && (controlPort!=NULL));
			break;
		}
	}
	
	if(address==0)
		return false;	// return false if no parallel port is at this address

	return true;		// return true if a parallel port was found
}


/// Gets the parallel port number for a parallel port object.
///\return the parallel port number.
unsigned int PPort::GetLPTNum(void) const
{
	return num;
}


/// Get data byte from a given register at the LPT base address.
///\return data from the parallel port register.
unsigned char PPort::Inp(unsigned short regOffset /**< offset into parallel port register set */)
{
	assert(regOffset < 3);

	// Set the pin on the parallel port that enables/disables JTAG operations if it has not already been set.
	if(enableJTAGPinSet == false)
	{
		enableJTAGPinSet = true; // Avoid a recursive spiral by saying it is set before it actually is
		Out(enableJTAG,enableJTAGPinPos,enableJTAGPinPos); // now set it
	}

	switch(IODriverIndex)
	{
	case DRIVERLINX:
		return DlPortReadPortUchar(address + regOffset);
		break;
	case TVICHW32:
		return GetPortByte(PPort::HW32, address + regOffset);
		break;
	case NO_LPT:
		return 0;
		break;
	case UNIIO:
	default:
		switch(regOffset)
		{
		case DATAREG:
			return dataPort->readChar();
			break;
		case STATREG:
			return statusPort->readChar();
			break;
		case CTRLREG:
			return controlPort->readChar();
			break;
		default:
			assert(1==0);
			break;
		}
		break;
	}
	return 0;
}


/// Send data byte to a given register at the LPT base address.
void PPort::Outp(unsigned short regOffset,	///< offset into parallel port register set
				unsigned char byte)			///< data to write to the register
{
	assert(regOffset < 3);

	// Set the pin on the parallel port that enables/disables JTAG operations if it has not already been set.
	if(	enableJTAGPinSet == false)
	{
		enableJTAGPinSet = true; // Avoid a recursive spiral by saying it is set before it actually is
		Out(enableJTAG,enableJTAGPinPos,enableJTAGPinPos); // now set it
	}

	switch(IODriverIndex)
	{
	case DRIVERLINX:
		DlPortWritePortUchar(address + regOffset, byte);
		break;
	case TVICHW32:
		SetPortByte(PPort::HW32, address + regOffset, byte);
		break;
	case NO_LPT:
		break;
	case UNIIO:
	default:
		switch(regOffset)
		{
		case DATAREG:
			dataPort->write(byte);
			break;
		case STATREG:
			statusPort->write(byte);
			break;
		case CTRLREG:
			controlPort->write(byte);
			break;
		default:
			assert(1==0);
			break;
		}
		break;
	}
}


/// Output a value on the designated pins of the concatenated 24-bit parallel port field.
/// Bits  0 -  7:	data pins
/// Bits 15 -  8:	status pins
/// Bits 23 - 16:	control pins
void PPort::Out(unsigned int v,		///< value to output
				unsigned int loPos,	///< low bit position of field
				unsigned int hiPos)	///< high bit position of field
{
	unsigned int d, new_d;
	assert(loPos<=23);
	assert(hiPos<=23);
	assert(loPos<=hiPos);
	if((updateCounter & 0xF)==0 || chkCounter!=0)
	{
		if(hiPos<8)
			regvals = (regvals & ~0x0000FF) | Inp(0);
		else if(loPos>15)
			regvals = (regvals & ~0xFF0000) | (Inp(2)<<16);
		else if(loPos>=8 && hiPos<=15)
			regvals = (regvals & ~0x00FF00) | (Inp(1)<<8);
		else
			regvals = (Inp(2)<<16) | (Inp(1)<<8) | Inp(0);
	}
	updateCounter++;
	
	d = regvals;
	new_d = (d & ~FIELDMASK(loPos,hiPos)) | (((v<<loPos)^invMask) & FIELDMASK(loPos,hiPos));
	regvals = new_d;

	if(hiPos<8)
		Outp(0,new_d);
	else if(loPos>15)
		Outp(2,new_d>>16);
	else if(loPos>=8 && hiPos<=15)
		Outp(1,new_d>>8);
	else
	{
		Outp(0,new_d);
		Outp(1,new_d>>8);
		Outp(2,new_d>>16);
	}

	if(chkCounter > 0)
	{ // check the value output on the port to make sure it matches the value that was sent
		// this check is only performed for the first few uses of the parallel port
		chkCounter--;
		if(hiPos<8)
		{
			d = Inp(0);
			new_d &= 0xff;	// zero the unused part of new output data
		}
		else if(loPos>15)
		{
			d = Inp(2)<<16;
			new_d &= 0xff0000;	// zero the unused part of new output data
		}
		else if(loPos>=8 && hiPos<=15)
		{
			d = Inp(1)<<8;
			new_d &= 0xff00;	// zero the unused part of new output data
		}
		else
		{
			d = Inp(0) | (Inp(1)<<8) | (Inp(2)<<16);
			new_d &= 0xffffff;	// zero the unused part of new output data
		}
		if(d != new_d)
		{
			err->SimpleMsg(XSErrorFatal,"Parallel port not responding!!\n\nCHECK YOUR PARALLEL PORT HARDWARE ADDRESS!!\n");
		}
	}
}


/// Return the current values on the designated pins of the concatenated 24-bit parallel port field.
/// Bits  0 -  7:	data pins
/// Bits 15 -  8:	status pins
/// Bits 23 - 16:	control pins
///\return the bit values in the selected bit field of the parallel port registers
unsigned int PPort::In(unsigned int loPos,	///< low bit position of field
						unsigned int hiPos)	///< high bit position of field
{
	unsigned int d;
	assert(loPos<=23);
	assert(hiPos<=23);
	assert(loPos<=hiPos);
	if(hiPos<8)
	{
		d = Inp(0);
		regvals = (regvals & ~0x0000FF) | d;
	}
	else if(loPos>15)
	{
		d = Inp(2)<<16;
		regvals = (regvals & ~0xFF0000) | d;
	}
	else if(loPos>=8 && hiPos<=15)
	{
		d = Inp(1)<<8;
		regvals = (regvals & ~0x00FF00) | d;
	}
	else
	{
		d = Inp(0) | (Inp(1)<<8) | (Inp(2)<<16);
		regvals = d;
	}
	return ((d^invMask) & FIELDMASK(loPos,hiPos)) >> loPos;
}


/// Set the inversion mask for the concatenated 24-bit parallel port field.
/// Bits  0 -  7:	inversion mask for data pins
/// Bits 15 -  8:	inversion mask for status pins
/// Bits 23 - 16:	inversion mask for control pins
void PPort::SetInvMask(unsigned int mask)
{
	invMask = mask;
}


/// Set/clear flag to enable or disable JTAG operations thru the parallel port.
void PPort::EnableJTAG(bool value)
{
	enableJTAG = value;
	enableJTAGPinSet = false;	// indicate that the pin has not been set to the new value, yet
}


/// Start collecting commands into the buffer.
int PPort::StartBuffer(void)
{
	/// There is no buffering implemented with the parallel port.
	return true;
}


/// Stop collecting commands into the buffer and transmit the buffer.
int PPort::FlushBuffer(void)
{
	/// There is no buffering implemented with the parallel port.
	return true;
}

/// Determine if command buffer is on or off.
bool PPort::IsBufferOn(void)
{
	/// There is no buffering implemented with the parallel port.
	return false;
}



/// Scan parallel ports for their existence
///\return the number of parallel ports found
int ScanPPort(XSError *err,			///< pointer to error reporting object
			  bool *portNumExists)	///< pointer to array of parallel port numbers
{
	static PPort *p = NULL;

	// Only create this object once because it cannot be destroyed without closing
	// the TVICHW32 handles used by any other objects in this application.
	if(p==NULL)
		p = new PPort;	// Create a parallel port object that's not assigned to an actual port.

	int numPPorts = 0;

	// Scan through all the possible parallel ports.
	for(int i=minPortNum; i<=maxPortNum; i++)
	{
		// See if the object can setup on a particular port
		if(p->Setup(err,i,0))
		{
			portNumExists[i] = true;	// Setup worked, so flag the port number as one that exists.
			numPPorts++;
		}
		else
			portNumExists[i] = false;	// Setup failed, so clear the port number flag.
	}

	// Don't close the parallel port object!  This closes the TVICHW32 handles used by other objects.

	return numPPorts;
}
