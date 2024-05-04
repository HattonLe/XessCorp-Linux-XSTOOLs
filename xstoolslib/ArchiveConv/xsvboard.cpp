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
#include "xsvboard.h"


// inversion mask for parallel port connection to XSV Board
// 00000011: bits  7- 0 are attached to data pins D7-D0
// 00000xxx: bits 15-11 are attached to status pins /S7,S6,S5,S4,S3
// xxxx1001: bits 19-16 are attached to control pins /C3,C2,/C1,/C0
// const unsigned int invMask = 0x090003;

// bit position of XCV configuration pins within parallel port registers
static const unsigned int posCCLK = 17;
static const unsigned int posPROG = 16;
static const unsigned int posDIN  = 19;
static const unsigned int posDLO  = 0;
static const unsigned int posDHI  = 7;
static const unsigned int posDONE = 23;	// not used

// bit positions of test vector input and output
static const unsigned int posTVCLK = 0;
static const unsigned int posTVOLO = 0;
static const unsigned int posTVOHI = 7;
static const unsigned int posTVILO = 11;
static const unsigned int posTVIHI = 14;

// bit position of XC95108 JTAG pins within parallel port registers
static const unsigned int posTCK  = 17;
static const unsigned int posTMS  = 18;
static const unsigned int posTDI  = 19;
static const unsigned int posTDO  = 15;

// bit position of prog. osc. pins within parallel port registers
static const unsigned int posOSC  = 0;

// bit positions of RAM access pins within parallel port
static const unsigned int posRRESET	= 1;
static const unsigned int posRCLK		= 0;
static const unsigned int posRDOLSB	= 2;
static const unsigned int posRDOMSB	= 5;
static const unsigned int posRDILSB	= 11;
static const unsigned int posRDIMSB	= 14;
static const unsigned int posRSTLSB	= 11;
static const unsigned int posRSTMSB	= 14;

// bit positions of Flash access pins within parallel port
static const unsigned int posFRESET	=  6;
static const unsigned int posFCLK		=  0;
static const unsigned int posFDOLSB	=  2;
static const unsigned int posFDOMSB	=  5;
static const unsigned int posFDILSB	= 11;
static const unsigned int posFDIMSB	= 13;
static const unsigned int posFSTLSB	= 11;
static const unsigned int posFSTMSB	= 13;

// bit positions for board test status
static const unsigned int posTESTSTATUS	= 11;

// bit position of SAA711X pins within parallel port registers
static const unsigned int posSAASCLW = 1;
static const unsigned int posSAASCLR = 11;
static const unsigned int posSAASDAW = 6;
static const unsigned int posSAASDAR = 12;

// USERCODE strings for various circuits programmed into the XSV CPLD
static const string oscIntfcCode			= "<0>!";
static const string flashIntfcCode			= "<1>!";
static const string flashConfigIntfcCode	= "<2>!";
static const string testIntfcCode			= "<3>!";
static const string dwnldIntfcCode			= "<4>!";


/// Create an XSV Board object.
XSVBoard::XSVBoard(void)
{
	XSError* err = new XSError(cerr);	// create error-reporting channel
	brdModel = NULL;
	Setup(err,"XSV-800",1);	// use a default board model and parallel port number at this point
}


/// Destroy the XSV Board object.
XSVBoard::~XSVBoard(void)
{
	;
}


// Look at xsboard.h for a description of the interface.
void XSVBoard::SetFlags(unsigned long f)
{
	flags = f;
}


// Look at xsboard.h for a description of the interface.
unsigned long XSVBoard::GetFlags(void)
{
	return flags;
}


