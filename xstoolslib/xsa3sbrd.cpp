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
#include "xsa3sbrd.h"

#include "../xstoolslib/guitools.h"


// bits  7- 0 are attached to data pins D7-D0
// bits 15-11 are attached to status pins S7,S6,S5,S4,S3
// bits 19-16 are attached to control pins C3,C2,C1,C0

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
static const unsigned int posRCLK	= 1;
static const unsigned int posRDOLSB	= 2;
static const unsigned int posRDOMSB	= 5;
static const unsigned int posRDILSB	= 11;
static const unsigned int posRDIMSB	= 14;
static const unsigned int posRSTLSB	= 11;
static const unsigned int posRSTMSB	= 14;

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
static const unsigned int posTESTSTATUS	= 14;

// USERCODE strings for various circuits programmed into the XSA-3S CPLD
static const char *oscIntfcCode			= "<0>!";
static const char *flashIntfcCode			= "<1>!";
static const char *flashConfigIntfcCode	= "<2>!";
static const char *testIntfcCode			= "<4>!";
static const char *dwnldIntfcCode			= "<4>!";


/// Create an XSA-3S Board object.
XSA3SBoard::XSA3SBoard(void)
{
	XSError* err = new XSError(cerr);	// create error-reporting channel
	brdModel = NULL;

    // Must call Setup() later
}


/// Destroy the XSA-3S Board object.
XSA3SBoard::~XSA3SBoard(void)
{
}


// Look at xsboard.h for a description of the interface.
void XSA3SBoard::SetFlags(unsigned long f)
{
	flags = f;
}


// Look at xsboard.h for a description of the interface.
unsigned long XSA3SBoard::GetFlags(void)
{
	return flags;
}


// Look at xsboard.h for a description of the interface.
bool XSA3SBoard::Setup(XSError* err, const char* model, unsigned int lptNum)
{
    bool status;

    status = false;

	// store some global information for the board
	brdErr = err;
	portNum = lptNum;

	// store name of board model
	if(brdModel!=NULL)
    {
		free(brdModel);
    }
	brdModel = (char*)malloc(strlen(model)+1);
    if (brdModel==NULL)
    {
		err->SimpleMsg(XSErrorFatal,"out of memory!!\n");
    }
    else
    {
        if (NULL != model)
        {
            strcpy(brdModel,model);

            // get information about all the models of XS boards
            int numBoards = GetXSBoardInfo(&brdInfo);

            // find parallel port inversion mask for this board model
            for (brdIndex = 0; brdIndex < numBoards; brdIndex++)
            {
                if (strcmp(brdModel,brdInfo[brdIndex].brdModel) == 0)
                {
                    break;
                }
            }
            if(brdIndex>=numBoards)
            {
//      		err->SimpleMsg(XSErrorFatal,"Unknown type of XSA-3S Board!\n");
            }
            else
            {
                int invMask = brdInfo[brdIndex].port[PORTTYPE_LPT].invMask;

                // initialize the objects for the components on the board
                status = testPort.Setup(err,lptNum,invMask,posTVCLK,posTVOLO,posTVOHI,posTVILO,posTVIHI);
                status = status && fpga.Setup(err,lptNum,invMask,posCCLK,posPROG,posDLO,posDHI,posDONE);
                status = status && cpld.Setup(err,lptNum,invMask,posTCK,posTMS,posTDI,posTDO);
                status = status && osc.Setup(err,lptNum,invMask,posOSC);
                status = status && ram.Setup(err,lptNum,invMask,posRRESET,posRCLK,posRDOLSB,posRDOMSB,posRDILSB,posRDIMSB,posRSTLSB,posRSTMSB,16,24);
                status = status && flash.Setup(err,lptNum,invMask,posFRESET,posFCLK,posFDOLSB,posFDOMSB,posFDILSB,posFDIMSB,posFSTLSB,posFSTMSB);
            }
        }
    }
	return status;	// return true if no error occurred for any object setup
}


