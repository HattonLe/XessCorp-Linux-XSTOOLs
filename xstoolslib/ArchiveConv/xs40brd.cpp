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

#include "xserror.h"
#include "utils.h"
#include "xs40brd.h"


// inversion mask for parallel port connection to XS40 Board
// 00000011: bits  7- 0 are attached to data pins D7-D0
// 00000xxx: bits 15-11 are attached to status pins /S7,S6,S5,S4,S3
// xxxx1001: bits 19-16 are attached to control pins /C3,C2,/C1,/C0
// const unsigned int invMask	= 0x090003;

// bit position of XC4000 configuration pins within parallel port registers
static const unsigned int posCCLK		= 17;
static const unsigned int posPROG		= 16;
static const unsigned int posDIN		= 19;
static const unsigned int posDONE		= 23;	// not used

// bit positions of test vector input and output
static const unsigned int posTVCLK = 0;
static const unsigned int posTVOLO = 0;
static const unsigned int posTVOHI = 7;
static const unsigned int posTVILO = 11;
static const unsigned int posTVIHI = 14;

// bit position of prog. osc. pins within parallel port registers
static const unsigned int posOSC		= 0;

// bit positions of RAM access pins within parallel port
static const unsigned int posRESET		= 0;
static const unsigned int posCLK		= 1;
static const unsigned int posDOLSB		= 2;
static const unsigned int posDOMSB		= 5;
static const unsigned int posDILSB		= 11;
static const unsigned int posDIMSB		= 14;
static const unsigned int posSTLSB		= 11;
static const unsigned int posSTMSB		= 14;

// bit positions of serial EEPROM access pins within parallel port
static const unsigned int posSEECLK		= 17;
static const unsigned int posSEEOE		= 6;
static const unsigned int posSEECE		= 7;
static const unsigned int posSEEDIN		= 19;

// bit positions for board test status
static const unsigned int posTESTSTATUS	= 14;
static const unsigned int posTESTRESET	= 5;


/// Create an XS40 Board object.
XS40Board::XS40Board(void)
{
	XSError* err = new XSError(cerr);	// create error-reporting channel
	brdModel = NULL;	// no particular XS40 board model, yet
	Setup(err,"XS40-005XL",1);	// use a default board model and parallel port number at this point
}


/// Destroy an XS40 Board object.
XS40Board::~XS40Board(void)
{
	;
}


// Look at xsboard.h for a description of the interface.
void XS40Board::SetFlags(unsigned long f)
{
	flags = f;
}


// Look at xsboard.h for a description of the interface.
unsigned long XS40Board::GetFlags(void)
{
	return flags;
}


// Look at xsboard.h for a description of the interface.
bool XS40Board::Setup(XSError* err, const char* model, unsigned int lptNum)
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
		err->SimpleMsg(XSErrorFatal,"Unknown type of XS40 Board!\n");
	int invMask = brdInfo[brdIndex].port[PORTTYPE_LPT].invMask;

	// initialize the objects for the components on the board
	bool status;
	status = testPort.Setup(err,lptNum,invMask,posTVCLK,posTVOLO,posTVOHI,posTVILO,posTVIHI);
	status = status && fpga.Setup(err,lptNum,invMask,posCCLK,posPROG,posDIN,posDONE);
	status = status && osc.Setup(err,lptNum,invMask,posOSC);
	status = status && ram.Setup(err,lptNum,invMask,posRESET,posCLK,posDOLSB,posDOMSB,posDILSB,posDIMSB,posSTLSB,posSTMSB,8,20);
	status = status && seeprom.Setup(err,lptNum,invMask,posSEECLK,posSEECE,posSEEOE,posSEEDIN);
	return status;	// return true if no error occurred for any object setup
}


