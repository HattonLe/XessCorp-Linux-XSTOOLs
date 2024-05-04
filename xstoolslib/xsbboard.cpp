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


#include <ctime>
#include <string>

#include <string.h>

using namespace std;

#include "utils.h"
#include "xserror.h"
#include "xsbboard.h"

#include "../xstoolslib/guitools.h"

// inversion mask for parallel port connection to XSB Board
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
static const unsigned int posOSCSCLW = 1;
static const unsigned int posOSCSCLR = 11;
static const unsigned int posOSCSDAW = 6;
static const unsigned int posOSCSDAR = 12;

// bit position of audio codec pins within parallel port registers
static const unsigned int posCDCCCLK = 1;
static const unsigned int posCDCCSNN = 5;
static const unsigned int posCDCCDTI = 6;
static const unsigned int posCDCCDTO = 12;

// bit positions of peripheral bus access pins within parallel port
static const unsigned int posRRESET	= 6;
static const unsigned int posRCLK   = 1;
static const unsigned int posRDOLSB	= 2;
static const unsigned int posRDOMSB	= 5;
static const unsigned int posRDILSB	= 11;
static const unsigned int posRDIMSB	= 13;
static const unsigned int posRSTLSB = 11;
static const unsigned int posRSTMSB = 13;

// bit positions of Flash access pins within parallel port
static const unsigned int posFRESET	=  0;
static const unsigned int posFCLK	=  1;
static const unsigned int posFDOLSB	=  2;
static const unsigned int posFDOMSB	=  5;
static const unsigned int posFDILSB	= 11;
static const unsigned int posFDIMSB	= 13;
static const unsigned int posFSTLSB	= 11;
static const unsigned int posFSTMSB	= 13;

// bit positions for board test status
static const unsigned int posTESTSTATUS	= 13;

// bit position of SAA711X pins within parallel port registers
static const unsigned int posSAASCLW = 1;
static const unsigned int posSAASCLR = 11;
static const unsigned int posSAASDAW = 6;
static const unsigned int posSAASDAR = 12;

// USERCODE strings for various circuits programmed into the XSB CPLD
static const string oscIntfcCode			= "<0>!";
static const string flashIntfcCode			= "<1>!";
static const string flashConfigIntfcCode	= "<2>!";
static const string testIntfcCode			= "<4>!";
static const string dwnldIntfcCode			= "<4>!";


/// Create an XSB Board object.
XSBBoard::XSBBoard(void)
{
	XSError* err = new XSError(cerr);	// create error-reporting channel
	brdModel = NULL;	// no particular XSA board model, yet
	Setup(err,"XSB-300E",1);	// use a default board model and parallel port number at this point
}


/// Destroy the XSB Board object.
XSBBoard::~XSBBoard(void)
{
	;
}


// Look at xsboard.h for a description of the interface.
void XSBBoard::SetFlags(unsigned long f)
{
	flags = f;
}


// Look at xsboard.h for a description of the interface.
unsigned long XSBBoard::GetFlags(void)
{
	return flags;
}


// Look at xsboard.h for a description of the interface.
bool XSBBoard::Setup(XSError* err, const char* model, unsigned int lptNum)
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
		err->SimpleMsg(XSErrorFatal,"Unknown type of XSB Board!\n");
	int invMask = brdInfo[brdIndex].port[PORTTYPE_LPT].invMask;

	// initialize the objects for the components on the board
	bool status;
	status = testPort.Setup(err,lptNum,invMask,posTVCLK,posTVOLO,posTVOHI,posTVILO,posTVIHI);
	status = status && fpga.Setup(err,lptNum,invMask,posCCLK,posPROG,posDLO,posDHI,posDONE);
	status = status && cpld.Setup(err,lptNum,invMask,posTCK,posTMS,posTDI,posTDO);
	status = status && osc.Setup(err,lptNum,invMask,posOSCSCLW,posOSCSDAW,posOSCSCLR,posOSCSDAR);
	status = status && ram.Setup(err,lptNum,invMask,posRRESET,posRCLK,posRDOLSB,posRDOMSB,posRDILSB,posRDIMSB,posRSTLSB,posRSTMSB,16,28);
	status = status && flash.Setup(err,lptNum,invMask,posFRESET,posFCLK,posFDOLSB,posFDOMSB,posFDILSB,posFDIMSB,posFSTLSB,posFSTMSB);
	status = status && codec.Setup(err,lptNum,invMask,posCDCCCLK,posCDCCSNN,posCDCCDTI,posCDCCDTO);
	status = status && videoin.Setup(err,lptNum,invMask,posSAASCLW,posSAASDAW,posSAASCLR,posSAASDAR);
	return status;	// return true if no error occurred for any object setup
}