// Look at xsboard.h for a description of the interface.
bool XSA3SBoard::CheckChipID(string& chipID)
{
	// get the chip ID from the interface cpld and compare it to the chip ID for
	// an XC9572XL CPLD (ignoring the first 4 bits which increment for each chip revision)
	char* XC9572XLID = "1001011000000100000010010011";

    chipID = cpld.GetChipID();

    // ID match
    return (0 == strncmp(XC9572XLID, chipID.c_str()+4, strlen(XC9572XLID)));
}


// Look at xsboard.h for a description of the interface.
bool XSA3SBoard::Configure(const char *fileName, bool *UserCancelled)
{
    bool status;

    status = false;
	XSError& errMsg = fpga.GetErr(); // setup error channel

	// Check the suffix of the bitstream file.  .BIT files go to the FPGA; .SVF files go to the interface CPLD.
	string suffix = GetSuffix(fileName);
    if (suffix=="BIT")
    {
        // configure the FPGA with a bitstream
		// look in the bitstream file to find the type of FPGA it is intended for and make sure that matches the type of FPGA on the board
        string type = ConvertToUpperCase(fpga.GetChipType(fileName));
        if (type == "")
		{
			errMsg.SimpleMsg(XSErrorMajor,"The .BIT file does not identify the target FPGA!!\n");
		}
        else
        {
            if (type != brdInfo[brdIndex].chipType)
            {
                string msg = (string)".BIT file is for the " + type + (string)" but this is not the FPGA on the " + brdModel + (string)"!!\n";
                errMsg.SimpleMsg(XSErrorMajor,msg);
            }
            else
            {
                bool ReadyToSend;

                ReadyToSend = true;
                // get the USERCODE from the interface CPLD to see if it supports downloading a bitstream to the FPGA
                string usercode = cpld.GetUSERCODE();
                if((usercode==oscIntfcCode) || (usercode==flashIntfcCode) || (usercode==flashConfigIntfcCode))
                {
                    // the interface CPLD is not configured with the standard downloading circuit
                    if (!ConfigureInterface(brdInfo[brdIndex].port[PORTTYPE_LPT].dwnldIntfcBitstreamFile, UserCancelled))
                    {
                        errMsg.SimpleMsg(XSErrorMajor,"Error configuring CPLD with downloading interface circuit!\n");
                        ReadyToSend = false;
                    }
                    else
                    {
                        fpga.EnableFastDownload(true);
                    }
                }
                else
                {
                    if ((usercode==dwnldIntfcCode) || (usercode==testIntfcCode))
                    {
                        // the CPLD contains the fast downloading interface
                        fpga.EnableFastDownload(true);
                    }
                    else
                    {
                        // can't tell what the CPLD contains, so assume the user loaded it with a legacy, low-speed interface
                        fpga.EnableFastDownload(false);
                    }
                }
                if (ReadyToSend)
                {
                    // initialize and then configure the FPGA with the contents of the bitstream file
                    fpga.InitConfigureFPGA();
                    status = fpga.ConfigureFPGA(fileName, UserCancelled);
                }
            }
        }
    }
    else
    {
        if (suffix == "SVF")
        {
            // configure the CPLD with an SVF bitstream
            status = ConfigureInterface(fileName, UserCancelled);
        }
        else
        {
            // neither a .BIT or .SVF file, so report the error
            errMsg.SimpleMsg(XSErrorMajor,"Only .BIT or .SVF files can be downloaded into the FPGA or CPLD on the XSA-3S Board!!");
        }
	}
    return status;
}


