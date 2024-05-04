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


#include <cassert>
#include <cstdarg>

#include "utils.h"
#include "i2cportlpt.h"


/// Create an I2C port.
I2CPortLPT::I2CPortLPT(void)
{
	;
}


/// Create an I2C port.
I2CPortLPT::I2CPortLPT(XSError* e,			///< error reporting channel 
			 unsigned int portNum,		///< parallel port number
			 unsigned int invMask,		///< parallel port inversion mask
			 unsigned int sclwPosArg,	///< bit position of I2C clock pin (data reg)
			 unsigned int sdawPosArg,	///< bit position of I2C write data pin (data reg)
			 unsigned int sclrPosArg,	///< bit position of I2C clock read pin (status reg)
			 unsigned int sdarPosArg,	///< bit position of I2C read data pin (status reg)
			 unsigned int bitDurArg)	///< duration of I2C bits in microseconds
{
	Setup(e,portNum,invMask,sclwPosArg,sdawPosArg,sclrPosArg,sdarPosArg,bitDurArg);
}


/// Initialize the members of the object.
bool I2CPortLPT::Setup(XSError* e,			///< error reporting channel 
			 unsigned int portNum,		///< parallel port number
			 unsigned int invMask,		///< parallel port inversion mask
			 unsigned int sclwPosArg,	///< bit position of I2C clock pin (data reg)
			 unsigned int sdawPosArg,	///< bit position of I2C write data pin (data reg)
			 unsigned int sclrPosArg,	///< bit position of I2C clock read pin (status reg)
			 unsigned int sdarPosArg,	///< bit position of I2C read data pin (status reg)
			 unsigned int bitDurArg)	///< duration of I2C bits in microseconds
{
	sclrPos  = sclrPosArg;
	sclwPos  = sclwPosArg;
	sdawPos = sdawPosArg;
	sdarPos = sdarPosArg;
	bitDur  = bitDurArg;
	if(PPort::Setup(e, portNum, invMask))
	{
		// set the clock and data pins to logic 1 which is a high-impedance state with resistor pull-ups
		SetSDA(1);
		SetSCL(1, false); // don't wait for slave to release SCL since FPGA may not be configured for I2C anyway
		// wait for things to settle
		InsertDelay(10000,MICROSECONDS);
		return true;
	}
	return false;
}


/// Force a value onto the clock line.
void I2CPortLPT::SetSCL(unsigned int bit, bool waitForSlave)
{
	Out(bit,sclwPos,sclwPos);
	// wait if the slave holds the clock line low while the
	// master tries to raise it
	while(waitForSlave && In(sclrPos,sclrPos)!=(bit&1))
		;
}


/// Read the current level on the clock line.
unsigned int I2CPortLPT::GetSCL()
{
	return In(sclrPos,sclrPos);
}


/// Toggle the clock twice to create a pulse.
void I2CPortLPT::PulseSCL(void)
{
	SetSCL(1);
	InsertDelay(bitDur,MICROSECONDS);
	SetSCL(0);
	InsertDelay(bitDur,MICROSECONDS);
}


/// Force a value onto the data line.
void I2CPortLPT::SetSDA(unsigned int bit)
{
	Out(bit,sdawPos,sdawPos);
}


/// Read the current level on the data line.
unsigned int I2CPortLPT::GetSDA()
{
	return In(sdarPos,sdarPos);
}


/// Sequence the levels on the data and clock to initiate a transfer on the I2C bus.
void I2CPortLPT::StartTransfer()
{
	// check state of SCL and SDA and set them so that
	// they are both high, but don't raise SDA while
	// SCL is also high because this is a STOP signal!!
	if(GetSCL()==1)
	{
		if(GetSDA()==0)
		{
			SetSCL(0);
			InsertDelay(bitDur,MICROSECONDS);
			SetSDA(1);
			InsertDelay(bitDur,MICROSECONDS);
			SetSCL(1);
		}
		InsertDelay(bitDur,MICROSECONDS);
	}
	else	// SCL is low so raise SDA and then SCL
	{
		SetSDA(1);	// don't bother to check SDA; just raise it
		InsertDelay(bitDur,MICROSECONDS);
		SetSCL(1);
		InsertDelay(bitDur,MICROSECONDS);
	}
	SetSDA(0);		// lower data while clock is high to start I2C transfer
	InsertDelay(bitDur,MICROSECONDS);
	SetSCL(0);		// now lower clock line
	InsertDelay(bitDur,MICROSECONDS);
}


/// Transfer a single bit over the I2C bus.
void I2CPortLPT::WriteBit(unsigned int bit)
{
	assert(GetSCL()==0);	// condition for sending bit
	SetSDA(bit);	// set data while clock is low
	InsertDelay(bitDur,MICROSECONDS);
	PulseSCL();		// pulse clock
}


/// Receive a single bit over the I2C bus.
///\return the received bit
unsigned int I2CPortLPT::ReadBit(void)
{
	assert(GetSCL()==0);	// conditions for receiving bit
	SetSCL(1);		// get bit from slave when clock is high
	//(bit from slave was probably already stable while the clock was low.)
	InsertDelay(bitDur,MICROSECONDS);
	unsigned int bit = GetSDA();	// get bit from slave
	SetSCL(0);
	InsertDelay(bitDur,MICROSECONDS);
	return bit;
}


