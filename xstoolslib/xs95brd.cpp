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
#include "xs95brd.h"

#include "../xstoolslib/guitools.h"

// inversion mask for parallel port connection to XS95 Board
// 00000011: bits  7- 0 are attached to data pins D7-D0
// 00000xxx: bits 15-11 are attached to status pins /S7,S6,S5,S4,S3
// xxxx1001: bits 19-16 are attached to control pins /C3,C2,/C1,/C0
// const unsigned int invMask = 0x090003;

// bit position of XC9572XL JTAG pins within parallel port registers
static const unsigned int posTCK  = 17;
static const unsigned int posTMS  = 18;
static const unsigned int posTDI  = 19;
static const unsigned int posTDO  = 15;

// bit positions of test vector input and output
static const unsigned int posTVCLK = 0;
static const unsigned int posTVOLO = 0;
static const unsigned int posTVOHI = 7;
static const unsigned int posTVILO = 11;
static const unsigned int posTVIHI = 14;

// bit position of prog. osc. pins within parallel port registers
static const unsigned int posOSC  = 0;

// bit positions for board test status
static const unsigned int posTESTSTATUS	= 14;
static const unsigned int posTESTRESET	= 5;


/// Create an XS95 Board object.
XS95Board::XS95Board(void)
{
	XSError* err = new XSError(cerr);	// create error-reporting channel
	brdModel = NULL;
	Setup(err,"XS95-108",1);	// use a default board model and parallel port number at this point
}


/// Destroy an XS95 Board object.
XS95Board::~XS95Board(void)
{
	;
}


// Look at xsboard.h for a description of the interface.
void XS95Board::SetFlags(unsigned long f)
{
	flags = f;
}


// Look at xsboard.h for a description of the interface.
unsigned long XS95Board::GetFlags(void)
{
	return flags;
}


// Look at xsboard.h for a description of the interface.
bool XS95Board::Setup(XSError* err, const char* model, unsigned int lptNum)
{
	// store some global information for the board
	brdErr = err;
	portNum = lptNum;

	// store name of board model
	if(brdModel!=NULL)
		free(brdModel);
	brdModel = (char*)malloc(strlen(model)+1);
	if(brdModel==NULL)
	{
		err->SimpleMsg(XSErrorFatal,"out of memory!!\n");
	}
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
	{
		err->SimpleMsg(XSErrorFatal,"Unknown type of XS95 Board!\n");
	}
	int invMask = brdInfo[brdIndex].port[PORTTYPE_LPT].invMask;

	// initialize the objects for the components on the board
	bool status;
	status = testPort.Setup(err,lptNum,invMask,posTVCLK,posTVOLO,posTVOHI,posTVILO,posTVIHI);
	status = status && cpld.Setup(err,lptNum,invMask,posTCK,posTMS,posTDI,posTDO);
	status = status && osc.Setup(err,lptNum,invMask,posOSC);
	status = status && ram.Setup(XC95108Type,err,lptNum,invMask,posTCK,posTMS,posTDI,posTDO);
	return status;	// return true if no error occurred for any object setup
}


// Look at xsboard.h for a description of the interface.
bool XS95Board::CheckChipID(void)
{
	// get the chip ID from the interface CPLD and compare it to the chip ID for
	// an XC95108 CPLD (ignoring the first 4 bits which increment for each chip revision)
	char* XC95108ID = "1001010100000110000010010011";

	string chipID = cpld.GetChipID();
    if (strncmp(XC95108ID,chipID.c_str()+4,strlen(XC95108ID)))
    {
        // ID did not match
		string instructions;
		instructions = "The CPLD of your XS95 Board is not responding!!\n\n";
		instructions += "\nChip ID = ";
		instructions += chipID;		// display the ID that was received
		instructions += "\n\n";
		instructions += "1) Is power getting to your XS95 Board?\n";
		instructions += "2) Is the downloading cable attached?\n";
		instructions += "\nContinue anyway?\n";
        if (GuiTools::PromptUser(instructions,PROMPT_OKCANCEL) == RESPONSE_CANCEL)
			return false;
	}
	return true;	// ID matched
}


