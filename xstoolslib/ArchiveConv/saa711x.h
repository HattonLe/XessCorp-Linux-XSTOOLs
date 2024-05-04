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


#ifndef SAA711X_H
#define SAA711X_H

#include <cassert>
#include <cstdarg>
#include <string>
using namespace std;

#include "i2cportlpt.h"


/**
Reads and writes the registers of the SAA711X video codec through an I2C bus interface.
*/
class SAA711X : I2CPortLPT
{
	public:

	SAA711X(void);

	SAA711X(XSError* e, unsigned int portNum, unsigned int invMask,
		unsigned int pos_saaSCLW, unsigned int pos_saaSDAW,
		unsigned int pos_saaSCLR, unsigned int pos_saaSDAR);

	int Setup(XSError* e, unsigned int portNum, unsigned int invMask,
		unsigned int pos_saaSCLW, unsigned int pos_saaSDAW,
		unsigned int pos_saaSCLR, unsigned int pos_saaSDAR);

	bool Configure(string& fileName);

	
	private:

	unsigned int posSAASCLW; ///< position of prog. osc. clock write pin
	unsigned int posSAASCLR; ///< position of prog. osc. clock read pin
	unsigned int posSAASDAW; ///< position of prog. osc. data write pin
	unsigned int posSAASDAR; ///< position of prog. osc. data read pin
};

#endif