// Look at xsboard.h for a description of the interface.
bool XSVBoard::Setup(XSError* err, const char* model, unsigned int lptNum)
{
	// store some global information for the board
	brdErr = err;
	portNum = lptNum;

	// store name of board model
	if(brdModel!=NULL)
		free(brdModel);
	brdModel = (char*)malloc(strlen(model)+1);
	if(brdModel==NULL)
		err->SimpleMsg(XSErrorFatal,"out of memory!!\n");
	strcpy(brdModel,model);

	// get information about all the models of XS boards
	int numBoards = GetXSBoardInfo(&brdInfo);

	// find parallel port inversion mask for this board model
	for(brdIndex=0; brdIndex<numBoards; brdIndex++)
	{
		if(strcmp(brdModel,brdInfo[brdIndex].brdModel) == 0)
			break;
	}
	if(brdIndex>=numBoards)
		err->SimpleMsg(XSErrorFatal,"Unknown type of XSV Board!\n");
	int invMask = brdInfo[brdIndex].port[PORTTYPE_LPT].invMask;

	// initialize the objects for the components on the board
	bool status;
	status = testPort.Setup(err,lptNum,invMask,posTVCLK,posTVOLO,posTVOHI,posTVILO,posTVIHI);
	status = status && fpga.Setup(err,lptNum,invMask,posCCLK,posPROG,posDIN,posDLO,posDHI,posDONE);
	status = status && cpld.Setup(err,lptNum,invMask,posTCK,posTMS,posTDI,posTDO);
	status = status && osc.Setup(err,lptNum,invMask,posOSC);
	status = status && ram.Setup(err,lptNum,invMask,posRRESET,posRCLK,posRDOLSB,posRDOMSB,posRDILSB,posRDIMSB,posRSTLSB,posRSTMSB,16,20);
	status = status && flash.Setup(err,lptNum,invMask,posFRESET,posFCLK,posFDOLSB,posFDOMSB,posFDILSB,posFDIMSB,posFSTLSB,posFSTMSB);
	status = status && videoin.Setup(err,lptNum,invMask,posSAASCLW,posSAASDAW,posSAASCLR,posSAASDAR);
	return status;	// return true if no error occurred for any object setup
}


// Look at xsboard.h for a description of the interface.
bool XSVBoard::CheckChipID(void)
{
	// get the chip ID from the interface cpld and compare it to the chip ID for
	// an XC95108 CPLD (ignoring the first 4 bits which increment for each chip revision)
	char* XC95108ID = "1001010100000110000010010011";
	string chipID = cpld.GetChipID();
	if(strncmp(XC95108ID,chipID.c_str()+4,strlen(XC95108ID)))
	{
		string instructions;
		instructions = "The interface CPLD of your XSV Board is not responding!!\n\n";
		instructions += "\nChip ID = ";
		instructions += chipID;		// display the ID that was received
		instructions += "\n\n";
		instructions += "1) Is power getting to your XSV Board?\n";
		instructions += "2) Is the downloading cable attached?\n";
		instructions += "3) Is there a shunt on J23?\n";
		instructions += "4) Are there shunts across pins 2 & 3 of J29, J30 and J31?\n";
		instructions += "\nContinue anyway?\n";
		if(PromptUser(instructions,PROMPT_OKCANCEL) == RESPONSE_CANCEL)
			return false;
	}
	return true;	// ID matched
}


// Look at xsboard.h for a description of the interface.
bool XSVBoard::Configure(string& fileName)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	// check the ID of the interface CPLD to see if it matches the type on this particular board.
	if(CheckChipID() == false)
		return false;

	// Check the suffix of the bitstream file.  .BIT files go to the FPGA; .SVF files go to the interface CPLD.
	string suffix = GetSuffix(fileName);
	if(suffix=="BIT")
	{ // configure the FPGA with a bitstream
		
		// look in the bitstream file to find the type of FPGA it is intended for and make sure that matches the type of FPGA on the board
		string type = fpga.GetChipType(fileName);
		ConvertToUpperCase(type);
		if(type == "")
		{
			errMsg.SimpleMsg(XSErrorMajor,"The .BIT file does not identify the target FPGA!!\n");
			return false;
		}
		if(type != brdInfo[brdIndex].chipType)
		{
			string msg = (string)".BIT file is for the " + type + (string)" but this is not the FPGA on the " + brdModel + (string)"!!\n";
			errMsg.SimpleMsg(XSErrorMajor,msg);
			return false;
		}

		// get the USERCODE from the interface CPLD to see if it supports downloading a bitstream to the FPGA
		string usercode = cpld.GetUSERCODE();
		if((usercode==oscIntfcCode) || (usercode==flashIntfcCode) || (usercode==flashConfigIntfcCode))
		{ // the interface CPLD is not configured with the standard downloading circuit
			if(ConfigureInterface(string(brdInfo[brdIndex].port[PORTTYPE_LPT].dwnldIntfcBitstreamFile)) == false)
			{
				errMsg.SimpleMsg(XSErrorMajor,"Error configuring CPLD with downloading interface circuit!\n");
				return false;
			}
			fpga.EnableFastDownload(true);
		}
		else if((usercode==dwnldIntfcCode) || (usercode==testIntfcCode))
		{ // the CPLD contains the fast downloading interface
			fpga.EnableFastDownload(true);
		}
		else
		{ // can't tell what the CPLD contains, so assume the user loaded it with a legacy, low-speed interface
			fpga.EnableFastDownload(false);
		}
		
		// initialize and then configure the FPGA with the contents of the bitstream file
		fpga.InitConfigureFPGA();
		return fpga.ConfigureFPGA(fileName);
	}
	else if(suffix=="SVF" || suffix=="svf")
	{ // configure the CPLD with an SVF bitstream
		return ConfigureInterface(fileName);
	}
	
	// neither a .BIT or .SVF file, so report the error
	errMsg.SimpleMsg(XSErrorMajor,"Only .BIT or .SVF files can be downloaded into the FPGA or CPLD on the XSV Board!!\n");
	return false;
}


