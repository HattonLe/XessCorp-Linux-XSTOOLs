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


#include <string>
#include <fstream>
#include <cassert>
#include <ctime>
#include <cstdarg>
using namespace std;

#include "usbjtag.h"
#include "usbcmd.h"
#include "_mpusbapi.h"
#include "utils.h"


static const bool doBulkIO = true;


/// Create a USB-based JTAG controller port.
USBJTAG::USBJTAG(void): USBPort(), JTAGPort()
{
	;
}


/// Create a USB-based JTAG controller port.
USBJTAG::USBJTAG(XSError* e,	// pointer to error reporting object
	unsigned int portNum,	// USB port number
	unsigned int endptNum		// USB endpoint number
	): USBPort(e,portNum,endptNum), JTAGPort(e)
{
	Setup(e,portNum,endptNum);
}


/// Initialize the USB-based JTAG controller port.
bool USBJTAG::Setup(XSError* e,	// pointer to error reporting object
	unsigned int portNum,	// USB port number
	unsigned int endptNum)	// USB endpoint number
{
	SetTraceOnOff(false,cerr);	// don't trace TAP signals
	USBPort::Setup(e,portNum,endptNum);
	return true;
}


/// Sets the error reporting channel.
void USBJTAG::SetErr(XSError* e)
{
	JTAGPort::SetErr(e);
}


/// Provides access to the error reporting channel.
XSError& USBJTAG::GetErr(void)
{
	return JTAGPort::GetErr();
}


/// Sense the inverters on the TCK and TDO pins of the JTAG controller port.
void USBJTAG::SenseInverters(void)
{
	unsigned char cmd[] = {SENSE_INVERTERS_CMD};
	unsigned char ack[sizeof(cmd)];
	unsigned long ackLength = sizeof(ack);
	SendRcvPacket(cmd, sizeof(cmd), ack, &ackLength, true);
}


/// Set the level on the JTAG TCK pin.
void USBJTAG::SetTCK(unsigned int b)
{
	tckVal = b; // the TCK doesn't really get set here when USB-based device is used
}


/// Get the level on the JTAG TCK pin.
///\return the level on the JTAG TCK pin
unsigned int USBJTAG::GetTCK(void)
{
	return tckVal;  // just return value in internal TCK storage when a USB-based device is used
}


/// Output a number of pulses on the JTAG TCK pin.
void USBJTAG::PulseTCK(unsigned int numTCKPulses) ///< number of pulses to generate
{
	if(numTCKPulses == 0)
		return;
	
	assert(GetTCK()==0);  // quiescent state of TCK should be zero
	
	if(traceFlag)
		*osTrace << GetTAPStateLabel(currentTAPState).c_str() << "\t"
		<< GetTMS() << " "
		<< GetTDI()	<< " "
		<< GetTDO() << endl;

	tdoVal = SingleIO(GetTMS(),GetTDI(),1);  // get value on TDO before clock pulse (see SendRcvBit)
	RunTest(numTCKPulses-1);
	UpdateTAPState(numTCKPulses);
}


/// Set the level on the JTAG TMS pin.
void USBJTAG::SetTMS(unsigned int b)
{
	tmsVal = b; // the TMS doesn't really get set here when USB-based device is used
}


/// Get the level on the JTAG TMS pin.
///\return the level on the JTAG TMS pin
unsigned int USBJTAG::GetTMS(void)
{
	return tmsVal;
}


/// Set the level on the JTAG TDI pin.
void USBJTAG::SetTDI(unsigned int b)
{
	tdiVal = b; // the TMS doesn't really get set here when USB-based device is used
}


/// Get the level on the JTAG TDI pin.
///\return the level on the JTAG TDI pin
unsigned int USBJTAG::GetTDI(void)
{
	return tdiVal;
}


/// Get the level on the JTAG TDO pin.
///\return the level on the JTAG TDO pin
unsigned int USBJTAG::GetTDO(void)
{
	return tdoVal;
}


/// Set the level on the FPGA PROG# pin.
///\ return success or failure.
int USBJTAG::SetPROG(unsigned int b)
{
	return USBPort::SetPROG(b);
}


/// Enable/disable the flash chip.
///\ return success or failure.
int USBJTAG::SetFlashEnable(unsigned int b)
{
	return USBPort::SetFlashEnable(b);
}