// Look at xsboard.h for a description of the interface.
bool XSA3SBoard::ConfigureInterface(const char *fileName, bool *UserCancelled)
{
    bool status;

    status  = false;
	XSError& errMsg = fpga.GetErr(); // setup error channel

	// check the type of the file that is going to be downloaded to the interface CPLD
	string suffix = GetSuffix(fileName);
    if (suffix == "SVF")
	{
		// initialize and then configure the CPLD with the contents of the bitstream file
		cpld.InitConfigureCPLD();
        status = cpld.ConfigureCPLD(fileName, UserCancelled);
	}
    else
    {
        if (suffix == "BIT")
        {
            // an FPGA bitstream file was passed, so go to another method that is used to configure the FPGA
            status = Configure(fileName, UserCancelled);
        }
        else
        {
            errMsg.SimpleMsg(XSErrorMajor,"Only .BIT or .SVF files can be downloaded into the FPGA or CPLD on the XSA-3S Board!!");
        }
    }
    return status;
}


// Look at xsboard.h for a description of the interface.
bool XSA3SBoard::DownloadRAM(const char *fileName, bool bigEndianBytes,
            bool bigEndianBits, bool doStart, bool doEnd, bool *UserCancelled)
{
    bool status;

    status = false;
	XSError& errMsg = fpga.GetErr(); // setup error channel

	// check the file suffix to see if it is appropriate for downloading to RAM
	string suffix = GetSuffix(fileName);
    if (suffix!="HEX" && suffix!="EXO" && suffix!="XES" && suffix!="MCS")
	{
		errMsg.SimpleMsg(XSErrorMajor,"Only .HEX, .MCS, .EXO or .XES files can be downloaded into the RAM on the XSA-3S Board!!\n");
	}
    else
    {
        // get the name of the file that contains a bitstream that will configure the FPGA to provide an interface
        // between the parallel port and the RAM.
        if (strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile)==0)
        {
            // it is an error if there is no RAM download interface configuration for this board
            string msg = brdModel + (string)" does not support RAM downloads!!\n";
            errMsg.SimpleMsg(XSErrorMajor,msg);
        }
        else
        {
            // only download the RAM interface if this is the first download of data to the RAM.
            // otherwise the interface should already be in place.
            if (doStart)
            {
                if (!Configure(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile, UserCancelled))
                {
                    errMsg.SimpleMsg(XSErrorMajor,"Error downloading RAM interface circuit!!\n");
                }
                else
                {
                    // download data to the RAM
                    status = ram.DownloadRAM(fileName,bigEndianBytes,bigEndianBits);
                    if (!status)
                    {
                        errMsg.SimpleMsg(XSErrorMajor,"Error downloading into RAM!!\n");
                    }
                }
            }
        }
    }
    return status;
}


// Look at xsboard.h for a description of the interface.
bool XSA3SBoard::UploadRAM(const char *fileName, const char* format,
				unsigned int loAddr, unsigned int hiAddr,
				bool bigEndianBytes, bool bigEndianBits,	
                bool doStart, bool doEnd, bool *UserCancelled)
{
    bool status;

    // setup error channel
    XSError& errMsg = fpga.GetErr();

    status = false;

	// get the name of the file that contains a bitstream that will configure the FPGA to provide an interface
	// between the parallel port and the RAM.
    if (0 == strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile))
	{
		// it is an error if there is no RAM download interface configuration for this board
		string msg = brdModel + (string)" does not support RAM uploads!!\n";
		errMsg.SimpleMsg(XSErrorMajor,msg);
	}
    else
    {
        // Assume we are good to go
        status = true;

        // only download the RAM interface if this is the first download of data to the RAM.
        // otherwise the interface should already be in place.
        if (doStart)
        {
            if (!Configure(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile, UserCancelled))
            {
                errMsg.SimpleMsg(XSErrorMajor,"Error downloading RAM interface circuit!!\n");
                status = false;
            }
        }

        if (status)
        {
            // upload data from the RAM
            status = ram.UploadRAM(fileName,format,loAddr,hiAddr,bigEndianBytes,bigEndianBits);
            if (!status)
            {
                errMsg.SimpleMsg(XSErrorMajor,"Error uploading from RAM!!\n");
            }
        }
    }
    return status;
}