// Look at xsboard.h for a description of the interface.
bool XSVBoard::ConfigureInterface(string& fileName)
{
	// check the type of the file that is going to be downloaded to the interface CPLD
	string suffix = GetSuffix(fileName);
	if(suffix!="SVF")
		return false;

	// check the ID of the interface CPLD to see if it matches the type on this particular board.
	if(CheckChipID() == false)
		return false;

	// initialize and then configure the CPLD with the contents of the bitstream file
	cpld.InitConfigureCPLD();
	return cpld.ConfigureCPLD(fileName);
}


// Look at xsboard.h for a description of the interface.
bool XSVBoard::DownloadRAM(string& fileName, bool bigEndianBytes,
			bool bigEndianBits, bool doStart, bool doEnd)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	// check the file suffix to see if it is appropriate for downloading to RAM
	string suffix = GetSuffix(fileName);
	if(suffix!="HEX" && suffix!="EXO" && suffix!="XES" && suffix!="MCS")
	{
		errMsg.SimpleMsg(XSErrorMajor,"Only .HEX, .MCS, .EXO or .XES files can be downloaded into the RAM on the XSV Board!!\n");
		return false;
	}
	
	// check the ID of the interface CPLD to see if it matches the type on this particular board.
	if(CheckChipID() == false)
		return false;

	// get the name of the file that contains a bitstream that will configure the FPGA to provide an interface
	// between the parallel port and the RAM.
	if(strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile)==0)
	{
		string msg = brdModel + (string)" does not support RAM downloads!!\n";
		errMsg.SimpleMsg(XSErrorMajor,msg);
		return false;
	}

	// only download the RAM interface if this is the first download of data to the RAM.
	// otherwise the interface should already be in place.
	if(doStart)
	{
		if(Configure(string(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile)) == false)
		{
			errMsg.SimpleMsg(XSErrorMajor,"Error downloading RAM interface circuit!!\n");
			return false;
		}
	}

	// download data to the RAM
	if(ram.DownloadRAM(fileName,bigEndianBytes,bigEndianBits) == false)
	{
		errMsg.SimpleMsg(XSErrorMajor,"Error downloading into RAM!!\n");
		return false;
	}

	return true;
}