// Look at xsboard.h for a description of the interface.
bool XS95Board::Configure(const char *fileName, bool *UserCancelled)
{
    bool status;

    status = false;
	XSError& errMsg = cpld.GetErr(); // setup error channel

	// check the type of the file that is going to be downloaded
	string suffix = GetSuffix(fileName);
    if (suffix != "SVF")
	{
		errMsg.SimpleMsg(XSErrorMajor,"Only .SVF files can be downloaded into the CPLD on the XS95 Board!!\n");
	}
    else
    {
        // abort if the CPLD chip ID doesn't match the ID for the CPLD on this type of board
        if (CheckChipID())
        {
            // initialize and then configure the CPLD with the contents of the bitstream file
            cpld.InitConfigureCPLD();
            status = cpld.ConfigureCPLD(fileName, UserCancelled);
        }
    }
    return status;
}


// Look at xsboard.h for a description of the interface.
bool XS95Board::ConfigureInterface(const char *fileName, bool *UserCancelled)
{
	return false;	// there is no interface CPLD on the XS40 Board, so this is an error
}


// Look at xsboard.h for a description of the interface.
bool XS95Board::DownloadRAM(const char *fileName, bool bigEndianBytes,
            bool bigEndianBits, bool doStart, bool doEnd, bool *UserCancelled)
{
	XSError& errMsg = cpld.GetErr(); // setup error channel

	// check the type of the file that is going to be downloaded
	string suffix = GetSuffix(fileName);
	if(suffix!="HEX" && suffix!="EXO" && suffix!="XES" && suffix!="MCS")
	{
		errMsg.SimpleMsg(XSErrorMajor,"Only .HEX, .MCS, .EXO or .XES files can be downloaded into the RAM on the XS95 Board!!\n");
		return false;
	}

	// abort if the CPLD chip ID doesn't match the ID for the CPLD on this type of board
	if(CheckChipID() == false)
		return false;

	// download data to the RAM
	if(ram.DownloadRAM(fileName,bigEndianBytes,bigEndianBits) == false)
	{
		errMsg.SimpleMsg(XSErrorMajor,"Error downloading into RAM!!\n");
		return false;
	}

	return true;
}


// Look at xsboard.h for a description of the interface.
bool XS95Board::UploadRAM(const char *fileName, const char* format,
				unsigned int loAddr, unsigned int hiAddr,
				bool bigEndianBytes, bool bigEndianBits,	
                bool doStart, bool doEnd, bool *UserCancelled)
{
	XSError& errMsg = cpld.GetErr(); // setup error channel

	// abort if the CPLD chip ID doesn't match the ID for the CPLD on this type of board
	if(CheckChipID() == false)
		return false;

	// upload data from the RAM
	if(ram.UploadRAM(fileName,format,loAddr,hiAddr,bigEndianBytes,bigEndianBits) == false)
	{
		errMsg.SimpleMsg(XSErrorMajor,"Error uploading from RAM!!\n");
		return false;
	}
	
	return true;
}


// Look at xsboard.h for a description of the interface.
bool XS95Board::ReadRAM(unsigned int addr, unsigned int* data,	
        bool bigEndianBytes, bool bigEndianBits, bool *UserCancelled)
{
	XSError& errMsg = cpld.GetErr(); // setup error channel

	// try reading the RAM
	return ram.ReadRAM(addr,data,bigEndianBytes,bigEndianBits);
}


// Look at xsboard.h for a description of the interface.
bool XS95Board::WriteRAM(unsigned int addr, unsigned int data,	
        bool bigEndianBytes, bool bigEndianBits, bool *UserCancelled)
{
	XSError& errMsg = cpld.GetErr(); // setup error channel

	// try writing to the RAM
	return ram.WriteRAM(addr,data,bigEndianBytes,bigEndianBits);
}


// Look at xsboard.h for a description of the interface.
bool XS95Board::DownloadFlash(const char *fileName, bool bigEndianBytes,
            bool bigEndianBits, bool doStart, bool doEnd, bool *UserCancelled)
{
	XSError& errMsg = cpld.GetErr(); // setup error channel
	errMsg.SimpleMsg(XSErrorMajor,"The XS95 Board does not have a Flash memory device!!\n");
	return false;
}