// check the interface chip ID and make sure it is an XC9572XL
bool XSBBoard::CheckChipID(void)
{
	// get the chip ID from the interface cpld and compare it to the chip ID for
	// an XC9572XL CPLD (ignoring the first 4 bits which increment for each chip revision)
	char* XC9572XLID = "1001011000000100000010010011";

	string chipID = cpld.GetChipID();
    if (strncmp(XC9572XLID,chipID.c_str()+4,strlen(XC9572XLID)))
	{
		string instructions;
		instructions = "The interface CPLD of your XSB Board is not responding!!\n\n";
		instructions += "\nChip ID = ";
		instructions += chipID;		// display the ID that was received
		instructions += "\n\n";
		instructions += "1) Is power getting to your XSB Board?\n";
		instructions += "2) Is the downloading cable attached?\n";
		instructions += "\nContinue anyway\n?";
        if (GuiTools::PromptUser(instructions,PROMPT_OKCANCEL) == RESPONSE_CANCEL)
        {
			return false;
        }
	}
	return true;	// ID matched
}


// Look at xsboard.h for a description of the interface.
bool XSBBoard::Configure(const char *fileName, bool *UserCancelled)
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
        type = ConvertToUpperCase(type);
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
		if((usercode==flashIntfcCode) || (usercode==flashConfigIntfcCode))
		{ // the interface CPLD is not configured with the standard downloading circuit
            if(ConfigureInterface(brdInfo[brdIndex].port[PORTTYPE_LPT].dwnldIntfcBitstreamFile) == false)
			{
				errMsg.SimpleMsg(XSErrorMajor,"Error configuring CPLD with downloading interface circuit!\n");
				return false;
			}
			fpga.EnableFastDownload(true);
		}
		else if((usercode==dwnldIntfcCode) || (usercode==testIntfcCode) || (usercode==oscIntfcCode))
		{ // the CPLD contains the fast downloading interface
			fpga.EnableFastDownload(true);
		}
		else
		{ // can't tell what the CPLD contains, so assume the user loaded it with a low-speed interface
			fpga.EnableFastDownload(false);
		}
		
		// initialize and then configure the FPGA with the contents of the bitstream file
		fpga.InitConfigureFPGA();
		return fpga.ConfigureFPGA(fileName);
	}
	else if(suffix=="SVF")
	{ // configure the CPLD with an SVF bitstream
        return ConfigureInterface(fileName, UserCancelled);
	}
	
	// neither a .BIT or .SVF file, so report the error
	errMsg.SimpleMsg(XSErrorMajor,"Only .BIT or .SVF files can be downloaded into the FPGA or CPLD on the XSB Board!!");
	return false;
}


// Look at xsboard.h for a description of the interface.
bool XSBBoard::ConfigureInterface(const char *fileName, bool *UserCancelled)
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
    return cpld.ConfigureCPLD(fileName, UserCancelled);
}