// Look at xsboard.h for a description of the interface.
bool XSA3SBoard::ReadRAM(unsigned int addr, unsigned int* data,	
        bool bigEndianBytes, bool bigEndianBits, bool *UserCancelled)
{
    string chipID;
	XSError& errMsg = fpga.GetErr(); // setup error channel

    if (!ram.ReadRAM(addr,data,bigEndianBytes,bigEndianBits))
    {
        // couldn't read RAM so maybe RAM interface is not loaded???
		
		// get the name of the file that contains a bitstream that will configure the FPGA to provide an interface
		// between the parallel port and the RAM
		if(strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile)==0)
		{
			string msg = brdModel + (string)" does not support RAM uploads!!\n";
			errMsg.SimpleMsg(XSErrorMajor,msg);
			return false;
		}
		
		// check the ID of the interface CPLD to see if it matches the type on this particular board.
        if (!CheckChipID(chipID))
        {
            return false;
        }
		// configure the FPGA with the RAM interface
        if (!Configure(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile, UserCancelled))
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
bool XSA3SBoard::WriteRAM(unsigned int addr, unsigned int data,	
        bool bigEndianBytes, bool bigEndianBits, bool *UserCancelled)
{
    string chipID;
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
        if (!CheckChipID(chipID))
        {
            return false;
        }
		// configure the FPGA with the RAM interface
        if (!Configure(brdInfo[brdIndex].port[PORTTYPE_LPT].ramBitstreamFile, UserCancelled))
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
bool XSA3SBoard::DownloadFlash(const char *fileName, bool bigEndianBytes,
            bool bigEndianBits, bool doStart, bool doEnd, bool *UserCancelled)
{
    bool status;

    status = false;
    string chipID;
    XSError& errMsg = fpga.GetErr(); // setup error channel

	// check the file suffix to see if it is appropriate for downloading to RAM
	string suffix = GetSuffix(fileName);
    if (suffix!="HEX" && suffix!="EXO" && suffix!="XES" && suffix!="MCS")
	{
        errMsg.SimpleMsg(XSErrorMajor,"Only .HEX, .MCS, .EXO or .HEX files can be downloaded into the Flash on the XSA-3S Board!!\n");
	}
    else
    {
        // get the name of the file that contains a bitstream that will configure the FPGA to provide an interface
        // between the parallel port and the Flash.
        if (0 == strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].flashBitstreamFile))
        {
            string msg = brdModel + (string)" does not support Flash programming!!\n";
            errMsg.SimpleMsg(XSErrorMajor,msg);
        }
        else
        {
            // check the ID of the interface CPLD to see if it matches the type on this particular board.
            if (CheckChipID(chipID))
            {
                bool Ready;

                Ready = true;

                // get the USERCODE from the interface CPLD to see if it supports downloading to the Flash
                if (cpld.GetUSERCODE() != dwnldIntfcCode)
                {
                    // the interface CPLD is not configured with the standard parallel port interface, so download it to the CPLD
                    if (!ConfigureInterface(brdInfo[brdIndex].port[PORTTYPE_LPT].dwnldIntfcBitstreamFile, UserCancelled))
                    {
                        errMsg.SimpleMsg(XSErrorMajor,"Error configuring CPLD with downloading interface!\n");
                        Ready = false;
                    }
                }
                if (Ready)
                {
                    // If this is the first data file being downloaded to the Flash, then configure the FPGA with the Flash interface.
                    // (Yes, the FPGA handles downloading to the Flash on this board.)
                    if (doStart)
                    {
                        if (!ConfigureInterface(brdInfo[brdIndex].port[PORTTYPE_LPT].flashBitstreamFile, UserCancelled))
                        {
                            errMsg.SimpleMsg(XSErrorMajor,"Error configuring FPGA with Flash programming circuit!\n");
                            Ready = false;
                        }
                    }

                    if (Ready)
                    {
                        // Set all the parallel port data pins high before starting the Flash programming because
                        // the CPLD sends them on to the FPGA and Flash and D6 shares the Flash WE# pin.  If D6 is low,
                        // then this causes problems with unintentional writes to Flash.
                        flash.Out(0xFF,0,7);

                        // download data into the Flash
                        status = !flash.DownloadFlash(fileName,bigEndianBytes,bigEndianBits,doStart);
                        if (!status)
                        {
                            errMsg.SimpleMsg(XSErrorMajor,"Error programming the Flash on the XSA-3S Board!\n");
                        }
                        else
                        {
                            // if this is the last file to be downloaded to Flash, then reprogram the CPLD with a circuit
                            // that will make the CPLD load the FPGA with the contents of the Flash upon power-up.
                            if (doEnd)
                            {
                                status = ConfigureInterface(brdInfo[brdIndex].port[PORTTYPE_LPT].flashConfigBitstreamFile, UserCancelled);
                                if (!status)
                                {
                                    errMsg.SimpleMsg(XSErrorMajor,"Error configuring CPLD with Flash configuration circuit!\n");
                                }
                            }
                        }
                    }
                }
            }
        }
	}
    return status;
}


