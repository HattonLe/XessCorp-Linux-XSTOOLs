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


#ifndef FLASHPORT_H
#define FLASHPORT_H

#include "progress.h"
#include "hexrecrd.h"
#include "pport.h"


#define BIG_ENDIAN_BYTES	true
#define	LITTLE_ENDIAN_BYTES	false
#define BIG_ENDIAN_BITS		true
#define	LITTLE_ENDIAN_BITS	false
#define ENDIAN_DEFAULTS	BIG_ENDIAN_BYTES,LITTLE_ENDIAN_BITS


/**
Uploading and downloading of hexadecimal records to the Flash memory on an XSA, XSB or XSV Board.

This object provides general methods for uploading and downloading Flash 
memory of varying address and data widths through the XS Board parallel 
port interface. Virtual methods are provided for managing the low-level 
operations of erasing the Flash and writing individual bytes of Flash. 

The download method reads hexadecimal data records from Intel, Motorola 
or XESS HEX files and sends them to the XS Board. Before using this 
method, the FPGA or CPLD on the XS Board should be programmed with a 
state machine that manages the passage of the address and data 
information through the parallel port. The download method partitions 
the address and data into small bit fields that are passed to the state 
machine which re-assembles them and then loads the data into the given 
address location in Flash memory. The state machine returns an indicator 
of its current state through the parallel port status lines so the 
download method can detect any loss of synchronization during the Flash 
download. 

The upload method works in a similar fashion. It passes an address 
through the parallel port to the state machine in the XS Board FPGA or 
CPLD. The CPLD returns the data from the given Flash memory address 
while the upload method is sending the next memory address. 

*/
class FlashPort : public PPort
{
	public:

	FlashPort(void);

	FlashPort(XSError* e,
		unsigned int portNum,
		unsigned int invMask,
		unsigned int pos_reset,
		unsigned int pos_clk,
		unsigned int pos_dolsb,
		unsigned int pos_domsb,
		unsigned int pos_dilsb,
		unsigned int pos_dimsb,
		unsigned int pos_stlsb,
		unsigned int pos_stmsb);

	int Setup(XSError* e,
		unsigned int portNum,
		unsigned int invMask,
		unsigned int pos_reset,
		unsigned int pos_clk,
		unsigned int pos_dolsb,
		unsigned int pos_domsb,
		unsigned int pos_dilsb,
		unsigned int pos_dimsb,
		unsigned int pos_stlsb,
		unsigned int pos_stmsb);

	bool DownloadFlash(
		string& hexfileName,
		bool bigEndianBytes,
		bool bigEndianBits,
		bool doErase);

	bool DownloadFlash(
		istream& is,
		bool bigEndianBytes,
		bool bigEndianBits,
		bool doErase);

	bool DownloadHexRecordToFlash(
		HexRecord& hx,
		bool bigEndianBytes,
		bool bigEndianBits);

	bool WriteFlashByte(
		unsigned int address,
		unsigned int data,
		bool bigEndianBytes,
		bool bigEndianBits);

	bool UploadFlash(
		string& hexfileName,
		const char* format,
		unsigned long loAddr,
		unsigned long hiAddr,
		bool bigEndianBytes,
		bool bigEndianBits);

	bool UploadFlash(
		ostream& os,
		const char* format,
		unsigned long loAddr,
		unsigned long hiAddr,
		bool bigEndianBytes,
		bool bigEndianBits);

	bool UploadHexRecordFromFlash(
		HexRecord& hx,
		unsigned long loAddr,
		unsigned long hiAddr,
		bool bigEndianBytes,
		bool bigEndianBits);

	bool ReadFlashByte(
		unsigned int address,
		unsigned int* b,
		bool bigEndianBytes,
		bool bigEndianBits);

	int Test(void);

	virtual bool ProgramFlash(
		unsigned int address,
		unsigned int data,
		bool bigEndianBytes,
		bool bigEndianBits) = 0;

	virtual bool EraseFlash(void) = 0;

	virtual bool EraseFlashBlock(unsigned int blockIndex) = 0;

	virtual bool ResetFlash(void) = 0;

	
	private:

	Progress *progressGauge;	// indicates progress of operations

	unsigned int posRESET; // bit position in parallel port of RAM RESET pin
	unsigned int posCLK;   // bit position in parallel port of RAM CLK pin
	unsigned int posDOLSB; // bit position in parallel port of LSB of RAM data-out pin
	unsigned int posDOMSB; // bit position in parallel port of MSB of RAM data-out pin
	unsigned int posDILSB; // bit position in parallel port of LSB of RAM data-in pin
	unsigned int posDIMSB; // bit position in parallel port of MSB of RAM data-in pin
	unsigned int posSTLSB; // bit position in parallel port of LSB of RAM status pin
	unsigned int posSTMSB; // bit position in parallel port of MSB of RAM status pin
};

#endif