// Look at xsboard.h for a description of the interface.
bool XSVBoard::UploadRAM(string& fileName, const char* format,	
				unsigned int loAddr, unsigned int hiAddr,
				bool bigEndianBytes, bool bigEndianBits,	
				bool doStart, bool doEnd)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	// get the name of the file that contains a bitstream that will configure the FPGA to provide an interface
	// between the parallel port and the RAM.
	if(strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile)==0)
	{
		// it is an error if there is no RAM download interface configuration for this board
		string msg = brdModel + (string)" does not support RAM uploads!!\n";
		errMsg.SimpleMsg(XSErrorMajor,msg);
		return false;
	}

	// check the ID of the interface CPLD to see if it matches the type on this particular board.
	if(CheckChipID() == false)
		return false;

	// only download the RAM interface if this is the first download of data to the RAM.
	// otherwise the interface should already be in place.
	if(doStart)
	{
		if(Configure(string(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile)) == false)
		{
			errMsg.SimpleMsg(XSErrorMajor,"Error downloading RAM interface circuit!!\n");
			return false;
		}
	}

	// upload data from the RAM
	if(ram.UploadRAM(fileName,format,loAddr,hiAddr,bigEndianBytes,bigEndianBits) == false)
	{
		errMsg.SimpleMsg(XSErrorMajor,"Error uploading from RAM!!\n");
		return false;
	}
	return true;
}


// Look at xsboard.h for a description of the interface.
bool XSVBoard::ReadRAM(unsigned int addr, unsigned int* data,	
		bool bigEndianBytes, bool bigEndianBits)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	if(ram.ReadRAM(addr,data,bigEndianBytes,bigEndianBits) == false)
	{ // couldn't read RAM so maybe RAM interface is not loaded???
		
		// get the name of the file that contains a bitstream that will configure the FPGA to provide an interface
		// between the parallel port and the RAM
		if(strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile)==0)
		{
			string msg = brdModel + (string)" does not support RAM uploads!!\n";
			errMsg.SimpleMsg(XSErrorMajor,msg);
			return false;
		}
		
		// check the ID of the interface CPLD to see if it matches the type on this particular board.
		if(CheckChipID() == false)
			return false;
		
		// configure the FPGA with the RAM interface
		if(Configure(string(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile)) == false)
		{
			errMsg.SimpleMsg(XSErrorMajor,"Error downloading RAM interface circuit!!\n");
			return false;
		}

		// now try reading the RAM again
		return ram.ReadRAM(addr,data,bigEndianBytes,bigEndianBits);	// now try reading the RAM again
	}
	
	return true;	// RAM read succeeded
}


// Look at xsboard.h for a description of the interface.
bool XSVBoard::WriteRAM(unsigned int addr, unsigned int data,	
		bool bigEndianBytes, bool bigEndianBits)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	if(ram.WriteRAM(addr,data,bigEndianBytes,bigEndianBits) == false)
	{ // couldn't write to RAM so maybe RAM interface is not loaded???
		
		// get the name of the file that contains a bitstream that will configure the FPGA to provide an interface
		// between the parallel port and the RAM
		if(strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile)==0)
		{
			string msg = brdModel + (string)" does not support RAM uploads!!\n";
			errMsg.SimpleMsg(XSErrorMajor,msg);
			return false;
		}
		
		// check the ID of the interface CPLD to see if it matches the type on this particular board.
		if(CheckChipID() == false)
			return false;
		
		// configure the FPGA with the RAM interface
		if(Configure(string(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile)) == false)
		{
			errMsg.SimpleMsg(XSErrorMajor,"Error downloading RAM interface circuit!!\n");
			return false;
		}

		// now try writing the RAM again
		return ram.WriteRAM(addr,data,bigEndianBytes,bigEndianBits);	// now try writing to the RAM again
	}
	
	return true;	// RAM write succeeded
}