// Look at xsboard.h for a description of the interface.
bool XSA3SBoard::UploadFlash(const char *fileName, const char* format,
				unsigned int loAddr, unsigned int hiAddr,
				bool bigEndianBytes, bool bigEndianBits,	
                bool doStart, bool doEnd, bool *UserCancelled)
{
    bool status;
    string chipID;
    XSError& errMsg = fpga.GetErr(); // setup error channel

    status = false;

	// get the name of the file that contains a bitstream that will configure the FPGA to provide an interface
	// between the parallel port and the Flash.
    if (0 == strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].flashBitstreamFile))
	{
		string msg = brdModel + (string)" does not support Flash uploads!!\n";
		errMsg.SimpleMsg(XSErrorMajor,msg);
	}
    else
    {
        // check the ID of the interface CPLD to see if it matches the type on this particular board.
        if (CheckChipID(chipID))
        {
            // Assume good to go
            status = true;

            // get the USERCODE from the interface CPLD to see if it supports uploading from the Flash
            if (cpld.GetUSERCODE() != dwnldIntfcCode)
            {
                // the interface CPLD is not configured with the standard parallel port interface, so download it to the CPLD
                if (!ConfigureInterface(brdInfo[brdIndex].port[PORTTYPE_LPT].dwnldIntfcBitstreamFile, UserCancelled))
                {
                    errMsg.SimpleMsg(XSErrorMajor,"Error configuring CPLD with downloading interface!\n");
                    status = false;
                }
            }
	
            if (status)
            {
                // load the FPGA with the Flash interface file
                // (Yes, the FPGA handles uploading from the Flash on this board.)
                if (doStart)
                {
                    if (!ConfigureInterface(brdInfo[brdIndex].port[PORTTYPE_LPT].flashBitstreamFile, UserCancelled))
                    {
                        errMsg.SimpleMsg(XSErrorMajor,"Error configuring FPGA with Flash programming circuit!\n");
                        status = false;
                    }
                }
            }
	
            if (status)
            {
                // Set all the parallel port data pins high before starting the Flash programming because
                // the CPLD sends them on to the FPGA and Flash and D6 shares the Flash WE# pin.  If D6 is low,
                // then this causes problems with unintentional writes to Flash.
                flash.Out(0xFF,0,7);

                // upload data into the Flash
                if (!flash.UploadFlash(fileName, format, loAddr, hiAddr, bigEndianBytes, bigEndianBits))
                {
                    errMsg.SimpleMsg(XSErrorMajor,"Error uploading from Flash!!\n");
                    status = false;
                }
                else
                {
                    // you can only upload a single file from Flash, so reprogram the CPLD with a circuit
                    // that will make the CPLD load the FPGA with the contents of the Flash upon power-up.
                    // (We are assuming this was the circuit programmed into the CPLD before uploading from the Flash.)
                    status = ConfigureInterface(brdInfo[brdIndex].port[PORTTYPE_LPT].flashConfigBitstreamFile, UserCancelled);
                    if (!status)
                    {
                        errMsg.SimpleMsg(XSErrorMajor,"Error configuring CPLD with Flash configuration circuit!\n");
                    }
                }
            }
        }
    }
    return status;
}


