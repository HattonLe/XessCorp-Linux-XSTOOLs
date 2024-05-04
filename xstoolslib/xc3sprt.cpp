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


//#include <dos.h>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <ctime>
#include <string>

#include <string.h>

using namespace std;

#ifdef WIN32
#include <afxwin.h>
#endif

#include "utils.h"
#include "xc3sprt.h"


/// Instantiate an XC3SPort object on a given parallel port.
XC3SPort::XC3SPort(void)
{
	progressGauge = NULL;
}


/// Instantiate an XC3SPort object on a given parallel port.
XC3SPort::XC3SPort(XSError* e, ///< pointer to error reporting object
				unsigned int portNum, ///< parallel port number
				unsigned int invMask, ///< inversion mask for the parallel port
				unsigned int posCCLK, ///< bit position in parallel port of CCLK pin
				unsigned int posPROG, ///< bit position in parallel port of PROGRAM pin
				unsigned int posDlo, ///< lower bit position in parallel port of config. data pins
				unsigned int posDhi, ///< upper bit position in parallel port of config. data pins
				unsigned int posDONE) ///< bit position in parallel port of DONE pin
{
	progressGauge = NULL;
	Setup(e,portNum,invMask,posCCLK,posPROG,posDlo,posDhi,posDONE);
}


/// Setup an XC3SPort object on a given parallel port.
///\return true if the operation was a success, false otherwise
bool XC3SPort::Setup(XSError* e, ///< pointer to error reporting object
				unsigned int portNum, ///< parallel port number
				unsigned int invMask, ///< inversion mask for the parallel port
				unsigned int posCCLK, ///< bit position in parallel port of CCLK pin
				unsigned int posPROG, ///< bit position in parallel port of PROGRAM pin
				unsigned int posDlo, ///< lower bit position in parallel port of config. data pins
				unsigned int posDhi, ///< upper bit position in parallel port of config. data pins
				unsigned int posDONE) ///< bit position in parallel port of DONE pin
{
	posDLO = posDlo;
	posDHI = posDhi;
	return CnfgPort::Setup(e,portNum,invMask,posCCLK,posPROG,posDlo,posDONE);
}


static const int DeviceFieldType = 0x62;		// field type for the FPGA device identifier

/// Get the chip identifier from the FPGA bitstream.
///\return the chip identifier
string XC3SPort::GetChipType(istream& is) ///< stream that delivers the bitstream
{
	XSError& err = GetErr();
	
	if(is.eof()!=0)
        return ""; // exit if no BIT stream exists
	
	// jump over the first field of the BIT file
	long unsigned int fieldLength = GetInteger(is);
	assert(fieldLength!=0);
	is.ignore(fieldLength);
	
	// process the second field
	fieldLength = GetInteger(is);
	assert(fieldLength==1);
	
	// now look for the field with the chip identifier
	bool status = ScanForField(is,DeviceFieldType);
	assert(status==true);
	fieldLength = GetInteger(is);
	assert(fieldLength>0 && fieldLength<100);
	char chipType[100];
	unsigned int i;
	for(i=0; i<fieldLength; i++)
		is >> chipType[i];
	chipType[i] = 0;		// terminate string
	return chipType;		// return the chip type
}


/// Get the chip identifier from the FPGA bitstream.
///\return the chip identifier
string XC3SPort::GetChipType(const char *bitfileName)	///< file containing the FPGA bitstream
{
	XSError& err = GetErr();
	
    if(strcmp(bitfileName, "")==0)
        return ""; // exit if no BIT file was given
	
    ifstream is(bitfileName,ios::binary);
	if(!is || is.fail()!=0)
	{ // error if given file could not be opened
		err.SetSeverity(XSErrorMajor);
        err << "could not open " << bitfileName << "\n";
		err.EndMsg();
        return "";
	}
	
	// now get the chip identifier from the bitstream file
	string chipType = GetChipType(is);
	
	is.close();
	
	return chipType;
}


/// Initialize FPGA configuration pins.
void XC3SPort::InitConfigureFPGA(void)
{
	SetPROG(1);
	SetCCLK(0);
}


/// Enable or disable fast bitstream downloads.
void XC3SPort::EnableFastDownload(bool t)	///< if true, enable fast parallel download, otherwise do serial download
{
	fastDownload = t;
}