// Look at xsboard.h for a description of the interface.
bool XSVBoard::DownloadFlash(string& fileName, bool bigEndianBytes,
			bool bigEndianBits, bool doStart, bool doEnd)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	// check the file suffix to see if it is appropriate for downloading to Flash
	string suffix = GetSuffix(fileName);
	if(suffix!="HEX" && suffix!="EXO" && suffix!="XES" && suffix!="MCS")
	{
		errMsg.SimpleMsg(XSErrorMajor,"Only .HEX, .MCS, .EXO or .XES files can be downloaded into the Flash on the XSV Board!!\n");
		return false;
	}

	// get the name of the file that contains a bitstream that will configure the CPLD to provide an interface
	// between the parallel port and the Flash.
	if(strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].flashBitstreamFile)==0)
	{
		string msg = brdModel + (string)" does not support Flash programming!!\n";
		errMsg.SimpleMsg(XSErrorMajor,msg);
		return false;
	}

	// check the ID of the interface CPLD to see if it matches the type on this particular board.
	if(CheckChipID() == false)
		return false;
	
	// get the USERCODE from the interface CPLD to see if it supports downloading to the Flash
	bool loadFlashIntfc;
	string usercode = cpld.GetUSERCODE();
	if((usercode==oscIntfcCode) || (usercode==dwnldIntfcCode) || 
		(usercode==flashConfigIntfcCode) || (usercode==testIntfcCode))
	{ // the interface CPLD is not configured with the standard Flash downloading interface ...
		loadFlashIntfc = true; // so force it to be loaded
	}
	else if(usercode==flashIntfcCode)
	{ // the CPLD already contains the Flash programming interface bitstream
		loadFlashIntfc = false;
	}
	else
	{ // can't tell what the CPLD contains, so load the Flash programming interface
		loadFlashIntfc = true;
	}
	
	// If this is the first data file being downloaded to the Flash, then configure the CPLD with the Flash interface
	if(doStart || loadFlashIntfc)
	{
		if(ConfigureInterface(string(brdInfo[brdIndex].port[PORTTYPE_LPT].flashBitstreamFile)) == false)
		{
			errMsg.SimpleMsg(XSErrorMajor,"Error configuring CPLD with Flash programming circuit!\n");
			return false;
		}
	}
	
	// download data into the Flash
	if(flash.DownloadFlash(fileName,bigEndianBytes,bigEndianBits,doStart) == false)
	{
		errMsg.SimpleMsg(XSErrorMajor,"Error programming the Flash on the XSV Board!\n");
		return false;
	}
	
	// if this is the last file to be downloaded to Flash, then reprogram the CPLD with a circuit
	// that will make the CPLD load the FPGA with the contents of the Flash upon power-up.
	if(doEnd)
	{
		if(ConfigureInterface(string(brdInfo[brdIndex].port[PORTTYPE_LPT].flashConfigBitstreamFile)) == false)
		{
			errMsg.SimpleMsg(XSErrorMajor,"Error configuring CPLD with Flash configuration circuit!\n");
			return false;
		}
	}
	
	return true;
}


// Look at xsboard.h for a description of the interface.
bool XSVBoard::UploadFlash(string& fileName, const char* format,	
				unsigned int loAddr, unsigned int hiAddr,
				bool bigEndianBytes, bool bigEndianBits,	
				bool doStart, bool doEnd)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	// get the name of the file that contains a bitstream that will configure the CPLD to provide an interface
	// between the parallel port and the Flash
	if(strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].flashBitstreamFile)==0)
	{
		string msg = brdModel + (string)" does not support Flash uploads!!\n";
		errMsg.SimpleMsg(XSErrorMajor,msg);
		return false;
	}

	// check the ID of the interface CPLD to see if it matches the type on this particular board.
	if(CheckChipID() == false)
		return false;
	
	// get the USERCODE from the interface CPLD to see if it supports uploading from the Flash
	bool loadFlashIntfc;
	string usercode = cpld.GetUSERCODE();
	if((usercode==oscIntfcCode) || (usercode==dwnldIntfcCode) || 
		(usercode==flashConfigIntfcCode) || (usercode==testIntfcCode))
	{ // the interface CPLD is not configured with the standard Flash interface ...
		loadFlashIntfc = true; // so force it to be loaded
	}
	else if(usercode==flashIntfcCode)
	{ // the CPLD already contains the Flash programming interface bitstream
		loadFlashIntfc = false;
	}
	else
	{ // can't tell what the CPLD contains, so load the Flash programming interface
		loadFlashIntfc = true;
	}

	if(loadFlashIntfc && doStart)
	{
		// load the CPLD with the Flash interface file
		if(ConfigureInterface(string(brdInfo[brdIndex].port[PORTTYPE_LPT].flashBitstreamFile)) == false)
		{
			errMsg.SimpleMsg(XSErrorMajor,"Error configuring CPLD with Flash programming circuit!\n");
			return false;
		}
	}

	// upload data from the Flash
	if(flash.UploadFlash(fileName,format,loAddr,hiAddr,bigEndianBytes,bigEndianBits) == false)
	{
		errMsg.SimpleMsg(XSErrorMajor,"Error uploading from Flash!!\n");
		return false;
	}

	// you can only upload a single file from Flash, so reprogram the CPLD with a circuit
	// that will make the CPLD load the FPGA with the contents of the Flash upon power-up.
	// (We are assuming this was the circuit programmed into the CPLD before uploading from the Flash.)
	if(ConfigureInterface(string(brdInfo[brdIndex].port[PORTTYPE_LPT].flashConfigBitstreamFile)) == false)
	{
		errMsg.SimpleMsg(XSErrorMajor,"Error configuring CPLD with Flash configuration circuit!\n");
		return false;
	}

	return true;
}


