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


#ifndef AKCDCPRT_H
#define AKCDCPRT_H

#include <cassert>
#include <cstdarg>
#include <string>
using namespace std;

#include "pport.h"

/**
Sets the option registers of an AKM audio codec.
*/
class AKCodecPort : PPort
{
	public:

	AKCodecPort(void);

	AKCodecPort(XSError* e,
		unsigned int portNum,
		unsigned int invMask,
		unsigned int pos_cdcCCLK,
		unsigned int pos_cdcCSNN,
		unsigned int pos_cdcCDTI,
		unsigned int pos_cdcCDTO);

	int Setup(XSError* e,
		unsigned int portNum,
		unsigned int invMask,
		unsigned int pos_cdcCCLK,
		unsigned int pos_cdcCSNN, 
		unsigned int pos_cdcCDTI, 
		unsigned int pos_cdcCDTO); 

	void PulseCCLK(void);

	bool WriteReg(int regAddr,	
		int regData);	

	bool Configure(int *reg);

	private:

	unsigned int posCdcCCLK; ///< position of codec clock pin
	unsigned int posCdcCSNN; ///< position of codec chip-select pin
	unsigned int posCdcCDTI; ///< position of codec data input pin
	unsigned int posCdcCDTO; ///< position of codec data output pin
};

#endif