// Look at xsboard.h for a description of the interface.
bool XS40Board::Configure(string& fileName)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel
	
	// check the type of the file that is going to be downloaded
	string suffix = GetSuffix(fileName);
	if(suffix!="BIT")
	{
		errMsg.SimpleMsg(XSErrorMajor,"Only .BIT files can be downloaded into the FPGA on the XS40 Board!!\n");
		return false;
	}

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
		string msg;
		msg = ".BIT file is for the " + type + " but this is not the FPGA on the " + brdModel + "!!\n";
		errMsg.SimpleMsg(XSErrorMajor,msg);
		return false;
	}
	
	// initialize and then configure the FPGA with the contents of the bitstream file
	fpga.InitConfigureFPGA();
	return fpga.ConfigureFPGA(fileName);
}


// Look at xsboard.h for a description of the interface.
bool XS40Board::ConfigureInterface(string& fileName)
{
	return false;	// there is no interface CPLD on the XS40 Board, so this is an error
}


// Look at xsboard.h for a description of the interface.
bool XS40Board::DownloadRAM(string& fileName, bool bigEndianBytes,
			bool bigEndianBits, bool doStart, bool doEnd)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	// check the type of the file that is going to be downloaded
	string suffix = GetSuffix(fileName);
	if(suffix!="HEX" && suffix!="EXO" && suffix!="XES" && suffix!="MCS")
	{
		errMsg.SimpleMsg(XSErrorMajor,"Only .HEX, .MCS, .EXO or .XES files can be downloaded into the RAM on the XS40 Board!!\n");
		return false;
	}

	// get the name of the file that contains a bitstream that will configure the FPGA to provide an interface
	// between the parallel port and the RAM
	if(strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile)==0)
	{
		string msg;
		msg = brdModel + (string)" does not support RAM downloads!!\n";
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
bool XS40Board::UploadRAM(string& fileName, const char* format,	
				unsigned int loAddr, unsigned int hiAddr,
				bool bigEndianBytes, bool bigEndianBits,	
				bool doStart, bool doEnd)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	// get the name of the file that contains a bitstream that will configure the FPGA to provide an interface
	// between the parallel port and the RAM
	if(strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile)==0)
	{
		string msg;
		msg = brdModel + (string)" does not support RAM uploads!!\n";
		errMsg.SimpleMsg(XSErrorMajor,msg);
		return false;
	}

	// only upload the RAM interface if this is the first upload of data from the RAM.
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
bool XS40Board::ReadRAM(unsigned int addr, unsigned int* data,	
		bool bigEndianBytes, bool bigEndianBits)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	if(ram.ReadRAM(addr,data,bigEndianBytes,bigEndianBits) == false)
	{ // couldn't read RAM so maybe RAM interface is not loaded???
		
		// get the name of the file that contains a bitstream that will configure the FPGA to provide an interface
		// between the parallel port and the RAM
		if(strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile)==0)
		{
			string msg;
			msg = brdModel + (string)" does not support RAM uploads!!\n";
			errMsg.SimpleMsg(XSErrorMajor,msg);
			return false;
		}
		
		// configure the FPGA with the RAM interface
		if(Configure(string(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile)) == false)
		{
			errMsg.SimpleMsg(XSErrorMajor,"Error downloading RAM interface circuit!!\n");
			return false;
		}

		// now try reading the RAM again
		return ram.ReadRAM(addr,data,bigEndianBytes,bigEndianBits);
	}
	
	return true;	// RAM read succeeded
}


// Look at xsboard.h for a description of the interface.
bool XS40Board::WriteRAM(unsigned int addr, unsigned int data,	
		bool bigEndianBytes, bool bigEndianBits)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	if(ram.WriteRAM(addr,data,bigEndianBytes,bigEndianBits) == false)
	{ // couldn't write to RAM so maybe RAM interface is not loaded???
		
		// get the name of the file that contains a bitstream that will configure the FPGA to provide an interface
		// between the parallel port and the RAM
		if(strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile)==0)
		{
			string msg;
			msg = brdModel + (string)" does not support RAM uploads!!\n";
			errMsg.SimpleMsg(XSErrorMajor,msg);
			return false;
		}
		
		// configure the FPGA with the RAM interface
		if(Configure(string(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile)) == false)
		{
			errMsg.SimpleMsg(XSErrorMajor,"Error downloading RAM interface circuit!!\n");
			return false;
		}

		// now try writinging the RAM again
		return ram.WriteRAM(addr,data,bigEndianBytes,bigEndianBits);	// now try writing to the RAM again
	}
	
	return true;	// RAM write succeeded
}


