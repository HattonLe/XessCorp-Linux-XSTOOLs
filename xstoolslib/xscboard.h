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


#ifndef XSCBOARD_H
#define XSCBOARD_H

#include "utils.h"
#include "xsboard.h"


/// Object for manipulating an XS Board through its JTAG interface.
class XSCBoard : public XSBoard
{
	public:

	XSCBoard(void);

	~XSCBoard(void);

	void SetFlags(unsigned long f);

	unsigned long GetFlags(void);

	bool Setup(XSError* err, const char* brdModel, unsigned int lptNum);

    bool Configure(const char *fileName);

    bool ConfigureInterface(const char *fileName);

    bool DownloadRAM(const char *fileName, bool bigEndianBytes, bool bigEndianBits,
		bool doStart, bool doEnd);

    bool UploadRAM(const char *fileName, const char* format, unsigned int loAddr,
		unsigned int hiAddr, bool bigEndianBytes, bool bigEndianBits,
		bool doStart, bool doEnd);

	bool ReadRAM(unsigned int addr, unsigned int* data, bool bigEndianBytes,
		bool bigEndianBits);

	bool WriteRAM(unsigned int addr, unsigned int data,
		bool bigEndianBytes, bool bigEndianBits);

    bool DownloadFlash(const char *fileName, bool bigEndianBytes, bool bigEndianBits,
		bool doStart, bool doEnd);

    bool UploadFlash(const char *fileName, const char* format, unsigned int loAddr,
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
	JTAGPort *jtagportFPGA;

	// Objects that represent devices on the board.
};

#endif
