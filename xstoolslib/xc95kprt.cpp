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


#include <cassert>
#include <fstream>
#include <iostream>
#include <ctime>
#include <string>

#include <string.h>

using namespace std;

#include "utils.h"
#include "hex.h"
#include "bitstrm.h"
#include "xc95kprt.h"
using std::string;


/// Instantiate an XC9500 CPLD object on a given parallel port.
XC95KPort::XC95KPort(void)
{
	;
}


/// Instantiate an XC9500 CPLD object on a given parallel port.
XC95KPort::XC95KPort(XSError* err,	///< pointer to error reporting object
		   unsigned int portNum,	///< parallel port number
		   unsigned int invMask,	///< inversion mask for the parallel port
		   unsigned int posTCK,	///< bit position in parallel port of TCK pin
		   unsigned int posTMS,	///< bit position in parallel port of TMS pin
		   unsigned int posTDI,	///< bit position in parallel port of TDI pin
		   unsigned int posTDO)	///< bit position in parallel port of TDO pin
{
	Setup(err,portNum,invMask,posTCK,posTMS,posTDI,posTDO);
}


/// Setup an XC95KPort object on a given parallel port.
///\return true if the operation was a success, false otherwise
bool XC95KPort::Setup(XSError* err,	///< pointer to error reporting object
		   unsigned int portNum,	///< parallel port number
		   unsigned int invMask,	///< inversion mask for the parallel port
		   unsigned int posTCK,	///< bit position in parallel port of TCK pin
		   unsigned int posTMS,	///< bit position in parallel port of TMS pin
		   unsigned int posTDI,	///< bit position in parallel port of TDI pin
		   unsigned int posTDO)	///< bit position in parallel port of TDO pin
{
	// return true if setup was successful
	return LPTJTAG::Setup(err,portNum,invMask,posTCK,posTMS,posTDI,posTDO,0);
}


/// Detect the presence and type of the XC9500 CPLD chip on the port.
///\return the chip identifier
string XC95KPort::GetChipID()
{
	Bitstream bsir(8);	// BSIR for sending IDCODE instruction
	Bitstream bsdr(32);	// BSDR for holding chip ID
	bsir.SetBits(0, 0,1,1,1,1,1,1,1, -1); // IDCODE instruction for reading chip id
	Bitstream null(0);	// zero-length bitstream

	InitTAP();							// initialize the JTAG TAP controller
	LoadBSIRthenBSDR(bsir,null,bsdr);	// get the ID code from the chip
	return bsdr.ToString();
}


/// Get the 32-bit USERCODE from the XC9500 CPLD chip
///\return the usercode
string XC95KPort::GetUSERCODE()
{
	Bitstream bsir(8);	// BSIR for sending USERCODE instruction
	Bitstream bsdr(32);	// BSDR for holding USERCODE
	bsir.SetBits(0, 1,0,1,1,1,1,1,1, -1); // USERCODE instruction for reading USERCODE
	Bitstream null(0);	// zero-length bitstream

	InitTAP();							// initialize the JTAG TAP controller
    LoadBSIRthenBSDR(bsir, null, bsdr);	// get the ID code from the chip

    unsigned char *code = bsdr.ToCharString();	// change the ID code into a character string
	ConvertUSERCODE(code);	// Change the USERCODE into its canonical form.
    string usercode = (char *) code;
	delete [] code;
	DEBUG_STMT("USERCODE = " << bsdr.ToString() << " " << usercode)
	return usercode;
}

/// Initialize the XC9500 CPLD for configuration through the JTAG interface.
void XC95KPort::InitConfigureCPLD(void)
{
	InitTAP();	// initialize TAP controller of XC95KPort chip to reset state
}


/// Send out a byte of configuration information to the XC9500 Flash.
bool XC95KPort::ConfigureCPLD(unsigned char b)	///< configuration byte
{
	// JTAG configuration bytes contain values for both TDI and TMS,
	// so only four clock pulses are needed per byte
    for (int i = 0; i < 4; i++)
	{
		SetTDI(b&1);	// byte is sent out LSB first
		b >>= 1;
		SetTMS(b&1);
		b >>= 1;
		PulseTCK();
	}

    return true;
}


/// Process SVF and send results through JTAG configuration pins.
/// 	///< stream that delivers SVF
/// 	///< name of SVF file that the stream is attached to
///\return true if the operation was a success, false otherwise
bool XC95KPort::ConfigureCPLD(istream& is, const char *fileName, bool *UserCancelled)
{
    return DownloadSVF(is, fileName, UserCancelled);
}


/// Configure the CPLD with the bit stream stored in an SVF file.
///< file containing SVF
///\return true if the operation was a success, false otherwise
bool XC95KPort::ConfigureCPLD(const char *fileName, bool *UserCancelled)
{
    bool status;

    status = false;
	XSError& err = GetErr();

    // if SVF file was given
    if (0 != strlen(fileName))
    {
        ifstream is(fileName, ios::binary);  // otherwise open SVF file
        if(!is || is.fail() || is.eof())
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

            status = ConfigureCPLD(is, fileName, UserCancelled);

            is.close();  // close-up the hex file
        }
    }
	return status;
}