// Look at xsboard.h for a description of the interface.
bool XS40Board::DownloadFlash(string& fileName, bool bigEndianBytes,
			bool bigEndianBits, bool doStart, bool doEnd)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	// check the type of the file that is going to be downloaded
	string suffix = GetSuffix(fileName);
	if(suffix!="BIT")
	{
		errMsg.SimpleMsg(XSErrorMajor,"Only .BIT files can be downloaded into the serial EEPROM on the XS40 Board!!\n");
		return false;
	}

	// use the board model name to find the corresponding index into the list of boards
	if(strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].flashBitstreamFile)==0)
	{
		string msg;
		msg = brdModel + (string)" does not support serial EEPROM programming!!\n";
		errMsg.SimpleMsg(XSErrorMajor,msg);
		return false;
	}

	// If this is the first data file being downloaded to the serial EEPROM, then give the user some instructions
	// that will initialize the hardware.
	string instructions;
	if(doStart)
	{
		instructions = "Before programming the XS40 Board serial EEPROM you must:\n";
		instructions += "1) Place shunts on jumpers J4, J6 and J11\n";
		instructions += "2) Remove any shunt on jumper J10\n";
		instructions += "3) Apply power to the board\n";
		if(PromptUser(instructions,PROMPT_OKCANCEL) == RESPONSE_CANCEL)
			return false;
		
		// configure the FPGA with the EEPROM interface
		if(Configure(string(brdInfo[brdIndex].port[PORTTYPE_LPT].flashBitstreamFile)) == false)
		{
			errMsg.SimpleMsg(XSErrorMajor,"Error downloading serial EEPROM interface circuit!!\n");
			return false;
		}
	}

	// download data into the serial EEPROM
	if(seeprom.ProgramEEPROM(fileName) == false)
	{
		errMsg.SimpleMsg(XSErrorMajor,"Error downloading into serial EEPROM!!\n");
		return false;
	}

	// If this was the last data file to download to the serial EEPROM, then give the user some instructions
	// that will reset the hardware.
	if(doEnd)
	{
		instructions = "The XS40 Board serial EEPROM has been programmed!!\n";
		instructions += "Now do these steps to enable configuration of your XS40 Board from the serial EEPROM:\n";
		instructions += "1) Remove the shunts from jumpers J4, J6 and J11\n";
		instructions += "2) Place a shunt on jumper J10\n";
		instructions += "3) Disconnect the parallel port cable\n";
		instructions += "4) Apply power to the board\n";
		PromptUser(instructions,PROMPT_OK);
	}

	return true;
}


// Look at xsboard.h for a description of the interface.
bool XS40Board::UploadFlash(string& fileName, const char* format,	
				unsigned int loAddr, unsigned int hiAddr,
				bool bigEndianBytes, bool bigEndianBits,	
				bool doStart, bool doEnd)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel
	errMsg.SimpleMsg(XSErrorMajor,"The serial EEPROM on the XS40 Board cannot be read back!!\n");
	return false;
}


// Look at xsboard.h for a description of the interface.
bool XS40Board::SetFreq(int div, bool extOscPresent)
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

	// give the user some instructions on how to initialize the oscillator for programming
	string instructions;
	instructions = "Before setting the XS40 Board frequency you must:\n";
	instructions += "1) Remove the power and downloading cable from your XS40 Board\n";
	instructions += "2) Place a shunt on the \"set\" position of jumper J12\n";
	instructions += "3) Reconnect the power cable\n";
	instructions += "4) Reconnect the downloading cable\n";
	instructions += "5) Click on the OK button\n";
	if(PromptUser(instructions,PROMPT_OKCANCEL) == RESPONSE_CANCEL)
		return false;

	// configure the FPGA with the oscillator interface
	if(Configure(string(brdInfo[brdIndex].port[PORTTYPE_LPT].oscBitstreamFile)) == false)
	{
		errMsg.SimpleMsg(XSErrorMajor,"Error downloading clock-set circuit!!\n");
		return false;
	}

	// program the oscillator
	osc.SetOscFrequency(div,extOscPresent);

	// give the user some instructions on how to reset the oscillator so the programming will take effect
	instructions = "The frequency of your XS40 Board has been set!!\n";
	instructions += "Now do these steps to activate the oscillator:\n";
	instructions += "1) Remove the power and downloading cable from your XS40 Board\n";
	instructions += "2) Move the shunt to the \"osc\" position of jumper J12\n";
	instructions += "3) Reconnect the power cable\n";
	instructions += "4) Reconnect the downloading cable\n";
	instructions += "5) Click on the OK button\n";
	PromptUser(instructions,PROMPT_OK);

	return true;
}