#define MAX(a,b)	((a)<(b)?(b):(a))
/// Transmit a bitstream through TDI while receiving a bitstream through TDO.
///
/// This subroutine assumes the TAP controller state is
/// Shift-IR or Shift-DR upon entry.  Upon termination, this subroutine
/// will leave the TAP controller in the Exit1-IR or Exit1-DR state.
///
/// Either sendBits or rcvBits can be a zero-length bitstream if you don't
/// want to transmit or receive bits during a particular call to this subroutine.
/// For example, you may want to load the BSDR with a bit pattern but
/// you might not care what data gets shifted out of the BSDR during
/// the loading.
///
/// SendBits and RcvBits can point to the same bitstream without causing problems.
///
/// The LSB of a bitstream has an index of 0.
void USBJTAG::SendRcvBitstream(Bitstream& sendBits, ///< bits to send out TDI pin 
					Bitstream& rcvBits,	///< bits received through TDO pin
					bool goToExit) ///< if true, leave shift state and enter exit state when finished 
{
	assert(currentTAPState==ShiftIR || currentTAPState==ShiftDR);
	unsigned int length = MAX(sendBits.GetLength(),rcvBits.GetLength());
	assert(length>0);
	
	// lower the TMS line to make sure the TAP controller stays
	// in the Shift-IR or Shift-DR state for the first length-1 cycles
	SetTMS(0);

	if(doBulkIO)
	{
		if(traceFlag)
			*osTrace << GetTAPStateLabel(currentTAPState).c_str() << "\t"
			<< "TDI:" << sendBits.GetLength()  << " "
			<< "TDO:" << rcvBits.GetLength() << endl;
		
		// create storage to hold the bits coming out of TDO
		unsigned char *tdoBits = rcvBits.GetLength()==0 ? NULL : rcvBits.ToCharString();

		// resize sendBits to make it as large as the number of bits that will be read from TDO
		int sendLength = sendBits.GetLength();
		if(sendLength!=0 && sendLength<length)
			sendBits.Resize(length);
		unsigned char *tdiBits = sendBits.ToCharString();
		
		// do the bulk TDI-TDO transfer
		int status = BulkIO(length,tdiBits,tdoBits);
		assert(status==0);
		
		// convert the TDO bits back into a Bitstream
		rcvBits.FromCharString(rcvBits.GetLength(),tdoBits);
		delete tdoBits;
		
		// restore sendBits to its original size if needed 
		if(sendLength<length)
			sendBits.Resize(sendLength);
		if(tdiBits != NULL)
			delete tdiBits;
		
		if(goToExit)
		{
			// TMS is 1 after jtag_bulk_io() completes
			SetTMS(1);
			
			// Set the state to Exit1IR or Exit1DR after jtag_bulk_io() completes
			if(currentTAPState==ShiftIR)
				currentTAPState=Exit1IR;
			else // if(currentTAPState==ShiftDR)
				currentTAPState=Exit1DR;
		}
	}
	else
		for( unsigned int i=0; i<length; i++ )
		{
			unsigned int rcvBit;

			// On the last bit, raise the TMS line so the TAP
			// controller will move out of the Shift state into
			// the Exit1 state.
			if( i==length-1 && goToExit )
				SetTMS(1);

			// send the next bit if the bitstream is not empty
			if(sendBits.GetLength() > i)
				rcvBit = SendRcvBit(sendBits[i]);
			/* else just shift in a zero */
			else
				rcvBit = SendRcvBit(0);

			// store the received bit if the bitstream is not empty
			if(rcvBits.GetLength() > i)
				rcvBits.SetBit(i,rcvBit);
		}

	assert((currentTAPState==Exit1DR && goToExit) || (currentTAPState==Exit1IR && goToExit) ||
		(currentTAPState==ShiftDR && !goToExit) || (currentTAPState==ShiftDR && !goToExit));
}


/// Perform a single JTAG transaction: read TDO, set TMS and TDI, and pulse TCK.
///\return the value read on the TDO pin. 
int USBJTAG::SingleIO(int tms,	///< level to output on TMS pin
			int tdi,			///< level to output on TDI pin
			int readTDO)		///< if true, read the level on the TDO pin if true; otherwise, don't read TDO
{
	unsigned char tmsTdi[2];
	tmsTdi[1]= ((tdi & 1) << 1) | (tms & 1);
	if(!readTDO)
	{
		unsigned long rcvLength = 0;
		tmsTdi[0] = TMS_TDI_CMD;
		SendRcvPacket(tmsTdi, 2, NULL, &rcvLength, true);
		return 0;
	}
	else
	{
		unsigned char tdo[2];
		unsigned long rcvLength = 2;
		tmsTdi[0] = TMS_TDI_TDO_CMD;
		SendRcvPacket(tmsTdi, 2, tdo, &rcvLength, true);
		return (tdo[1]>>2) & 1;
	}
}