// Look at xsboard.h for a description of the interface.
bool XSVBoard::SetFreq(int div, bool extOscPresent)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	// get the name of the file that contains a bitstream that will configure the FPGA to provide an interface
	// between the parallel port and the programmable oscillator
	if(strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].oscBitstreamFile)==0)
	{
		string msg = brdModel + (string)" does not support oscillator programming!!\n";
		errMsg.SimpleMsg(XSErrorMajor,msg);
		return false;
	}

	// check the ID of the interface CPLD to see if it matches the type on this particular board.
	if(CheckChipID() == false)
		return false;
	
	// get the USERCODE from the interface CPLD to see if it supports programming the oscillator
	bool loadOscIntfc;
	string usercode = cpld.GetUSERCODE();
	if((usercode==flashIntfcCode) || (usercode==dwnldIntfcCode) || 
		(usercode==flashConfigIntfcCode) || (usercode==testIntfcCode))
	{ // the interface CPLD is not configured with the oscillator interface ...
		loadOscIntfc = true; // so force it to be loaded
	}
	else if(usercode==oscIntfcCode)
	{ // the CPLD already contains the oscillator interface bitstream
		loadOscIntfc = false;
	}
	else
	{ // can't tell what the CPLD contains, so load the oscillator interface
		loadOscIntfc = true;
	}

	// give the user some instructions on how to initialize the oscillator for programming
	string instructions;
	instructions = "Before setting the XSV Board frequency you must:\n";
	instructions += "1) Turn off the power to your XSV Board\n";
	instructions += "2) Remove the downloading cable from your XSV Board\n";
	instructions += "3) Place a shunt across pins 2 and 3 of jumpers J29, J30, and J31\n";
	instructions += "4) Place a shunt across the pins of jumper J23\n";
	instructions += "5) Place a shunt across pins 1 and 2 of jumpers J22 and J36\n";
	instructions += "6) Turn on the power to your XSV Board\n";
	instructions += "7) Reconnect the downloading cable\n";
	instructions += "8) Click on the OK button\n";
	if(PromptUser(instructions,PROMPT_OKCANCEL) == RESPONSE_CANCEL)
		return false;

	// load the CPLD with the interface that allows the programming of the oscillator thru the parallel port
	if(loadOscIntfc)
	{
		cpld.ConfigureCPLD(string(brdInfo[brdIndex].port[PORTTYPE_LPT].oscBitstreamFile));
		if(errMsg.IsError())
		{
			errMsg.SimpleMsg(XSErrorMajor,"Error downloading clock-set circuit!!\n");
			return false;
		}
	}

	// program the oscillator
	osc.SetOscFrequency(div,extOscPresent);

	// erase the oscillator interface from the CPLD
	cpld.ConfigureCPLD(string(brdInfo[brdIndex].port[PORTTYPE_LPT].eraseBitstreamFile));
	if(errMsg.IsError())
	{
		errMsg.SimpleMsg(XSErrorMajor,"Error erasing CPLD!!\n");
		return false;
	}

	// give the user some instructions on how to reset the oscillator so the programming will take effect
	instructions = "The frequency of your XSV Board has been set!!\n";
	instructions += "Now do these steps to activate the oscillator:\n";
	instructions += "1) Turn off the power to your XSV Board\n";
	instructions += "2) Remove the downloading cable from your XSV Board\n";
	instructions += "3) Move the shunt to pins 2 and 3 of jumper J22\n";
	instructions += "4) Leave the shunt across pins 1 and 2 of jumper J36\n";
	instructions += "5) Turn on the power to your XSV Board\n";
	instructions += "6) Reconnect the downloading cable\n";
	instructions += "7) Click on the OK button\n\n";
	instructions += "YOU MUST REPROGRAM THE CPLD INTERFACE ON YOUR XSV BOARD!!\n";
	PromptUser(instructions,PROMPT_OK);

	return true;
}