// Look at xsboard.h for a description of the interface.
bool XS95Board::UploadFlash(const char *fileName, const char* format,
				unsigned int loAddr, unsigned int hiAddr,
				bool bigEndianBytes, bool bigEndianBits,	
                bool doStart, bool doEnd, bool *UserCancelled)
{
	XSError& errMsg = cpld.GetErr(); // setup error channel
	errMsg.SimpleMsg(XSErrorMajor,"The XS95 Board does not have a Flash memory device!!\n");
	return false;
}


// Look at xsboard.h for a description of the interface.
bool XS95Board::SetFreq(int div, bool extOscPresent)
{
    bool status;

    status = false;
	XSError& errMsg = cpld.GetErr(); // setup error channel

	// get the name of the file that contains a bitstream that will configure the CPLD to provide an interface
	// between the parallel port and the programmable oscillator
    if (0 == strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].oscBitstreamFile))
	{
		string msg;
		msg = brdModel + (string)" does not support oscillator programming!!\n";
		errMsg.SimpleMsg(XSErrorMajor,msg);
	}
    else
    {
        bool UserCancelled;

        string instructions;

        // give the user some instructions on how to initialize the oscillator for programming
        instructions = "Before setting the XS95 Board frequency you must:\n";
        instructions += "1) Remove the power and downloading cable from your XS95 Board\n";
        instructions += "2) Place a shunt on the \"set\" position of jumper J6\n";
        instructions += "3) Reconnect the power cable\n";
        instructions += "4) Reconnect the downloading cable\n";
        instructions += "5) Click on the OK button\n";
        UserCancelled =(GuiTools::PromptUser(instructions,PROMPT_OKCANCEL) == RESPONSE_CANCEL);

        if (!UserCancelled)
        {
            // configure the CPLD with the oscillator interface
            status = cpld.ConfigureCPLD(brdInfo[brdIndex].port[PORTTYPE_LPT].oscBitstreamFile, &UserCancelled);
            if (errMsg.IsError())
            {
                errMsg.SimpleMsg(XSErrorMajor,"Error downloading clock-set circuit!!\n");
            }
            else
            {
                // program the oscillator
                osc.SetOscFrequency(div,extOscPresent);

                // erase the oscillator interface from the CPLD
                status = cpld.ConfigureCPLD(brdInfo[brdIndex].port[PORTTYPE_LPT].eraseBitstreamFile, &UserCancelled);
                if (errMsg.IsError())
                {
                    errMsg.SimpleMsg(XSErrorMajor,"Error erasing CPLD!!\n");
                }
            }

            // give the user some instructions on how to reset the oscillator so the programming will take effect
            instructions = "The frequency of your XS95 Board has been set!!\n";
            instructions += "Now do these steps to activate the oscillator:\n";
            instructions += "1) Remove the power and downloading cable from your XS95 Board\n";
            instructions += "2) Move the shunt to the \"osc\" position of jumper J6\n";
            instructions += "3) Reconnect the power cable\n";
            instructions += "4) Reconnect the downloading cable\n";
            instructions += "5) Click on the OK button\n";
            GuiTools::GuiTools::PromptUser(instructions,PROMPT_OK);
        }
    }
    return status;
}


// Look at xsboard.h for a description of the interface.
bool XS95Board::SetupAudio(int *reg)
{
	XSError& errMsg = cpld.GetErr(); // setup error channel

	string msg = brdModel + (string)" does not support audio codec programming!!\n";
	errMsg.SimpleMsg(XSErrorMajor,msg);
	return false;
}


// Look at xsboard.h for a description of the interface.
bool XS95Board::SetupVideoIn(string& fileName)
{
	XSError& errMsg = cpld.GetErr(); // setup error channel

	string msg = brdModel + (string)" does not support video input programming!!\n";
	errMsg.SimpleMsg(XSErrorMajor,msg);
	return false;
}


