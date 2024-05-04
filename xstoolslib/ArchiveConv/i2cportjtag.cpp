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
#include "i2cportjtag.h"


/// Create an I2C port.
I2CPortJTAG::I2CPortJTAG(void)
{
	;
}


/// Create an I2C port.
I2CPortJTAG::I2CPortJTAG(XSError* e,	///< error reporting channel 
	unsigned int portNum,				///< USB port number
	unsigned int endptNum,				///< USB endpoint number
	string brdModel						///< type of XESS Board attached thru USB
	): RegioJTAG(e,portNum,endptNum,brdModel)
{
	Setup(e,portNum,endptNum);
}


#define	PRESCALE_REG_LO		0x00
#define	PRESCALE_REG_HI		0x01
#define	CONTROL_REG			0x02
#define	TX_REG				0x03
#define	RX_REG				0x03
#define	COMMAND_REG			0x04
#define	STATUS_REG			0x04

#define	ENABLE_CORE_BIT		0x80
#define	ENABLE_INTR_BIT		0x40

#define	START_BIT			0x80
#define	STOP_BIT			0x40
#define	READ_BIT			0x20
#define	WRITE_BIT			0x10
#define	ACK_BIT				0x08
#define	IACK_BIT			0x01

#define RXACK_BIT			0x80
#define BUSY_BIT			0x40
#define ARBLOST_BIT			0x20
#define TIP_BIT				0x02
#define IFLAG				0x01

/// Initialize the members of the object.
bool I2CPortJTAG::Setup(XSError* e,		///< error reporting channel 
	unsigned int portNum,				///< USB port number
	unsigned int endptNum)				///< USB endpoint number
{
	Write(PRESCALE_REG_LO,199);	// set prescaler to 100 MHz / (5 x 100 KHz) - 1 = 199
	Write(PRESCALE_REG_HI,0);
	Write(CONTROL_REG,ENABLE_CORE_BIT);	// enable I2C core; disable interrupts
	return true;
}


/// Send a sequence of multi-byte packets over the I2C bus.
void I2CPortJTAG::WriteBuffer(unsigned char* buffer,	///< buffer with multiple packets stuffed in it
	unsigned int numBytes1, 	///< number of bytes in first packet
	...)						///< number of bytes in succeeding packets, terminated with a 0 or negative number
{
	va_list marker;
	va_start(marker,numBytes1);

	bool startup, teardown;
	startup = true;
	teardown = true;

	unsigned char cmd = 0;  // initialize to NOP command
	unsigned int index = 0;
	for(unsigned int numBytes=numBytes1; numBytes>0; )
	{

		cout << "Write buffer: ";

		cmd |= START_BIT;	// output a start bit at the beginning of each packet
//		StartTransfer();

		// transmit every byte except the last in the current packet
		for(unsigned int i=0; i<numBytes-1; i++, index++)
		{
			cout << hex << (unsigned int)buffer[index] << " ";

			Write(TX_REG,buffer[index],startup,teardown);	// load the current byte into the transmit register
//			startup = false;
//			teardown = false;
			cmd |= WRITE_BIT;				// set the write-enable bit in the command
			DEBUG_STMT( "Transmitting " << hex << (int)buffer[index] << " using command " << hex << (int)cmd << "\n")
			Write(COMMAND_REG,cmd,startup,teardown);			// initiate the transmission
//			InsertDelay(10,MILLISECONDS);
			cmd = 0; // reset command to NOP for next iteration
			// wait while transfer in progress
			DEBUG_STMT("Waiting for transfer to complete ... ");
			while(Read(STATUS_REG,startup,teardown) & TIP_BIT)
				;
			DEBUG_STMT("completed!\n");
			// wait for interrupt flag to be set
			DEBUG_STMT("Waiting for interrupt to occur ... ");
			while(Read(STATUS_REG,startup,teardown) & IFLAG == 0)
				;
			DEBUG_STMT("found it!\n");
			Write(COMMAND_REG,IACK_BIT,startup,teardown); // clear interrupt flag
//			WriteByte(buffer[index]);
//			if(GetErr().IsError()==true)
//				return;
		}

		// transmit the last byte in the current packet
		cout << hex << (unsigned int)buffer[index] << "\n";

		Write(TX_REG,buffer[index],startup,teardown);		// load the last byte into the transmit register
//		startup = false;
//		teardown = false;
		numBytes = va_arg(marker,unsigned int);	// number of bytes in the next packet
		if(numBytes == 0)
			cmd |= STOP_BIT;	// send a stop bit if this is the last byte of the last packet
		cmd |= WRITE_BIT;
		DEBUG_STMT( "Transmitting " << hex << (int)buffer[index] << " using command " << hex << (int)cmd << "\n")
		Write(COMMAND_REG,cmd,startup,teardown);				// initiate the transmission
//		InsertDelay(10,MILLISECONDS);
		cmd = 0; // reset command to NOP for next iteration
		DEBUG_STMT("Waiting for transfer to complete ... ");
		while(Read(STATUS_REG,startup,teardown) & TIP_BIT)
			;
		DEBUG_STMT("completed!\n");
		// wait for interrupt flag to be set
		DEBUG_STMT("Waiting for interrupt to occur ... ");
		while(Read(STATUS_REG,startup,teardown) & IFLAG == 0)
			;
		DEBUG_STMT("found it!\n");
		Write(COMMAND_REG,IACK_BIT,startup,teardown); // clear interrupt flag
//		if(GetErr().IsError()==true)
//			return;
	}
//	StopTransfer();
	va_end(marker);
}


/// Receive a sequence of bytes over the I2C bus.
void I2CPortJTAG::ReadBuffer(unsigned int numWriteBytes,	///< number of bytes to send 
	unsigned char* wrBuffer,								///< buffer with data to write to device
	unsigned int numReadBytes, 								///< number of bytes to receive
	unsigned char* rdBuffer)								///< pointer to buffer for received data
{
//	StartTransfer();
	unsigned i;
	/// send data to device
	for(i=0; i<numWriteBytes; i++)
	{
//		if(i==(numWriteBytes-1) && i>0)
//			StartTransfer();
//		WriteByte(wrBuffer[i]);
		if(GetErr().IsError()==true)
			return;
	}
	/// receive data from device in response to previous transmission
//	for(i=0; i<numReadBytes; i++)
//		rdBuffer[i] = ReadByte(i!=(numReadBytes-1));	// master-receiver doesn't ACK the last byte received
//	StopTransfer();
}


/// Write a byte of data into a given register in a device on the I2C bus.
void I2CPortJTAG::WriteReg(unsigned char devAddr, 	///< I2C address of device
	unsigned char regAddr, 							///< address of register in device
	unsigned char regData)							///< data byte to store in register
{
	unsigned char buf[3];
	buf[0] = devAddr & ~0x01;	// create write address
	buf[1] = regAddr;
	buf[2] = regData;
	WriteBuffer(buf,3,0);
}


/// Read a byte of data from a register in a device on the I2C bus.
void I2CPortJTAG::ReadReg(unsigned char devAddr,	///< I2C address of device
	unsigned char regAddr,							///< address of register in device
	unsigned char* regData)							///< store the data from the register here
{
	unsigned char buf[3];
	buf[0] = devAddr & ~0x01;	// create write address
	buf[1] = regAddr;
	buf[2] = devAddr | 0x01;	// create read address
	ReadBuffer(3,buf,1,regData);
}