// Look at xsboard.h for a description of the interface.
bool XSVBoard::SetupAudio(int *reg)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	string msg = brdModel + (string)" does not support codec programming!!\n";
	errMsg.SimpleMsg(XSErrorMajor,msg);
	return false;
}


// Look at xsboard.h for a description of the interface.
bool XSVBoard::SetupVideoIn(string& fileName)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	// if there is no osc interface bitstream file, then this board model does not support the video codec either
	if(strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].oscBitstreamFile)==0)
	{
		string msg = brdModel + (string)" does not support oscillator programming!!\n";
		errMsg.SimpleMsg(XSErrorMajor,msg);
		return false;
	}

	// check the ID of the interface CPLD to see if it matches the type on this particular board.
	if(CheckChipID() == false)
		return false;
	
	// Get the name of the file that contains a bitstream that will configure the CPLD to provide an interface
	// between the parallel port and the video codec. The video codec interface is in the same directory as the osc interface file.
	string bitFileName = GetPrefix(string(brdInfo[brdIndex].port[PORTTYPE_LPT].oscBitstreamFile));
	bitFileName += "/";
	bitFileName += "cfgvidin.bit";

	// download the codec interface into the FPGA
	if(Configure(bitFileName)==false)
	{
		errMsg.SimpleMsg(XSErrorMajor,"Error downloading video input setup circuit!!\n");
		return false;
	}

	// give the user feedback on the results of configuring the codec interface
	if(videoin.Configure(fileName) == true)
	{
		string instructions = "The video input of your XSV Board has been configured!!\n";
		PromptUser(instructions,PROMPT_OK);
		return true;
	}
	else
	{
		string instructions = "An error occured while configuring the video input!!\n";
		PromptUser(instructions,PROMPT_OK);
		return false;
	}
}