// Look at xsboard.h for a description of the interface.
bool XSBBoard::DownloadRAM(const char *fileName, bool bigEndianBytes,
			bool bigEndianBits, bool doStart, bool doEnd)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	// check the file suffix to see if it is appropriate for downloading to RAM
	string suffix = GetSuffix(fileName);
	if(suffix!="HEX" && suffix!="EXO" && suffix!="XES" && suffix!="MCS")
	{
		errMsg.SimpleMsg(XSErrorMajor,"Only .HEX, .MCS, .EXO or .XES files can be downloaded into the RAM on the XSB Board!!\n");
		return false;
	}

	// get the name of the file that contains a bitstream that will configure the FPGA to provide an interface
	// between the parallel port and the RAM.
	if(strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile)==0)
	{
		// it is an error if there is no RAM download interface configuration for this board
		string msg = brdModel + (string)" does not support RAM downloads!!\n";
		errMsg.SimpleMsg(XSErrorMajor,msg);
		return false;
	}

	// only download the RAM interface if this is the first download of data to the RAM.
	// otherwise the interface should already be in place.
	if(doStart)
	{
        if(Configure(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile) == false)
		{
			errMsg.SimpleMsg(XSErrorMajor,"Error downloading RAM interface circuit!!\n");
			return false;
		}
	}

	// download data to the RAM
	if(ram.Download(fileName,bigEndianBytes,bigEndianBits) == false)
	{
		errMsg.SimpleMsg(XSErrorMajor,"Error downloading into RAM!!\n");
		return false;
	}

	return true;
}


// Look at xsboard.h for a description of the interface.
bool XSBBoard::UploadRAM(const char *fileName, const char* format,
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

	// only download the RAM interface if this is the first download of data to the RAM.
	// otherwise the interface should already be in place.
	if(doStart)
	{
        if(Configure(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile) == false)
		{
			errMsg.SimpleMsg(XSErrorMajor,"Error downloading RAM interface circuit!!\n");
			return false;
		}
	}

	// upload data from the RAM
	if(ram.Upload(fileName,format,loAddr,hiAddr,bigEndianBytes,bigEndianBits) == false)
	{
		errMsg.SimpleMsg(XSErrorMajor,"Error uploading from RAM!!\n");
		return false;
	}
	return true;
}


// Look at xsboard.h for a description of the interface.
bool XSBBoard::ReadRAM(unsigned int addr, unsigned int* data,	
		bool bigEndianBytes, bool bigEndianBits)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	if(ram.Read(addr,data,bigEndianBytes,bigEndianBits) == false)
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
        if(Configure(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile) == false)
		{
			errMsg.SimpleMsg(XSErrorMajor,"Error downloading RAM interface circuit!!\n");
			return false;
		}

		// now try reading the RAM again
		return ram.Read(addr,data,bigEndianBytes,bigEndianBits);	// now try reading the RAM again
	}
	
	return true;	// RAM read succeeded
}


// Look at xsboard.h for a description of the interface.
bool XSBBoard::WriteRAM(unsigned int addr, unsigned int data,	
		bool bigEndianBytes, bool bigEndianBits)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	if(ram.Write(addr,data,bigEndianBytes,bigEndianBits) == false)
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
        if(Configure(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile) == false)
		{
			errMsg.SimpleMsg(XSErrorMajor,"Error downloading RAM interface circuit!!\n");
			return false;
		}

		// now try writing the RAM again
		return ram.Write(addr,data,bigEndianBytes,bigEndianBits);	// now try writing to the RAM again
	}
	
	return true;	// RAM write succeeded
}


