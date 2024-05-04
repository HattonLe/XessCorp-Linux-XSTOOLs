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


#include <ctime>
#include <string>
using namespace std;

#include "utils.h"
#include "xserror.h"
#include "xsnullboard.h"


// inversion mask for parallel port connection to XSA Board
// 00000011: bits  7- 0 are attached to data pins D7-D0
// 00000xxx: bits 15-11 are attached to status pins /S7,S6,S5,S4,S3
// xxxx1001: bits 19-16 are attached to control pins /C3,C2,/C1,/C0
// const unsigned int invMask = 0x090003;

// bit position of Spartan2 configuration pins within parallel port registers
static const unsigned int posCCLK = 0;
static const unsigned int posPROG = 7;
static const unsigned int posDLO  = 2;
static const unsigned int posDHI  = 5;
static const unsigned int posDONE = 11;

// bit positions of test vector input and output
static const unsigned int posTVCLK = 0;
static const unsigned int posTVOLO = 0;
static const unsigned int posTVOHI = 7;
static const unsigned int posTVILO = 11;
static const unsigned int posTVIHI = 14;

// bit position of XC9572XL JTAG pins within parallel port registers
static const unsigned int posTCK  = 17;
static const unsigned int posTMS  = 18;
static const unsigned int posTDI  = 19;
static const unsigned int posTDO  = 15;

// bit position of prog. osc. pins within parallel port registers
static const unsigned int posOSC  = 16;

// bit positions of RAM access pins within parallel port
static const unsigned int posRRESET	= 0;
static const unsigned int posRCLK		= 1;
static const unsigned int posRDOLSB	= 2;
static const unsigned int posRDOMSB	= 5;
static const unsigned int posRDILSB	= 11;
static const unsigned int posRDIMSB	= 14;
static const unsigned int posRSTLSB	= 11;
static const unsigned int posRSTMSB	= 14;

// bit positions of Flash access pins within parallel port
static const unsigned int posFRESET	=  0;
static const unsigned int posFCLK		=  1;
static const unsigned int posFDOLSB	=  2;
static const unsigned int posFDOMSB	=  5;
static const unsigned int posFDILSB	= 11;
static const unsigned int posFDIMSB	= 13;
static const unsigned int posFSTLSB	= 11;
static const unsigned int posFSTMSB	= 13;

// bit positions for board test status
static const unsigned int posTESTSTATUS	= 14;

// USERCODE strings for various circuits programmed into the XSA CPLD
static const string oscIntfcCode			= "<0>!";
static const string flashIntfcCode			= "<1>!";
static const string flashConfigIntfcCode	= "<2>!";
static const string testIntfcCode			= "<4>!";
static const string dwnldIntfcCode			= "<4>!";


/// Create an XSA-50 or XSA-100 Board object.
XSNULLBoard::XSNULLBoard(void)
{
	XSError* err = new XSError(cerr);	// create error-reporting channel
	brdModel = NULL;	// no particular XSA board model, yet
	Setup(err,"XSNULL",0);
}


/// Destroy the XSA Board object.
XSNULLBoard::~XSNULLBoard(void)
{
	;
}


// Look at xsboard.h for a description of the interface.
void XSNULLBoard::SetFlags(unsigned long f)
{
	flags = f;
}


// Look at xsboard.h for a description of the interface.
unsigned long XSNULLBoard::GetFlags(void)
{
	return flags;
}


// Look at xsboard.h for a description of the interface.
bool XSNULLBoard::Setup(XSError* err, const char* model, unsigned int lptNum)
{
	// store some global information for the board
	brdErr = err;

	// store name of board model
	if(brdModel!=NULL)
		free(brdModel);
	brdModel = (char*)malloc(strlen(model)+1);
	if(brdModel==NULL)
		err->SimpleMsg(XSErrorFatal,"out of memory!!\n");
	strcpy(brdModel,model);

	return true;
}


// Look at xsboard.h for a description of the interface.
bool XSNULLBoard::CheckChipID(void)
{
	brdErr->SimpleMsg(XSErrorMinor,"Cannot check the chip ID on a NULL board!!\n");
	return false;	// ID can never match a null board
}


// Look at xsboard.h for a description of the interface.
bool XSNULLBoard::Configure(string& fileName)
{
	brdErr->SimpleMsg(XSErrorMinor,"Cannot configure a NULL board with a bitstream!!\n");
	return true;
}
	

// Look at xsboard.h for a description of the interface.
bool XSNULLBoard::ConfigureInterface(string& fileName)
{
	brdErr->SimpleMsg(XSErrorMinor,"Cannot configure the interface of a NULL board!!\n");
	return true;
}