// Look at xsboard.h for a description of the interface.
bool XSA3SBoard::SetFreq(int div, bool extOscPresent)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	string msg = brdModel + (string)" does not have a programmable oscillator!!\n";
	msg += "You will need to reprogram the CPLD to divide the master clock or divide the clock in your FPGA design.";
	errMsg.SimpleMsg(XSErrorMajor,msg);
	return false;
}


// Look at xsboard.h for a description of the interface.
bool XSA3SBoard::SetupAudio(int *reg)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	string msg = brdModel + (string)" does not support audio codec programming!!\n";
	errMsg.SimpleMsg(XSErrorMajor,msg);
	return false;
}


// Look at xsboard.h for a description of the interface.
bool XSA3SBoard::SetupVideoIn(string& fileName)
{
	XSError& errMsg = fpga.GetErr(); // setup error channel

	string msg = brdModel + (string)" does not support video input programming!!\n";
	errMsg.SimpleMsg(XSErrorMajor,msg);
	return false;
}

bool XSA3SBoard::PerformTest(XSError& errMsg)
{
    bool status;
    string msg;

    status = false;

    // display a progress indicator while the diagnostic is running
    string desc((string)"Testing " + brdModel), subdesc("Testing...");

    Progress* progressGauge = new Progress(NULL, &errMsg);
    if (NULL != progressGauge)
    {
        int testProgress;

        progressGauge->Setup(desc, subdesc, 0, 100);

        // Monitor the test status pin to see if the test is still running or not.
        // While the diagnostic runs, the test status pin pulses at some rate with 50% duty-cycle.
        // As soon as the duty cycle deviates from 50% over the intergration period, one of the following actions is taken:
        //    1. If the duty cycle is less than 33% then the test is done and the board has failed.
        //    2. If the duty cycle is greater than 66% then the test is done and the board has failed.
        //    3. If no pulses are seen during the integration period, then something has happened to the board or interface.
        //    4. Otherwise, the test is still running.
        const double integrationPeriod = 0.1; // seconds

        testProgress = 0;
        while (true)
        {
            // count the amount of time the status pin is high or low during the integration period
            clock_t startTime = clock();
            clock_t endTime = startTime + integrationPeriod * CLOCKS_PER_SEC;
            unsigned int hiCnt = 0;
            unsigned int loCnt = 0;

            // Note the progress bar stops increasing when it hits its high limit value.
            progressGauge->Report(testProgress++);

            while (clock() < endTime)
            {
                if (cpld.In(posTESTSTATUS,posTESTSTATUS) == 1)
                {
                    hiCnt++;
                }
                else
                {
                    loCnt++;
                }
            }

            if ((hiCnt==0) || (loCnt==0))
            {
                char Msg[80];

                std::snprintf(Msg, sizeof(Msg), "(Test Status Pin Pulse Counts Hi:%u Lo:%u)\n\n", hiCnt, loCnt);

                // no pulses from test circuit, so something is wrong with the board clock
                msg = (string)"\nYour " + brdModel + (string)" failed the test!\n";
                msg += "The clock oscillator is not working!\n";
                msg += Msg;
                msg += "Check the following:\n";
                msg += "1) Is your board connected to the PC parallel port?\n";
                msg += "2) Is your PC parallel port in unidirectional mode?\n";
                msg += "3) Is your board the only device connected to the parallel port?\n";
                msg += "4) Is your board getting power?\n";
                msg += "5) Is your board sitting on a non-conductive surface?\n";
                msg += "6) Is the shunt on the \'xs\' position of jumper J9?\n";
                msg += "7) Is the shunt on jumper J2?\n";
                errMsg.SimpleMsg(XSErrorMajor,msg);
                status = false;
                break;
            }

            if (loCnt/2 > hiCnt)
            {
                // duty cycle is less than 1/3 so the board test failed
                msg = (string)"\nYour " + brdModel + (string)" failed the test!\n\n";
                msg += "Check the following:\n";
                msg += "1) Is your board connected to the PC parallel port?\n";
                msg += "2) Is your PC parallel port in unidirectional mode?\n";
                msg += "3) Is your board the only device connected to the parallel port?\n";
                msg += "4) Is your board getting power?\n";
                msg += "5) Is your board sitting on a non-conductive surface?\n";
                msg += "6) Is the shunt on the \'xs\' position of jumper J9?\n";
                msg += "7) Is the shunt on jumper J2?\n";
                errMsg.SimpleMsg(XSErrorMajor,msg);
                status = false;
                break;
            }

            if (hiCnt/2 > loCnt)
            {
                // duty cycle is greater than 2/3 so the board test passed
                msg = (string)"\nYour " + brdModel + (string)" passed the test!\n";
                GuiTools::PromptUser(msg,PROMPT_OK);
                status = true;
                break;
            }
            // otherwise, the test is still in-progress if the duty-cycle is in the range [1/2, 2/3]
        }

        if (NULL != progressGauge)
        {
            progressGauge->EndProgress();
        }

    }

    return status;
}

