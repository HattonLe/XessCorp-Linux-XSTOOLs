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


#ifndef I2CPORTJTAG_H
#define I2CPORTJTAG_H

#include "i2cport.h"
#include "regiojtag.h"


/**
Performs master reads and writes of registers in devices on an I2C bus.

This object performs I2C bus transactions over a JTAG port.
It lets you read and write registers of a device at a given address on an I2C bus.

*/
class I2CPortJTAG : public I2CPort, public RegioJTAG
{
	public:

	I2CPortJTAG();

	I2CPortJTAG(XSError* e, unsigned int portNum, unsigned int endptNum, string brdModel);

	bool Setup(XSError* e, unsigned int portNum, unsigned int endptNum);

	void WriteBuffer(unsigned char* buffer, unsigned int numBytes1, ...);

	void ReadBuffer(unsigned int numWriteBytes, unsigned char* wrBuffer,
		unsigned int numReadBytes, unsigned char* rdBuffer);

	void WriteReg(unsigned char devAddr, unsigned char regAddr, unsigned char regData);

	void ReadReg(unsigned char devAddr, unsigned char regAddr, unsigned char* regData);


	private:
	
};

#endif