/// Send out a byte of configuration information.
void XC3SPort::ConfigureFPGA(unsigned char b)	///< configuration byte from FPGA bitstream
{
	if(fastDownload)
    {
        // fast parallel configuration
		// reverse the bits of the configuration byte
		unsigned char rev_b;
		for(int i=0; i<8; i++)
			rev_b = (rev_b<<1) | ((b>>i) & 0x1);
		
		// now send it to the Spartan3 as two nybble-wide chunks
		Out((rev_b>>4)&0xF,posDLO,posDHI);	// send upper nybble
		SetCCLK(1);						// latches upper nybble
		Out(rev_b&0xF,posDLO,posDHI);	// now send lower nybble;
		SetCCLK(0);						// entire byte is now loaded into the FPGA
	}
	else
    {
        // slow serial configuration
		// configuration bytes for the Spartan3 programming pins only contain
		// values for the DIN pin, so eight clock pulses are needed per byte.
		for(int i=0; i<8; i++)
		{
			SetDIN(b&0x80 ? 1:0);  // byte is sent MSB first
			b <<= 1;
			PulseCCLK();
		}
	}
}


static const int BitstreamFieldType = 0x65;	// field type for the bitstream data

/// Process a stream and send the configuration bitstream to the board.
///\return true if the operation was a success, false otherwise
bool XC3SPort::ConfigureFPGA(istream& is, bool *UserCancelled)	///< stream that delivers the FPGA bitstream
{
    bool status;

    status = false;
	XSError& err = GetErr();
	
    // if BIT stream exists
    if (0 == is.eof())
    {
        // jump over the first field of the BIT file
        long unsigned int fieldLength = GetInteger(is);
        assert(fieldLength != 0);
        is.ignore(fieldLength);
	
        // process the second field
        fieldLength = GetInteger(is);
        if (1 == fieldLength)
        {
	        // now look for the field with the bitstream data
            if (ScanForField(is, BitstreamFieldType))
            {
                long unsigned int i;

                // show the progress indicator
                if (NULL != progressGauge)
                {
                    progressGauge->Report(0);
                    *UserCancelled = progressGauge->UserHasCancelled();
                }

                // now we are at the start of the configuration bits
                fieldLength = GetInteger(is, 4); // get the length of the bit stream
                assert(fieldLength>0);
                DEBUG_STMT("bit stream field length = " << fieldLength)

                // start the configuration process
                PulsePROG();					// pulse the PROGRAM pin to start the configuration of the FPGA
                InsertDelay(30,MILLISECONDS);	// insert delay after PROGRAM pin goes high

                i = 0;
                while (!*UserCancelled && (i < fieldLength))
                {
                    // read the bytes and send the config. bits to the board
                    unsigned char byte;

                    is.read((char*) &byte, 1);
                    assert(0 == is.eof());	// should not hit end-of-file

                    ConfigureFPGA(byte);

                    if (NULL != progressGauge)
                    {
                        *UserCancelled = progressGauge->UserHasCancelled();

                        // output some feedback as the configuration proceeds
                        if (0 == (i % 1000))
                        {
                            progressGauge->Report(is.tellg());
                        }
                    }
                    i++;
                }
                if (!*UserCancelled)
                {
                    // Success if all bytes processed
                    status = (i == fieldLength);
                }

                if (NULL != progressGauge)
                {
                    progressGauge->Report(is.tellg());	// this should set gauge to 100%
                }
	
                // clock a few more times at the end of the configuration process
                // to clean things up...
                for(int ii = 0; ii < 8; ii++)
                {
                    PulseCCLK();
                }

                // insert a delay to let the chip initialize
                InsertDelay(10, MILLISECONDS);
            }
        }
    }
    return status;
}


/// Process an FPGA bitstream file and send the configuration bitstream to the board.
///\return true if the operation was a success, false otherwise
bool XC3SPort::ConfigureFPGA(const char *fileName, bool *UserCancelled)	///< file with FPGA bitstream
{
    bool status;

    status = false;
	XSError& err = GetErr();
	
    // if SVF or BIT file was given
    if (0 != strlen(fileName))
    {
        ifstream is(fileName,ios::binary);  // otherwise open BIT or SVF file

        if (!is || is.fail() || is.eof())
        {
            // error - couldn't open file
            err.SetSeverity(XSErrorMajor);
            err << "could not open " << fileName << "\n";
            err.EndMsg();
        }
        else
        {
            // determine the size of the file
            is.seekg(0,ios::end);	// move pointer to end of file
            streampos streamEndPos = is.tellg();	// pointer position = position of end of file

            is.seekg(0,ios::beg);	// return pointer to beginning of file

            string desc("Configure FPGA"), subdesc("Downloading "+ StripPrefix(fileName));
            progressGauge = new Progress(NULL, &err);
            if (NULL != progressGauge)
            {
                progressGauge->Setup(desc,subdesc,0,streamEndPos);

                status = ConfigureFPGA(is, UserCancelled);

                if (NULL != progressGauge)
                {
                    progressGauge->EndProgress();
                    delete progressGauge;
                    progressGauge = NULL;
                }
            }

            is.close();  // close-up the hex file
        }
    }
	return status;
}