// Look at xsboard.h for a description of the interface.
bool XSBBoard::DownloadFlash(const char *fileName, bool bigEndianBytes,
            bool bigEndianBits, bool doStart, bool doEnd, bool *UserCancelled)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	// check the file suffix to see if it is appropriate for downloading to Flash
	string suffix = GetSuffix(fileName);
	if(suffix!="HEX" && suffix!="EXO" && suffix!="XES" && suffix!="MCS")
	{
		errMsg.SimpleMsg(XSErrorMajor,"Only .HEx, .MCS, .EXO or .HEX files can be downloaded into the Flash on the XSB Board!!\n");
		return false;
	}

	// get the name of the file that contains a bitstream that will configure the FPGA to provide an interface
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
	{ // the interface CPLD is not configured with the Flash programming interface...
		loadFlashIntfc = true;	// so force it to be loaded
	}
	else if(usercode==flashIntfcCode)
	{ // the CPLD already contains the Flash programming interface bitstream
		loadFlashIntfc = false;
	}
	else
	{ // can't tell what the CPLD contains, so load the Flash programming interface
		loadFlashIntfc = true;
	}

	// If this is the first data file being downloaded to the Flash, then configure the CPLD with the Flash interface.
	if(doStart || loadFlashIntfc)
	{
        if(ConfigureInterface(brdInfo[brdIndex].port[PORTTYPE_LPT].flashBitstreamFile) == false)
		{
			errMsg.SimpleMsg(XSErrorMajor,"Error configuring CPLD with Flash programming circuit!\n");
			return false;
		}
	}
	
	// download data into the Flash
	if(flash.DownloadFlash(fileName,bigEndianBytes,bigEndianBits,doStart) == false)
	{
		errMsg.SimpleMsg(XSErrorMajor,"Error programming the Flash on the XSB Board!\n");
		return false;
	}
	
	// if this is the last file to be downloaded to Flash, then reprogram the CPLD with a circuit
	// that will make the CPLD load the FPGA with the contents of the Flash upon power-up.
	if(doEnd)
	{
        if(ConfigureInterface(brdInfo[brdIndex].port[PORTTYPE_LPT].flashConfigBitstreamFile) == false)
		{
			errMsg.SimpleMsg(XSErrorMajor,"Error configuring CPLD with Flash configuration circuit!\n");
			return false;
		}
	}

	return true;
}


// Look at xsboard.h for a description of the interface.
bool XSBBoard::UploadFlash(const char *fileName, const char* format,
				unsigned int loAddr, unsigned int hiAddr,
				bool bigEndianBytes, bool bigEndianBits,	
				bool doStart, bool doEnd)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	// get the name of the file that contains a bitstream that will configure the FPGA to provide an interface
	// between the parallel port and the Flash.
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
	{ // the interface CPLD is not configured with the standard Flash interface interface ...
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
        if(ConfigureInterface(brdInfo[brdIndex].port[PORTTYPE_LPT].flashBitstreamFile) == false)
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
    if(ConfigureInterface(brdInfo[brdIndex].port[PORTTYPE_LPT].flashConfigBitstreamFile) == false)
	{
		errMsg.SimpleMsg(XSErrorMajor,"Error configuring CPLD with Flash configuration circuit!\n");
		return false;
	}

	return true;
}


// Look at xsboard.h for a description of the interface.
bool XSBBoard::SetFreq(int div, bool extOscPresent)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	// get the name of the file that contains a bitstream that will configure the FPGA to provide an interface
	// between the parallel port and the oscillator
	if(strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].oscBitstreamFile)==0)
	{
		string msg = brdModel + (string)" does not support oscillator programming!!\n";
		errMsg.SimpleMsg(XSErrorMajor,msg);
		return false;
	}

	// check the ID of the interface CPLD to see if it matches the type on this particular board.
	if(CheckChipID() == false)
		return false;

	// the JEDEC file with the default programming for the oscillator is in the same directory as the osc interface file
	string jedecFileName = GetPrefix(string(brdInfo[brdIndex].port[PORTTYPE_LPT].oscBitstreamFile));
	jedecFileName += "/";
	jedecFileName += "cy22393.jed"; // we shouldn't be hard-coding the file name for the osc programming

	// download the oscillator interface into the FPGA
    if (!Configure(brdInfo[brdIndex].port[PORTTYPE_LPT].oscBitstreamFile, UserCancelled))
	{
		errMsg.SimpleMsg(XSErrorMajor,"Error downloading clock-set circuit!!\n");
		return false;
	}
	
	// download the default programming for the oscillator and the new frequency divisor
	osc.SetOscFrequency(div,jedecFileName);
	
	// tell the user the oscillator has been programmed
	string instructions = "The frequency of your XSB Board has been set!!\n";
    GuiTools::PromptUser(instructions,PROMPT_OK);

	return true;
}