/// Send an ACK or NACK over the I2C bus.
void I2CPortLPT::WriteAck(unsigned int ack) ///< ACK=0, NACK=1
{
	assert(GetSCL()==0);
	SetSDA(ack);	// 0=ACK; 1=NACK
	InsertDelay(bitDur,MICROSECONDS);
	PulseSCL();
}


/// Receive an ACK or NACK over the I2C bus.
///\return the ACK (0) or NACK (1) bit
unsigned int I2CPortLPT::ReadAck()
{
	assert(GetSCL()==0);	// condition for receiving ack
	SetSDA(1);	// release the data line so the receiver can ack/nack
	InsertDelay(bitDur,MICROSECONDS);
	SetSCL(1);	// raise clock
	InsertDelay(bitDur,MICROSECONDS);
	unsigned int ack = GetSDA();	// read ack/nack from slave
	SetSCL(0);	// return to condition for writing bits
	InsertDelay(bitDur,MICROSECONDS);
	return ack;
}


/// Terminate an I2C tranfer.
void I2CPortLPT::StopTransfer()
{
	assert(GetSCL()==0);
	SetSDA(0);		// make sure data line is low
	InsertDelay(bitDur,MICROSECONDS);
	SetSCL(1);		// raise clock
	InsertDelay(bitDur,MICROSECONDS);
	SetSDA(1);		// raise data while clock is high to stop I2C transfer
	InsertDelay(bitDur,MICROSECONDS);
}


/// Send an entire byte of data over the I2C bus.
void I2CPortLPT::WriteByte(unsigned int byte)
{
	for(unsigned int mask=0x80; mask!=0; mask>>=1)
		WriteBit((byte & mask) ? 1:0);
	if(ReadAck()!=0)
	{
		StopTransfer();
		string errMsg = "Transmission NACK'ed";
		GetErr().SimpleMsg(XSErrorMinor,errMsg);
	}
}


/// Receive an entire byte of data over the I2C bus.
///\return the received byte
unsigned int I2CPortLPT::ReadByte(bool doAck) ///< set to true if an ACK is needed after the reception
{
	SetSDA(1);	// release data line so slave can transmit
	InsertDelay(bitDur,MICROSECONDS);
	unsigned int byte = 0;
	for(unsigned int mask=0x80; mask!=0; mask>>=1)
		byte |= (ReadBit() ? mask:0);
	if(doAck)
	{
		// master receiver doesn't ACK the last byte read
		WriteAck(0);
		SetSDA(1);
		InsertDelay(bitDur,MICROSECONDS);
	}
	return byte;
}


/// Send a sequence of multi-byte packets over the I2C bus.
void I2CPortLPT::WriteBuffer(unsigned char* buffer,	///< buffer with multiple packets stuffed in it
						unsigned int numBytes1, 	///< number of bytes in first packet
						...)						///< number of bytes in succeeding packets, terminated with a 0 or negative number
{
	va_list marker;
	va_start(marker,numBytes1);
	
	unsigned int index = 0;
	for(unsigned int numBytes=numBytes1; numBytes>0; numBytes=va_arg(marker,unsigned int))
	{
		StartTransfer();
		for(unsigned int i=0; i<numBytes; i++, index++)
		{
			WriteByte(buffer[index]);
			if(GetErr().IsError()==true)
				return;
		}
	}
	StopTransfer();
	va_end(marker);
}


/// Receive a sequence of bytes over the I2C bus.
void I2CPortLPT::ReadBuffer(unsigned int numWriteBytes,	///< number of bytes to send 
						unsigned char* wrBuffer,		///< buffer with data to write to device
						unsigned int numReadBytes, 		///< number of bytes to receive
						unsigned char* rdBuffer)		///< pointer to buffer for received data
{
	StartTransfer();
	unsigned i;
	/// send data to device
	for(i=0; i<numWriteBytes; i++)
	{
		if(i==(numWriteBytes-1) && i>0)
			StartTransfer();
		WriteByte(wrBuffer[i]);
		if(GetErr().IsError()==true)
			return;
	}
	/// receive data from device in response to previous transmission
	for(i=0; i<numReadBytes; i++)
		rdBuffer[i] = ReadByte(i!=(numReadBytes-1));	// master-receiver doesn't ACK the last byte received
	StopTransfer();
}


/// Write a byte of data into a given register in a device on the I2C bus.
void I2CPortLPT::WriteReg(unsigned char devAddr, 	///< I2C address of device
					unsigned char regAddr, 		///< address of register in device
					unsigned char regData)		///< data byte to store in register
{
	unsigned char buf[3];
	buf[0] = devAddr & ~0x01;	// create write address
	buf[1] = regAddr;
	buf[2] = regData;
	WriteBuffer(buf,3,0);
}


/// Read a byte of data from a register in a device on the I2C bus.
void I2CPortLPT::ReadReg(unsigned char devAddr,	///< I2C address of device
					unsigned char regAddr,		///< address of register in device
					unsigned char* regData)		///< store the data from the register here
{
	unsigned char buf[3];
	buf[0] = devAddr & ~0x01;	// create write address
	buf[1] = regAddr;
	buf[2] = devAddr | 0x01;	// create read address
	ReadBuffer(3,buf,1,regData);
}
