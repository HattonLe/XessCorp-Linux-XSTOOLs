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


#ifndef PBUSPORT_H
#define PBUSPORT_H

#include "progress.h"
#include "pport.h"
#include "hexrecrd.h"

#define BIG_ENDIAN_BYTES	true
#define	LITTLE_ENDIAN_BYTES	false
#define BIG_ENDIAN_BITS		true
#define	LITTLE_ENDIAN_BITS	false


/**
Reads and writes devices on the XSB-300E peripheral bus.

This object provides methods for uploading and downloading RAM-like devices
of varying address and data widths through the XSB Board parallel port 
interface. 

The download method reads hexadecimal data records from Intel, Motorola 
or XESS HEX files and sends them to the XSB Board. Before using this 
method, the FPGA or CPLD on the XSB Board should be programmed with a 
state machine that manages the passage of the address and data 
information through the parallel port. The download method partitions 
the address and data into small bit fields that are passed to the state 
machine which re-assembles them and then loads the data into the given 
address location on the peripheral bus. The state machine returns an indicator of its 
current state through the parallel port status lines so the download 
method can detect any loss of synchronization during the download. 

The upload method works in a similar fashion. It passes an address 
through the parallel port to the state machine in the XSB Board FPGA or 
CPLD. The data from the given address Flash memory is returned 
while the upload method is sending the next memory address. 

*/
class PBusPort : public PPort
{
	public:

	PBusPort();

	PBusPort(XSError* e, unsigned int portNum, unsigned int invMask,
		unsigned int pos_reset, unsigned int pos_clk, unsigned int pos_dolsb,
		unsigned int pos_domsb, unsigned int pos_dilsb, unsigned int pos_dimsb,
		unsigned int pos_stlsb, unsigned int pos_stmsb, unsigned int dWidth,
		unsigned int aWidth);

	int Setup(XSError* e, unsigned int portNum, unsigned int invMask,
		unsigned int pos_reset, unsigned int pos_clk, unsigned int pos_dolsb,
		unsigned int pos_domsb, unsigned int pos_dilsb, unsigned int pos_dimsb,
		unsigned int pos_stlsb, unsigned int pos_stmsb, unsigned int dWidth,
		unsigned int aWidth);

    bool Download(const char *hexfileName, bool bigEndianBytes, bool bigEndianBits);

	bool Download(istream& is, bool bigEndianBytes, bool bigEndianBits);

    bool Upload(const char *hexfileName, const char* format, unsigned long loAddr,
		unsigned long hiAddr, bool bigEndianBytes, bool bigEndianBits);

	bool Upload(ostream& os, const char* format, unsigned long loAddr,
		unsigned long hiAddr, bool bigEndianBytes, bool bigEndianBits);

	bool DownloadHexRecord(HexRecord& hx, bool bigEndianBytes, bool bigEndianBits);

	bool Write(unsigned int addr, unsigned int data, bool bigEndianBytes,
		bool bigEndianBits, bool loadAddress = true);

	bool UploadHexRecord(HexRecord& hx, unsigned long loAddr,
		unsigned long hiAddr, bool bigEndianBytes, bool bigEndianBits);

	bool Read(unsigned int addr, unsigned int* data, bool bigEndianBytes,
		bool bigEndianBits, bool loadAddress = true);


	protected:

	Progress *progressGauge;	///< indicates progress of operations

	
	private:

	unsigned int posRESET; ///< bit position in parallel port of RESET pin
	unsigned int posCLK;   ///< bit position in parallel port of CLK pin
	unsigned int posDOLSB; ///< bit position in parallel port of LSB of data-out pin
	unsigned int posDOMSB; ///< bit position in parallel port of MSB of data-out pin
	unsigned int posDILSB; ///< bit position in parallel port of LSB of data-in pin
	unsigned int posDIMSB; ///< bit position in parallel port of MSB of data-in pin
	unsigned int posSTLSB; ///< bit position in parallel port of LSB of status pin
	unsigned int posSTMSB; ///< bit position in parallel port of MSB of status pin
	unsigned int dataWidth;	///< width of data
	unsigned int addrWidth;	///< width of address
};

#endif