// Look at xsboard.h for a description of the interface.
bool XSBBoard::SetupAudio(int *reg)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	// use general-purpose flags to turn ON or OFF the configuration of the interface to the audio codec.
	// this is useful if we want to do interactive setting of codec registers without incurring bitstream download delays.
	if(!(flags & 1))
	{
        bool UserCancelled;

		if(strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].oscBitstreamFile)==0)
		{
			string msg = brdModel + (string)" does not support audio codec programming!!\n";
			errMsg.SimpleMsg(XSErrorMajor,msg);
			return false;
		}
		
		// check the ID of the interface CPLD to see if it matches the type on this particular board.
		if(CheckChipID() == false)
			return false;
		
		// Get the name of the file that contains a bitstream that will configure the CPLD to provide an interface
		// between the parallel port and the audio codec. The audio codec interface is in the same directory as the osc interface file.
		string bitFileName = GetPrefix(string(brdInfo[brdIndex].port[PORTTYPE_LPT].oscBitstreamFile));
		bitFileName += "/";
		bitFileName += "cfgcodec.bit"; // codec interface file name is hard-coded.
		
		// download the codec interface into the FPGA
        if (!Configure(bitFileName.c_str(), UserCancelled))
		{
			errMsg.SimpleMsg(XSErrorMajor,"Error downloading codec setup circuit!!\n");
			return false;
		}
	}

	// download register values into the codec
	bool status = codec.Configure(reg);

	if(!(flags & 1))
	{
		// give the user feedback on the results of configuring the codec interface
		if(status == true)
		{
			string instructions = "The codec of your XSB Board has been configured!!\n";
            GuiTools::PromptUser(instructions,PROMPT_OK);
			return true;
		}
		else
		{
			string instructions = "An error occured while configuring the XSB codec!!\n";
            GuiTools::PromptUser(instructions,PROMPT_OK);
			return false;
		}
	}
	return status;
}


// Look at xsboard.h for a description of the interface.
bool XSBBoard::SetupVideoIn(string& fileName)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	// if there is no osc interface bitstream file, then this board model does not support the video codec either
	if(strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].oscBitstreamFile)==0)
	{
		string msg = brdModel + (string)" does not support video codec programming!!\n";
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
	bitFileName += "cfgi2c.bit";

	// download the codec interface into the FPGA
    if (!Configure(bitFileName.c_str(), , UserCancelled))
	{
		errMsg.SimpleMsg(XSErrorMajor,"Error downloading video input setup circuit!!\n");
		return false;
	}

	// give the user feedback on the results of configuring the codec interface
	if(videoin.Configure(fileName) == true)
	{
		string instructions = "The video input of your XSB Board has been configured!!\n";
        GuiTools::PromptUser(instructions,PROMPT_OK);
		return true;
	}
	else
	{
		string instructions = "An error occured while configuring the video input!!\n";
        GuiTools::PromptUser(instructions,PROMPT_OK);
		return false;
	}
}


// Look at xsboard.h for a description of the interface.
bool XSBBoard::Test(void)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel
    bool UserCancelled;

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
        if (!ConfigureInterface(brdInfo[brdIndex].port[PORTTYPE_LPT].testIntfcBitstreamFile, UserCancelled))
		{
			errMsg.SimpleMsg(XSErrorMajor,"Error configuring CPLD with self-test interface circuit!\n");
			return false;
		}
	}

#if 0
	if(Configure(string(brdInfo[brdIndex].oscBitstreamFile))==false)
	{
		errMsg.SimpleMsg(XSErrorMajor,"Error downloading SDRAM clock-set circuit!!\n");
		return false;
	}
	// the SDRAM clock has two divider registers
	osc.WriteReg(0x0a,8);
	osc.WriteReg(0x0b,8);