// Look at xsboard.h for a description of the interface.
bool XS95Board::Test(void)
{
    bool status;
	XSError& errMsg = cpld.GetErr(); // setup error channel

	// if there is no diagnostic bitstream file, then this board model does not support diagnostics
	string msg;
    if (0 == strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].testBitstreamFile))
	{
		msg = brdModel + (string)" does not support self-test!!\n";
		errMsg.SimpleMsg(XSErrorMajor,msg);
	}
    else
    {
        bool UserCancelled;

        cpld.Out(1,posTESTRESET,posTESTRESET);	// make sure micro is reset as soon as CPLD is configured

        // download the diagnostic program for the micro to the on-board RAM
        if (!DownloadRAM(brdInfo[brdIndex].port[PORTTYPE_LPT].testObjFile,BIG_ENDIAN_BYTES,LITTLE_ENDIAN_BITS,true,false, &UserCancelled))
        {
            errMsg.SimpleMsg(XSErrorMajor,"Error downloading XS95 Board self-test program!\n");
        }
        else
        {
            cpld.Out(1,posTESTRESET,posTESTRESET);	// make sure micro is reset as soon as CPLD is configured

            // download the CPLD bitstream that supports the diagnostic program run by the micro
            if (!Configure(brdInfo[brdIndex].port[PORTTYPE_LPT].testBitstreamFile, &UserCancelled))
            {
                errMsg.SimpleMsg(XSErrorMajor,"Error downloading XS95 Board self-test circuit!\n");
            }
            else
            {
                // reset the micro and release the reset to start the diagnostic running
                cpld.Out(1,posTESTRESET,posTESTRESET);	// make sure the microcontroller is reset
                InsertDelay(10,MILLISECONDS);
                cpld.Out(0,posTESTRESET,posTESTRESET);	// release the microcontroller reset and do the test

                // display a progress indicator while the diagnostic is running
                string desc((string)"Testing " + brdModel), subdesc("Testing...");

                Progress* progressGauge = new Progress(NULL, &errMsg);
                if (NULL != progressGauge)
                {
                    progressGauge->Setup(desc,subdesc,0,100);

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
                            {
                                currDiff++;
                            }
                            prevStatus = currStatus;
                        }

                        // update the progress indicator
                        testProgress = testProgress>=100 ? 0 : testProgress+10;
                        progressGauge->Report(testProgress);

                        if (currDiff == 0)
                        {
                            // Test is done and the board failed.  Give the user some help.
                            msg = (string)"\nYour " + brdModel + (string)" failed the test!\n\n";
                            msg += "Check the following:\n";
                            msg += "1) Is your board connected to the PC parallel port?\n";
                            msg += "2) Is your PC parallel port in unidirectional mode?\n";
                            msg += "3) Is your board the only device connected to the parallel port?\n";
                            msg += "4) Is your board getting power?\n";
                            msg += "5) Is your board sitting on a non-conductive surface?\n";
                            msg += "6) Is the programmable oscillator frequency greater than 1 MHz?\n";
                            msg += "7) Is the shunt on jumper J6 in the osc position?\n";
                            msg += "8) Is the shunt on jumper J7 in the ext position?\n";
                            errMsg.SimpleMsg(XSErrorMajor,msg);
                            break;
                        }
                        else
                        {
                            if ((currDiff > 2*prevDiff) && (prevDiff>0))
                            {
                                // test is done and the board passed
                                msg = (string)"\nYour " + brdModel + (string)" passed the test!\n";
                                GuiTools::PromptUser(msg,PROMPT_OK);
                                status = true;
                                break;
                            }
                        }

                        // the test is still running
                        prevDiff = currDiff;
                    }

                    if (NULL != progressGauge)
                    {
                        // get rid of the progress indicator
                        progressGauge->EndProgress();
                        delete progressGauge;
                        progressGauge = NULL;
                    }
                }
            }
        }
    }
    return status;
}


// Look at xsboard.h for a description of the interface.
unsigned char XS95Board::ApplyTestVectors(unsigned char singleVector, unsigned char mask, unsigned char *vector,
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
unsigned char XS95Board::GetTestVector(void)
{
	return testPort.GetTestVector();
}


// Look at xsboard.h for a description of the interface.
bool XS95Board::DownloadRAMFromIntArray(unsigned *intArray, unsigned address, unsigned numInts)			
{
	return false; // This method is not implemented.
}


// Look at xsboard.h for a description of the interface.
bool XS95Board::UploadRAMToIntArray(unsigned *intArray, unsigned address, unsigned numInts)			
{
	return false; // This method is not implemented.
}
