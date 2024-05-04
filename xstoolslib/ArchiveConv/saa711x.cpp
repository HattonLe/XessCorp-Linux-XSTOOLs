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

#include "saa711x.h"
#include "utils.h"


const unsigned int BitDuration = 30;	// programming bit duration
const unsigned deviceAddr = 0x42;		// I2C address for the SAA711X video decoder


/// Create an SAA711X controller port.
SAA711X::SAA711X(void)
{
	;
}


/// Create an SAA711X controller port.
SAA711X::SAA711X(XSError* e,	///< pointer to error reporting object
		   unsigned int portNum,		///< parallel port number
		   unsigned int invMask,		///< inversion mask for the parallel port
		   unsigned int pos_saaSCLW,	///< bit position in parallel port of osc. clock write pin
		   unsigned int pos_saaSDAW,	///< bit position in parallel port of osc. data write pin
		   unsigned int pos_saaSCLR,	///< bit position in parallel port of osc. clock read pin
		   unsigned int pos_saaSDAR) 	///< bit position in parallel port of osc. data read pin
{
	Setup(e,portNum,invMask,pos_saaSCLW,pos_saaSCLR,pos_saaSDAW,pos_saaSDAR);
}


/// Initialize an SAA711X controller port.
int SAA711X::Setup(XSError* e,	///< pointer to error reporting object
		   unsigned int portNum,		///< parallel port number
		   unsigned int invMask,		///< inversion mask for the parallel port
		   unsigned int pos_saaSCLW,	///< bit position in parallel port of osc. clock write pin
		   unsigned int pos_saaSDAW,	///< bit position in parallel port of osc. data write pin
		   unsigned int pos_saaSCLR,	///< bit position in parallel port of osc. clock read pin
		   unsigned int pos_saaSDAR)	///< bit position in parallel port of osc. data read pin
{
	posSAASCLW = pos_saaSCLW;
	posSAASCLR = pos_saaSCLR;
	posSAASDAW = pos_saaSDAW;
	posSAASDAR = pos_saaSDAR;
	return I2CPortLPT::Setup(e,portNum,invMask,pos_saaSCLW,pos_saaSDAW,pos_saaSCLR,pos_saaSDAR,BitDuration);
}


/// Load the registers of the SAA711X.
///\return true if the operation is successful, false if not.
bool SAA711X::Configure(string& fileName) ///< file containing values to load into video decoder registers
{
	assert(fileName.length()>0);

	ifstream is(fileName.c_str());  // open file with register addresses and values

	assert(!(is.fail() || is.eof()));

	int regAddr, regData, i;
	int regs[256];

	for(i=0; i<256; i++)
		regs[i] = -1;

	for(i=1; !is.eof(); i++)
	{
		is >> hex >> regAddr >> regData;
		regs[regAddr] = regData;	// save register values for comparison
		fprintf(stderr,"(%02x)<=%02x%c",regAddr,regData,(i%8)==0 ? '\n':' ');
		WriteReg(deviceAddr, regAddr, regData);
	}
	fprintf(stderr,"\n\n");

	unsigned char cRegData;

	for(i=1; i<=0x19; i++)
	{
		ReadReg(deviceAddr,i,&cRegData);
		if((cRegData != regs[i]) && (regs[i]!=-1))
			fprintf(stderr,"Mismatch: (%02x)=%02x instead of %02x\n",i,(int)cRegData,regs[i]);
	}
	for(i=0x30; i<=0x3A; i++)
	{
		ReadReg(deviceAddr,i,&cRegData);
		if((cRegData != regs[i]) && (regs[i]!=-1))
			fprintf(stderr,"Mismatch: (%02x)=%02x instead of %02x\n",i,(int)cRegData,regs[i]);
	}
	for(i=0x40; i<=0x62; i++)
	{
		ReadReg(deviceAddr,i,&cRegData);
		if((cRegData != regs[i]) && (regs[i]!=-1))
			fprintf(stderr,"Mismatch: (%02x)=%02x instead of %02x\n",i,(int)cRegData,regs[i]);
	}
	for(i=0x80; i<=0xEF; i++)
	{
		ReadReg(deviceAddr,i,&cRegData);
		if((cRegData != regs[i]) && (regs[i]!=-1))
			fprintf(stderr,"Mismatch: (%02x)=%02x instead of %02x\n",i,(int)cRegData,regs[i]);
	}

	ReadReg(deviceAddr,0x1F,&cRegData);
	fprintf(stderr,"Status = %02x\n",(int)cRegData);
	ReadReg(deviceAddr,0x00,&cRegData);
	fprintf(stderr,"SAA7114 version = %02x\n",(int)cRegData);

	return true;
}