#endif

	// configure the FPGA with the diagnostic circuit
    if(Configure(brdInfo[brdIndex].port[PORTTYPE_LPT].testBitstreamFile) == false)
	{
		errMsg.SimpleMsg(XSErrorMajor,"Error downloading XSB Board self-test circuit!\n");
		return false;
	}

	// display a progress indicator while the diagnostic is running
	string desc((string)"Testing " + (string)brdModel), subdesc("Testing...");
    Progress* progressGauge = new Progress(NULL, &errMsg);
    if (NULL != progressGauge)
    {
        progressGauge->Setup(desc,subdesc,0,100);
    }
    int testProgress = 0;

	// Monitor the test status pin to see if the test is still running or not.
	// While the diagnostic runs, the test status pin pulses at some rate with 50% duty-cycle.
	// As soon as the duty cycle deviates from 50% over the intergration period, one of the following actions is taken:
	//    1. If the duty cycle is less than 33% then the test is done and the board has failed.
	//    2. If the duty cycle is greater than 66% then the test is done and the board has failed.
	//    3. If no pulses are seen during the integration period, then something has happened to the board or interface.
	//    4. Otherwise, the test is still running.
	const double integrationPeriod = 0.1; // seconds
	while(true)
	{
		// count the amount of time the status pin is high or low during the integration period
		clock_t startTime = clock();
		clock_t endTime = startTime + integrationPeriod * CLOCKS_PER_SEC;
		unsigned int hiCnt = 0;
		unsigned int loCnt = 0;
		while(clock() < endTime)
		{
			if(cpld.In(posTESTSTATUS,posTESTSTATUS) == 1)
				hiCnt++;
			else
				loCnt++;
		}
		if((hiCnt==0) || (loCnt==0))
		{ // no pulses from test circuit, so something is wrong with the board clock
			msg = (string)"\nYour " + brdModel + (string)" failed the test (no clock)!\n\n";
			msg += "Check the following:\n";
			msg += "1) Is your board connected to the PC parallel port?\n";
			msg += "2) Is your PC parallel port in unidirectional mode?\n";
			msg += "3) Is your board the only device connected to the parallel port?\n";
			msg += "4) Is your board getting power?\n";
			msg += "5) Is your board sitting on a non-conductive surface?\n";
			msg += "6) Is the shunt on the \'xs\' position of jumper JP1?\n";
			errMsg.SimpleMsg(XSErrorMajor,msg);
			delete progressGauge;
			return false;
		}
		if(loCnt/2 > hiCnt)
		{ // duty cycle is less than 1/3 so the board test failed
			msg = (string)"\nYour " + brdModel + (string)" failed the test!\n\n";
			msg += "Check the following:\n";
			msg += "1) Is your board connected to the PC parallel port?\n";
			msg += "2) Is your PC parallel port in unidirectional mode?\n";
			msg += "3) Is your board the only device connected to the parallel port?\n";
			msg += "4) Is your board getting power?\n";
			msg += "5) Is your board sitting on a non-conductive surface?\n";
			msg += "6) Is the shunt on the \'xs\' position of jumper JP1?\n";
			errMsg.SimpleMsg(XSErrorMajor,msg);
			delete progressGauge;
			return false;
		}
		if(hiCnt/2 > loCnt)
		{ // duty cycle is greater than 2/3 so the board test passed
			msg = (string)"\nYour " + brdModel + (string)" passed the test!\n";
            GuiTools::PromptUser(msg,PROMPT_OK);
			delete progressGauge;
			return true;
		}
		// otherwise, the test is still in-progress if the duty-cycle is in the range [1/2, 2/3]
	}

	delete progressGauge;

	return false;
}


// Look at xsboard.h for a description of the interface.
unsigned char XSBBoard::ApplyTestVectors(unsigned char singleVector, unsigned char mask, unsigned char *vector,
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
unsigned char XSBBoard::GetTestVector(void)
{
	return testPort.GetTestVector();
}


// Look at xsboard.h for a description of the interface.
bool XSBBoard::DownloadRAMFromIntArray(unsigned *intArray, unsigned address, unsigned numInts)			
{
	return false; // This method is not implemented.
}


// Look at xsboard.h for a description of the interface.
bool XSBBoard::UploadRAMToIntArray(unsigned *intArray, unsigned address, unsigned numInts)			
{
	return false; // This method is not implemented.
}