bool XSA3SBoard::DumpUserCode(const char *Prompt, string usercode)
{
    char Msg[200];
    char HexValue[8];
    char HexStr[50];
    char FmtStr[200];
    int i;
    string msg;

    strcpy(HexStr, "");
    std::snprintf(HexValue, sizeof(HexValue), "0x%02X", usercode[0] & 0xFF);
    if (1 < usercode.length())
    {
        for (i = 1; i < usercode.length(); i++)
        {
            strcat(HexStr, HexValue);
            strcat(HexStr, ", ");
            std::snprintf(HexValue, sizeof(HexValue), "0x%02X", usercode[i] & 0xFF);
        }
    }
    strcat(HexStr, HexValue);

    // can't tell what the CPLD contains, so load standard testing interface
    strcpy(FmtStr, Prompt);
    strcat(FmtStr, " '%s' (%s)\nContinue anyway?\n");
    std::snprintf(Msg, sizeof(Msg), FmtStr, usercode.c_str(), HexStr);
    msg = Msg;
    return (GuiTools::PromptUser(msg, PROMPT_OKCANCEL) == RESPONSE_CANCEL);
}


// Look at xsboard.h for a description of the interface.
bool XSA3SBoard::Test(void)
{
    bool status;
    bool UserCancelled;
    string msg;

    status = false;

	XSError& errMsg = fpga.GetErr(); // setup error channel

	// if there is no diagnostic bitstream file, then this board model does not support diagnostics
    if (strlen(brdInfo[brdIndex].port[PORTTYPE_LPT].testIntfcBitstreamFile) == 0)
	{
        msg = brdModel + (string)" does not support self-test!!\n";
        errMsg.SimpleMsg(XSErrorMajor,msg);
	}
    else
    {
        bool Ready;
        string chipID;

        // Assume we are good to go
        Ready = true;

        // check the ID of the interface CPLD to see if it matches the type on this particular board.
        if (!CheckChipID(chipID))
        {
            string instructions;

            instructions = "The interface CPLD of your XSA-3S Board is not responding!!\n\n";
            instructions += "\nChip ID = ";
            instructions += chipID;		// display the ID that was received
            instructions += "\n\n";
            instructions += "1) Is power getting to your XSA-3S Board?\n";
            instructions += "2) Is the downloading cable attached?\n";
            instructions += "\nContinue anyway?\n";

            if (GuiTools::PromptUser(instructions, PROMPT_OKCANCEL) == RESPONSE_CANCEL)
            {
                Ready = false;
            }
        }

        if (Ready)
        {
            // get the USERCODE from the interface CPLD to see if it supports running diagnostics
            bool loadTestIntfc;

            // Assume not ready for testing yet
            Ready = false;

            string usercode = cpld.GetUSERCODE();

            if (0 == usercode.compare(testIntfcCode))
            {
                // the CPLD already contains the standard testing interface bitstream
                errMsg.SimpleMsg(XSErrorMinor,"CPLD already contains the standard testing interface bitstream\n");
                loadTestIntfc = false;
                Ready = true;
            }
            else
            {
                if ((0 == usercode.compare(oscIntfcCode)) ||
                    (0 == usercode.compare(flashIntfcCode)) ||
                    (0 == usercode.compare(flashConfigIntfcCode)) ||
                    (0 == usercode.compare(dwnldIntfcCode)))
                {
                    // the interface CPLD is not configured with the standard testing interface...
                    loadTestIntfc = true; // so force it to be loaded

                    UserCancelled = DumpUserCode("CPLD is not configured with the standard testing interface ", usercode);
                }
                else
                {
                    loadTestIntfc = true;

                    // can't tell what the CPLD contains, so load standard testing interface
                    UserCancelled = DumpUserCode("CPLD interface unknown ", usercode);
                }
            }
            if (!UserCancelled)
            {
                if (loadTestIntfc)
                {
                    // load the CPLD with the diagnostic interface
                    if (!ConfigureInterface(brdInfo[brdIndex].port[PORTTYPE_LPT].testIntfcBitstreamFile, &UserCancelled))
                    {
                        errMsg.SimpleMsg(XSErrorMajor,"Error configuring CPLD with self-test interface circuit!\n");
                    }
                    else
                    {
                        Ready = !UserCancelled;
                    }
                }
            }

            if (Ready)
            {
                // configure the FPGA with the diagnostic circuit
                if (!Configure(brdInfo[brdIndex].port[PORTTYPE_LPT].testBitstreamFile, &UserCancelled))
                {
                    Ready = false;
                    if (!UserCancelled)
                    {
                        errMsg.SimpleMsg(XSErrorMajor,"Error downloading XSA-3S Board self-test circuit!\n");
                    }
                }
            }

            if (Ready && !UserCancelled)
            {
                status = PerformTest(errMsg);
            }
        }
    }
    return status;
}