// Look at xsboard.h for a description of the interface.
bool XSNULLBoard::DownloadRAM(string& fileName, bool bigEndianBytes,
			bool bigEndianBits, bool doStart, bool doEnd)
{
	brdErr->SimpleMsg(XSErrorMinor,"Cannot download to the RAM of a NULL board!!\n");
	return true;
}


// Look at xsboard.h for a description of the interface.
bool XSNULLBoard::UploadRAM(string& fileName, const char* format,	
				unsigned int loAddr, unsigned int hiAddr,
				bool bigEndianBytes, bool bigEndianBits,	
				bool doStart, bool doEnd)
{
	brdErr->SimpleMsg(XSErrorMinor,"Cannot upload from the RAM of a NULL board!!\n");
	return true;
}


// Look at xsboard.h for a description of the interface.
bool XSNULLBoard::ReadRAM(unsigned int addr, unsigned int* data,	
		bool bigEndianBytes, bool bigEndianBits)
{
	brdErr->SimpleMsg(XSErrorMinor,"Cannot read from the RAM of a NULL board!!\n");
	return true;
}


// Look at xsboard.h for a description of the interface.
bool XSNULLBoard::WriteRAM(unsigned int addr, unsigned int data,	
		bool bigEndianBytes, bool bigEndianBits)
{
	brdErr->SimpleMsg(XSErrorMinor,"Cannot write to the RAM of a NULL board!!\n");
	return true;
}


// Look at xsboard.h for a description of the interface.
bool XSNULLBoard::DownloadFlash(string& fileName, bool bigEndianBytes,
			bool bigEndianBits, bool doStart, bool doEnd)
{
	brdErr->SimpleMsg(XSErrorMinor,"Cannot download to the flash of a NULL board!!\n");
	return true;
}


// Look at xsboard.h for a description of the interface.
bool XSNULLBoard::UploadFlash(string& fileName, const char* format,	
				unsigned int loAddr, unsigned int hiAddr,
				bool bigEndianBytes, bool bigEndianBits,	
				bool doStart, bool doEnd)
{
	brdErr->SimpleMsg(XSErrorMinor,"Cannot upload from the flash of a NULL board!!\n");
	return true;
}


// Look at xsboard.h for a description of the interface.
bool XSNULLBoard::SetFreq(int div, bool extOscPresent)
{
	brdErr->SimpleMsg(XSErrorMinor,"Cannot set the frequency of a NULL board!!\n");
	return true;
}


// Look at xsboard.h for a description of the interface.
bool XSNULLBoard::SetupAudio(int *reg)
{
	brdErr->SimpleMsg(XSErrorMinor,"Cannot setup the audio codec of a NULL board!!\n");
	return true;
}


// Look at xsboard.h for a description of the interface.
bool XSNULLBoard::SetupVideoIn(string& fileName)
{
	brdErr->SimpleMsg(XSErrorMinor,"Cannot setup the video codec of a NULL board!!\n");
	return true;
}


// Look at xsboard.h for a description of the interface.
bool XSNULLBoard::Test(void)
{
	brdErr->SimpleMsg(XSErrorMinor,"Cannot run diagnostic tests on a NULL board!!\n");
	return true;
}


// Look at xsboard.h for a description of the interface.
unsigned char XSNULLBoard::ApplyTestVectors(unsigned char singleVector, unsigned char mask, unsigned char *vector,
											unsigned char *response, unsigned int numVectors)
{
	brdErr->SimpleMsg(XSErrorMinor,"Cannot apply test vectors to a NULL board!!\n");
	return 0;
}


// Look at xsboard.h for a description of the interface.
unsigned char XSNULLBoard::GetTestVector(void)
{
	brdErr->SimpleMsg(XSErrorMinor,"Cannot get test vectors from a NULL board!!\n");
	return 0;
}


// Look at xsboard.h for a description of the interface.
bool XSNULLBoard::DownloadRAMFromIntArray(unsigned *intArray, unsigned address, unsigned numInts)			
{
	brdErr->SimpleMsg(XSErrorMinor,"Cannot download to a NULL board!!\n");
	return false; // This method is not implemented.
}


// Look at xsboard.h for a description of the interface.
bool XSNULLBoard::UploadRAMToIntArray(unsigned *intArray, unsigned address, unsigned numInts)			
{
	brdErr->SimpleMsg(XSErrorMinor,"Cannot upload from a NULL board!!\n");
	return false; // This method is not implemented.
}