// Look at xsboard.h for a description of the interface.
bool XS40Board::SetupAudio(int *reg)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	string msg = brdModel + (string)" does not support audio codec programming!!\n";
	errMsg.SimpleMsg(XSErrorMajor,msg);
	return false;
}


// Look at xsboard.h for a description of the interface.
bool XS40Board::SetupVideoIn(string& fileName)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	string msg = brdModel + (string)" does not support video input programming!!\n";
	errMsg.SimpleMsg(XSErrorMajor,msg);
	return false;
}


// Look at xsboard.h for a description of the interface.
bool XS40Board::Test(void)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	// if there is no diagnostic bitstream file, then this board model does not support diagnostics
	string msg;
	if(strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].testBitstreamFile)==0)
	{
		msg = brdModel + (string)" does not support self-test!!\n";
		errMsg.SimpleMsg(XSErrorMajor,msg);
		return false;
	}

	// download the diagnostic program for the micro to the on-board RAM
	if(DownloadRAM(string(brdInfo[brdIndex].port[PORTTYPE_LPT].testObjFile),BIG_ENDIAN_BYTES,LITTLE_ENDIAN_BITS,true,false) == false)
	{
		errMsg.SimpleMsg(XSErrorMajor,"Error downloading XS40 Board self-test program!\n");
		return false;
	}

	fpga.Out(1,posTESTRESET,posTESTRESET);	// make sure micro is reset as soon as FPGA is configured

	// download the FPGA bitstream that supports the diagnostic program run by the micro
	if(Configure(string(brdInfo[brdIndex].port[PORTTYPE_LPT].testBitstreamFile)) == false)
	{
		errMsg.SimpleMsg(XSErrorMajor,"Error downloading XS40 Board self-test circuit!\n");
		return false;
	}

	// reset the micro and release the reset to start the diagnostic running
	fpga.Out(1,posTESTRESET,posTESTRESET);	// reset the microcontroller
	InsertDelay(10,MILLISECONDS);
	fpga.Out(0,posTESTRESET,posTESTRESET);	// release the microcontroller and do the test

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
	int prevStatus = fpga.In(posTESTSTATUS,posTESTSTATUS);
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
			currStatus = fpga.In(posTESTSTATUS,posTESTSTATUS);
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
			msg += " 7) Is the shunt on jumper J12 in the osc position?\n";
			msg += " 8) Is the shunt on jumper J7 in the ext position?\n";
			msg += " 9) Are the shunts on jumpers J4 and J11?\n";
			msg += "10) Are the shunts removed from jumpers J6 and J10?\n";
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
	
	delete progressGauge; // get rid of the progress indicator
	
	return false;
}


// Look at xsboard.h for a description of the interface.
unsigned char XS40Board::ApplyTestVectors(unsigned char singleVector, unsigned char mask, unsigned char *vector,
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
unsigned char XS40Board::GetTestVector(void)
{
	return testPort.GetTestVector();
}


// Look at xsboard.h for a description of the interface.
bool XS40Board::DownloadRAMFromIntArray(unsigned *intArray, unsigned address, unsigned numInts)			
{
	return false; // This method is not implemented.
}


// Look at xsboard.h for a description of the interface.
bool XS40Board::UploadRAMToIntArray(unsigned *intArray, unsigned address, unsigned numInts)			
{
	return false; // This method is not implemented.
}
