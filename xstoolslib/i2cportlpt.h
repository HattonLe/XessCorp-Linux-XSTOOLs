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


#ifndef I2CPORTLPT_H
#define I2CPORTLPT_H

#include "i2cport.h"
#include "pport.h"


/**
Performs master reads and writes of registers in devices on an I2C bus.

This object performs I2C bus transactions over a parallel port.
It lets you read and write registers of a device at a given address on an I2C bus.

*/
class I2CPortLPT : public I2CPort, public PPort
{
	public:

	I2CPortLPT();

	I2CPortLPT(XSError* e,
		unsigned int portNum, 
		unsigned int invMask,		
		unsigned int sclwPosArg, 
		unsigned int sdawPosArg, 
		unsigned int sclrPosArg, 
		unsigned int sdarPosArg, 
		unsigned int bitDurArg);

	bool Setup(XSError* e, 
		unsigned int portNum, 
		unsigned int invMask,		
		unsigned int sclwPosArg, 
		unsigned int sdawPosArg, 
		unsigned int sclrPosArg, 
		unsigned int sdarPosArg, 
		unsigned int bitDurArg);

	private:

	void SetSCL(unsigned int bit, bool waitForSlave = true);

	unsigned int GetSCL();

	void PulseSCL(void);

	void SetSDA(unsigned int bit);

	unsigned int GetSDA();

	void StartTransfer();

	void WriteBit(unsigned int bit);

	unsigned int ReadBit(void);

	void WriteAck(unsigned int ack);

	unsigned int ReadAck();

	void StopTransfer();

	void WriteByte(unsigned int byte);

	unsigned int ReadByte(bool doAck);

	public:

	void WriteBuffer(unsigned char* buffer, unsigned int numBytes1, ...);

	void ReadBuffer(unsigned int numWriteBytes, unsigned char* wrBuffer,
		unsigned int numReadBytes, unsigned char* rdBuffer);

	void WriteReg(unsigned char devAddr, unsigned char regAddr, unsigned char regData);

	void ReadReg(unsigned char devAddr, unsigned char regAddr, unsigned char* regData);


	private:
	
	unsigned int sclwPos;
	unsigned int sdawPos;
	unsigned int sclrPos;
	unsigned int sdarPos;
	unsigned int bitDur;
};

#endif