/// Send a number of bits through the TDI pin while (possibly) receiving bits from the TDO pin.
///\return USB_SUCCESS or USB_FAILURE
int USBJTAG::BulkIO(unsigned int length,	///< number of TDI bits to send or TDO bits to receive
				unsigned char* tdi,			///< bits to send thru TDI with first bit in bit 0 of tdi[0]
				unsigned char* tdo)			///< pointer to array that gets loaded with bits received thru TDO 
{
	if(tdi != NULL && tdo == NULL)
	{
		unsigned char cmd[] = {TDI_CMD,0,0,0,0};
		cmd[1] =  length      & 0xff;
		cmd[2] = (length>> 8) & 0xff;
		cmd[3] = (length>>16) & 0xff;
		cmd[4] = (length>>24) & 0xff;
		unsigned long rcvLength = 0;
		if(SendRcvPacket(cmd, sizeof(cmd), NULL, &rcvLength, true) == USB_SUCCESS)
		{
			rcvLength = 0;
			return SendRcvPacket(tdi,(length+7)/8, NULL, &rcvLength, false);
		}
		return USB_FAILURE;
	}
	else if( tdi == NULL && tdo != NULL)
	{
		unsigned char cmd[] = {TDO_CMD,0,0,0,0};
		cmd[1] =  length      & 0xff;
		cmd[2] = (length>> 8) & 0xff;
		cmd[3] = (length>>16) & 0xff;
		cmd[4] = (length>>24) & 0xff;
		unsigned long rcvLength = 0;
		if(SendRcvPacket(cmd, sizeof(cmd), NULL, &rcvLength, true) == USB_SUCCESS)
		{
			rcvLength = (length+7)/8;
			return SendRcvPacket(NULL, 0, tdo, &rcvLength, false);
		}
		return USB_FAILURE;
	}
	else if( tdi != NULL && tdo != NULL)
	{
		unsigned char cmd[] = {TDI_TDO_CMD,0,0,0,0};
		cmd[1] =  length      & 0xff;
		cmd[2] = (length>> 8) & 0xff;
		cmd[3] = (length>>16) & 0xff;
		cmd[4] = (length>>24) & 0xff;
		unsigned long rcvLength = 0;
		if(SendRcvPacket(cmd, sizeof(cmd), NULL, &rcvLength, true) == USB_SUCCESS)
		{
			rcvLength = (length+7)/8;
			return SendRcvPacket(tdi,(length+7)/8, tdo, &rcvLength, false);
		}
		return USB_FAILURE;
	}

	return USB_SUCCESS;
}


/// Perform JTAG RUN-TEST operation by pulsing the TCK pin a number of times.
///\return USB_SUCCESS or USB_FAILURE
int USBJTAG::RunTest(unsigned int numTCKPulses) ///< number of pulses to generate
{
	if(numTCKPulses == 0)
		return USB_SUCCESS;
	
	unsigned char cmd[] = {RUNTEST_CMD,0,0,0,0};
	unsigned char ack[sizeof(cmd)];
	cmd[1] =  numTCKPulses      & 0xff;
	cmd[2] = (numTCKPulses>> 8) & 0xff;
	cmd[3] = (numTCKPulses>>16) & 0xff;
	cmd[4] = (numTCKPulses>>24) & 0xff;
	unsigned long ackLength = sizeof(ack);
	return SendRcvPacket(cmd, sizeof(cmd), ack, &ackLength, true);
}


/// Apply test vectors and collect the responses.
/// This method sends byte-wide test vectors and receives the responses through the USB port.
///\return the response to a single test vector (if numVectors==1).
unsigned char USBJTAG::ApplyTestVectors(unsigned char singleVector, unsigned char mask, unsigned char *vector,
											unsigned char *response, unsigned int numVectors)
{
	assert(numVectors!=0);
	assert((numVectors>1 && vector!=NULL) || (numVectors==1 && vector==NULL));
	assert((numVectors>1 && response!=NULL) || (numVectors==1 && response==NULL));
	
	if(numVectors==1)
	{
		unsigned char cmd[] = {SINGLE_TEST_VECTOR_CMD,mask,singleVector};
		unsigned long rcvLength = 3;
		unsigned char resp[3];
		if( SendRcvPacket(cmd, sizeof(cmd), resp, &rcvLength, true) == USB_FAILURE )
			return 0;
		else
			return resp[2];
	}
	else
	{
		return 0;
	}
}


/// Get current test vector being applied to the FPGA.
///\return the value of the current test vector.
unsigned char USBJTAG::GetTestVector(void)
{
		unsigned char cmd[] = {GET_TEST_VECTOR_CMD};
		unsigned long rcvLength = 3;
		unsigned char resp[3];
		if( SendRcvPacket(cmd, sizeof(cmd), resp, &rcvLength, true) == USB_FAILURE )
			return 0;
		else
			return resp[2];
}


