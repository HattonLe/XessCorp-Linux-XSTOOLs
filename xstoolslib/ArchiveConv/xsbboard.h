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


#ifndef XSBBOARD_H
#define XSBBOARD_H

#include "utils.h"
#include "xsboard.h"
#include "xc2sprt.h"
#include "xc95kprt.h"
#include "osccyprt.h"
#include "pbusport.h"
#include "at49fprt.h"
#include "akcdcprt.h"
#include "saa711x.h"
#include "testport.h"


/// Object for manipulating an XSB-300E Board.
class XSBBoard : public XSBoard
{
	public:

	XSBBoard(void);

	~XSBBoard(void);

	void SetFlags(unsigned long f);

	unsigned long GetFlags(void);

	bool CheckChipID(void);

	bool Setup(XSError* err, const char* brdModel, unsigned int lptNum);

	bool Configure(string& fileName);

	bool ConfigureInterface(string& fileName);

	bool DownloadRAM(string& fileName, bool bigEndianBytes, bool bigEndianBits,
		bool doStart, bool doEnd);

	bool UploadRAM(string& fileName, const char* format, unsigned int loAddr,
		unsigned int hiAddr, bool bigEndianBytes, bool bigEndianBits,
		bool doStart, bool doEnd);

	bool ReadRAM(unsigned int addr, unsigned int* data, bool bigEndianBytes,
		bool bigEndianBits);

	bool WriteRAM(unsigned int addr, unsigned int data,
		bool bigEndianBytes, bool bigEndianBits);

	bool DownloadFlash(string& fileName, bool bigEndianBytes, bool bigEndianBits,
		bool doStart, bool doEnd);

	bool UploadFlash(string& fileName, const char* format, unsigned int loAddr,
		unsigned int hiAddr, bool bigEndianBytes, bool bigEndianBits,
		bool doStart,bool doEnd);

	bool SetFreq(int div, bool extOscPresent);

	bool SetupAudio(int *reg);

	bool SetupVideoIn(string& fileName);

	bool Test(void);

	unsigned char ApplyTestVectors(unsigned char singleVector, unsigned char mask,
		unsigned char *vector=NULL, unsigned char *response=NULL, unsigned int numVectors=1);

	unsigned char GetTestVector(void);

	bool DownloadRAMFromIntArray(unsigned *intArray, unsigned address, unsigned numInts);

	bool UploadRAMToIntArray(unsigned *intArray, unsigned address, unsigned numInts);			

	
	private:
	
	// Objects that represent devices on the board.
	TestPort testPort;		///< port for applying test vectors
	XC2SPort fpga;			///< main programmable logic device on the board
	XC95KPort cpld;			///< CPLD that manages the parallel port interface
	OscCyPort osc;			///< Cypress programmable oscillator
	PBusPort ram;			///< SDRAM on peripheral bus
	AT49FPort flash;		///< Atmel Flash
	AKCodecPort codec;		///< AKM audio codec
	SAA711X videoin;		///< Philips video decoder
};

#endif