// Look at xsboard.h for a description of the interface.
unsigned char XSA3SBoard::ApplyTestVectors(unsigned char singleVector, unsigned char mask, unsigned char *vector,
											unsigned char *response, unsigned int numVectors)
{
	assert(numVectors!=0);
	assert((numVectors>1 && vector!=NULL) || (numVectors==1 && vector==NULL));
	assert((numVectors>1 && response!=NULL) || (numVectors==1 && response==NULL));
	
    if (numVectors == 1)
	{
		return testPort.ApplyTestVector(singleVector,mask);
	}
	else
	{
		int i;
        for (i = 0; i < numVectors; i++)
		{
			response[i] = testPort.ApplyTestVector(vector[i],mask);		
		}
		return 0;
	}
}


// Look at xsboard.h for a description of the interface.
unsigned char XSA3SBoard::GetTestVector(void)
{
	return testPort.GetTestVector();
}


// Look at xsboard.h for a description of the interface.
bool XSA3SBoard::DownloadRAMFromIntArray(unsigned *intArray, unsigned address, unsigned numInts)			
{
	return false; // This method is not implemented.
}


// Look at xsboard.h for a description of the interface.
bool XSA3SBoard::UploadRAMToIntArray(unsigned *intArray, unsigned address, unsigned numInts)			
{
	return false; // This method is not implemented.
}