// Look at xsboard.h for a description of the interface.
bool XSVBoard::Test(void)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	// if there is no diagnostic bitstream file, then this board model does not support diagnostics
	string msg;
	if(strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].testIntfcBitstreamFile)==0)
	{
		msg = brdModel + (string)" does not support self-test!!\n";
		errMsg.SimpleMsg(XSErrorMajor,msg);
		return false;
	}

	// check the ID of the interface CPLD to see if it matches the type on this particular board.
	if(CheckChipID() == false)
		return false;
	
	// get the USERCODE from the interface CPLD to see if it supports running diagnostics
	bool loadTestIntfc;
	string usercode = cpld.GetUSERCODE();
	if((usercode==oscIntfcCode) || (usercode==flashIntfcCode) || 
		(usercode==flashConfigIntfcCode) || (usercode==dwnldIntfcCode))
	{ // the interface CPLD is not configured with the standard testing interface...
		loadTestIntfc = true; // so force it to be loaded
	}
	else if(usercode==testIntfcCode)
	{ // the CPLD already contains the standard testing interface bitstream
		loadTestIntfc = false;
	}
	else
	{ // can't tell what the CPLD contains, so load standard testing interface
		loadTestIntfc = true;
	}

	if(loadTestIntfc)
	{
		// load the CPLD with the diagnostic interface
		if(ConfigureInterface(string(brdInfo[brdIndex].port[PORTTYPE_LPT].testIntfcBitstreamFile)) == false)
		{
			errMsg.SimpleMsg(XSErrorMajor,"Error configuring CPLD with self-test interface circuit!\n");
			return false;
		}
	}

	// configure the FPGA with the diagnostic circuit
	if(Configure(string(brdInfo[brdIndex].port[PORTTYPE_LPT].testBitstreamFile)) == false)
	{
		errMsg.SimpleMsg(XSErrorMajor,"Error downloading XSV Board self-test circuit!\n");
		return false;
	}

	// display a progress indicator while the diagnostic is running
	string desc((string)"Testing " + brdModel), subdesc("Testing...");
	Progress* progressGauge = new Progress(&errMsg,desc,subdesc,0,100);
	int testProgress = 0;

	// Monitor the test status pin to see if the test is still running or not.
	// While the diagnostic runs, the test status pin pulses at some rate.
	// The number of pulses within the integration period is counted and one of the following actions is taken:
	//    1. If the count is ever 0, then the test is done and the board has failed.
	//    2. If the count increases by a factor of two or more, then the test is done and the board passed.
	//    3. Otherwise, the test is still running.
	int prevStatus = cpld.In(posTESTSTATUS,posTESTSTATUS);
	int currStatus;
	int prevDiff = 0;
	int currDiff;
	const float integrationPeriod = 0.5;	// in seconds
	while(true)
	{
		// count the number of pulses on the status pin during the integration period
		clock_t startTime = clock();
		clock_t endTime = startTime + integrationPeriod * CLOCKS_PER_SEC;
		currDiff = 0; // pulse counter starts at 0
		while(clock()<endTime)
		{
			currStatus = cpld.In(posTESTSTATUS,posTESTSTATUS);
			if(currStatus != prevStatus)
				currDiff++;
			prevStatus = currStatus;
		}
		
		// update the progress indicator
		testProgress = testProgress>=100 ? 0 : testProgress+10;
		progressGauge->Report(testProgress);
		
		if(currDiff == 0)
		{	// Test is done and the board failed.  Give the user some help.
			msg = (string)"\nYour " + brdModel + (string)" failed the test!\n\n";
			msg += "Check the following:\n";
			msg += " 1) Is your board connected to the PC parallel port?\n";
			msg += " 2) Is your PC parallel port in unidirectional mode?\n";
			msg += " 3) Is your board the only device connected to the parallel port?\n";
			msg += " 4) Is your board getting power?\n";
			msg += " 5) Is your board sitting on a non-conductive surface?\n";
			msg += " 6) Is the programmable oscillator frequency greater than 1 MHz?\n";
			msg += " 7) Is the shunt on jumper J23?\n";
			msg += " 8) Is the shunt on pins 2-3 of jumper J31?\n";
			msg += " 9) Is the shunt on pins 2-3 of jumper J22?\n";
			msg += "10) Is the shunt on pins 1-2 of jumper J36?\n";
			msg += "11) Is the shunt on pins 1-2 of jumper J32?\n";
			errMsg.SimpleMsg(XSErrorMajor,msg);
			delete progressGauge;
			return false;
		}
		else
		{
			if((currDiff > 2*prevDiff) && (prevDiff>0))
			{	// test is done and the board passed
				msg = (string)"\nYour " + brdModel + (string)" passed the test!\n";
				PromptUser(msg,PROMPT_OK);
				delete progressGauge;
				return true;
			}
		}

		// the test is still running
		prevDiff = currDiff;	// save the current number of pulses to compare during the next integration period 
	}

	delete progressGauge;

	return false;
}


// Look at xsboard.h for a description of the interface.
unsigned char XSVBoard::ApplyTestVectors(unsigned char singleVector, unsigned char mask, unsigned char *vector,
											unsigned char *response, unsigned int numVectors)
{
	assert(numVectors!=0);
	assert((numVectors>1 && vector!=NULL) || (numVectors==1 && vector==NULL));
	assert((numVectors>1 && response!=NULL) || (numVectors==1 && response==NULL));
	
	if(numVectors==1)
	{
		return testPort.ApplyTestVector(singleVector,mask);
	}
	else
	{
		int i;
		for(i=0; i<numVectors; i++)
		{
			response[i] = testPort.ApplyTestVector(vector[i],mask);		
		}
		return 0;
	}
}


// Look at xsboard.h for a description of the interface.
unsigned char XSVBoard::GetTestVector(void)
{
	return testPort.GetTestVector();
}


// Look at xsboard.h for a description of the interface.
bool XSVBoard::DownloadRAMFromIntArray(unsigned *intArray, unsigned address, unsigned numInts)			
{
	return false; // This method is not implemented.
}


// Look at xsboard.h for a description of the interface.
bool XSVBoard::UploadRAMToIntArray(unsigned *intArray, unsigned address, unsigned numInts)			
{
	return false; // This method is not implemented.
}