/// Set the programmable oscillator frequency.
///\return True if the oscillator frequency was set.
bool USBJTAG::SetOscFrequency(int div,	///< divisor for master frequency
					bool extOscPresent)	///< if true, master frequency arrives from external source, otherwise use internal 100 MHz source in DS1075
{
	unsigned char cmd[] = {SET_OSC_FREQ_CMD,0,0,0,0};
	unsigned char ack[sizeof(cmd)];
	cmd[1] = div      & 0xff;
	cmd[2] = (div>>8) & 0xff;
	cmd[3] = extOscPresent;
	unsigned long ackLength = sizeof(ack);
	return SendRcvPacket(cmd, sizeof(cmd), ack, &ackLength, true) == USB_SUCCESS;
}


static USBJTAG *primaryUSBJTAG[MAX_NUM_MPUSB_DEV] = {NULL,};
static USBJTAG *secondaryUSBJTAG[MAX_NUM_MPUSB_DEV] = {NULL,};
static int numActivePorts = 0;


/// Scan USB ports for XSUSB devices
///\return the number of XSUSB devices found
int ScanUSBJTAG(XSError *err,				///< pointer to error reporting object
				unsigned int num_trials)	///< try this number of times to open each device
{
	numActivePorts = 0;

	// remove any objects whose USB connections are no longer working
	int i;
	for(i=0; i<MAX_NUM_MPUSB_DEV; i++)
	{
		if(primaryUSBJTAG[i] != NULL)
		{
			primaryUSBJTAG[i]->Close();
			if(primaryUSBJTAG[i]->Open(num_trials) == USB_FAILURE)
			{
				delete primaryUSBJTAG[i];
				primaryUSBJTAG[i] = NULL;
			}
			else
			{
				numActivePorts++;
				primaryUSBJTAG[i]->Close(); // don't leave USB ports open when not actively in use
			}
		}
		if(secondaryUSBJTAG[i] != NULL)
		{
			secondaryUSBJTAG[i]->Close();
			if(secondaryUSBJTAG[i]->Open(num_trials) == USB_FAILURE)
			{
				delete secondaryUSBJTAG[i];
				secondaryUSBJTAG[i] = NULL;
			}
			else
			{
				secondaryUSBJTAG[i]->Close(); // don't leave USB ports open when not actively in use
			}
		}
	}

	// scan for and add any new USB connections
	int numUSBJTAG = GetUSBPortCount();
	assert(numUSBJTAG >= numActivePorts);
	for(i=0; i<MAX_NUM_MPUSB_DEV && numUSBJTAG>numActivePorts; i++)
	{
		if(primaryUSBJTAG[i] == NULL)
		{
			primaryUSBJTAG[i] = new USBJTAG(err,i,1);
			if(primaryUSBJTAG[i]->Open(num_trials) == USB_FAILURE)
			{
				delete primaryUSBJTAG[i];
				primaryUSBJTAG[i] = NULL;
			}
			else
			{
				// primary USB port to FPGA was found, so we have an active USB port
				primaryUSBJTAG[i]->Close();
				numActivePorts++;

				// look for the secondary port to the CPLD (doesn't exist if this is XuLA board)
				secondaryUSBJTAG[i] = new USBJTAG(err,i,2);
				if(secondaryUSBJTAG[i]->Open(num_trials) == USB_FAILURE)
				{
					delete secondaryUSBJTAG[i];
					secondaryUSBJTAG[i] = NULL;
				}
				else
				{
					secondaryUSBJTAG[i]->Close();
				}
			}
		}
	}
	return numActivePorts;
}


/// Get a pointer to an active USB port object
///\return a pointer to the XSUSB device
USBJTAG* GetUSBJTAG(int portNum, ///< port index between 0 and # of active ports-1
					int endptNum) ///< endpoint: 1 for primary endpoint, 2 for secondary endpoint
{
	DEBUG_STMT("portNum = " << portNum << "\nnumActivePorts = " << numActivePorts << endl)
	if(portNum>=numActivePorts)
	{
		cerr << "port number too large!!\n";
		return NULL;
	}

	int i,j;
	for(i=0,j=0; i<MAX_NUM_MPUSB_DEV; i++)
	{
		if(primaryUSBJTAG[i] != NULL)
		{
			if(j == portNum)
			{
				switch(endptNum)
				{
				case 1: return primaryUSBJTAG[i];
				case 2: return secondaryUSBJTAG[i];
				default: return NULL;
				}
			}
			j++;
		}
	}

	DEBUG_STMT("Couldn't find an active USBJTAG port!!\n")
	return NULL;
}

