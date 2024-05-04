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


#ifndef JTAGRAMPORT_H
#define JTAGRAMPORT_H

#include "progress.h"
#include "lptjtag.h"
#include "xcbsdr.h"
#include "hexrecrd.h"


#define BIG_ENDIAN_BYTES	true
#define	LITTLE_ENDIAN_BYTES	false
#define BIG_ENDIAN_BITS		true
#define	LITTLE_ENDIAN_BITS	false


enum {XC9572Type=0, XC95108Type=1};


/** Uploading and downloading the XS95 Board RAM through via JTAG.

This object specializes the LPTJTAG by adding methods to support RAM 
upload and download operations using the JTAG circuitry of the XC9500 
CPLD. The levels for the RAM address, data and control pins are shifted 
into the appropriate bits of the XC9500 boundary-scan data register 
(BSDR) through the parallel port. The JTAG EXTEST instruction outputs 
these levels onto the XC9500 pins that connect to the RAM chip. Then the 
levels on the RAM pins are sampled back into the XC9500 BSDR and are 
sent back through the parallel port. This low-level operation for 
reading and writing RAM locations via the JTAG circuitry forms the basis 
of the higher-level methods for downloading and uploading Intel, 
Motorola and XESS HEX files to and from the RAM. 

*/
class JTAGRAMPort : public LPTJTAG
{
	public:

	JTAGRAMPort();

	JTAGRAMPort(int xc95Type, XSError* e, unsigned int portNum,
		unsigned int invMask, unsigned int pos_tck, unsigned int pos_tms,
		unsigned int pos_tdi, unsigned int pos_tdo);

	bool Setup(int xc95Type, XSError* e, unsigned int portNum,
		unsigned int invMask, unsigned int pos_tck, unsigned int pos_tms,
		unsigned int pos_tdi, unsigned int pos_tdo);

    bool DownloadRAM(const char *hexfileName, bool bigEndianBytes, bool bigEndianBits);

	bool DownloadRAM(istream& is, bool bigEndianBytes, bool bigEndianBits);

    bool UploadRAM(const char *hexfileName, const char* format, unsigned long loAddr,
		unsigned long hiAddr, bool bigEndianBytes, bool bigEndianBits);

	bool UploadRAM(ostream& os, const char* format, unsigned long loAddr,
		unsigned long hiAddr, bool bigEndianBytes, bool bigEndianBits);

	bool DownloadHexRecordToRAM(HexRecord& hx, bool bigEndianBytes, bool bigEndianBits);

	bool WriteRAM(unsigned int addr, unsigned int data,
		bool bigEndianBytes, bool bigEndianBits);

	bool UploadHexRecordFromRAM(HexRecord& hx, unsigned long loAddr,
		unsigned long hiAddr, bool bigEndianBytes, bool bigEndianBits);

	bool ReadRAM(unsigned int addr, unsigned int* data,
		bool bigEndianBytes, bool bigEndianBits);


	protected:

	Progress *progressGauge;	///< indicator for progress of certain operations

	
	private:

	Bitstream* bsirPtr;		///< storage for boundary-scan instruction register
	XCBSDR* bsdrPtr;		///< storage for boundary-scan data register
};

#endif
