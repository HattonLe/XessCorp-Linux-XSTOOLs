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

#include "utils.h"
#include "lptjtag.h"

// pulse TCK for # of TCK pulses < threshold, otherwise insert a delay without pulsing TCK
#define	DO_DELAY_THRESHOLD	50


/// Create a parallel port-based JTAG controller port.
LPTJTAG::LPTJTAG(void): PPort(), JTAGPort()
{
	;
}


/// Create a parallel port-based JTAG controller port.
LPTJTAG::LPTJTAG( XSError* e,	///< pointer to error reporting object
				  unsigned int portNum,		///< parallel port number
				  unsigned int invMask,		///< inversion mask for the parallel port
				  unsigned int pos_tck,		///< bit position in parallel port of TCK pin
				  unsigned int pos_tms,		///< bit position in parallel port of TMS pin
				  unsigned int pos_tdi,		///< bit position in parallel port of TDI pin
				  unsigned int pos_tdo,		///< bit position in parallel port of TDO pin
				  unsigned int pos_prog)	///< bit position in parallel port of FPGA PROG# pin
				  : PPort(e,portNum,invMask), JTAGPort(e)
{
	Setup(e,portNum,invMask,pos_tck,pos_tms,pos_tdi,pos_tdo,pos_prog);
}


/// Setup the parallel port-based JTAG controller port.
bool LPTJTAG::Setup(XSError* e,	///< pointer to error reporting object
				   unsigned int portNum,	///< parallel port number
				   unsigned int invMask,	///< inversion mask for the parallel port
				   unsigned int pos_tck,	///< bit position in parallel port of TCK pin
				   unsigned int pos_tms,	///< bit position in parallel port of TMS pin
				   unsigned int pos_tdi,	///< bit position in parallel port of TDI pin
				   unsigned int pos_tdo,	///< bit position in parallel port of TDO pin
				   unsigned int pos_prog)	///< bit position in parallel port of FPGA PROG# pin
{
	posTCK = pos_tck;
	posTMS = pos_tms;
	posTDI = pos_tdi;
	posTDO = pos_tdo;
	posPROG = pos_prog;
	SetTraceOnOff(false,cerr);	// don't trace TAP signals
	JTAGPort::Setup(e);
	bool pportOK = PPort::Setup(e,portNum,invMask);
	if(pportOK)
		EnableJTAG(true);	// enable JTAG operations through the parallel port
	return pportOK;
}


/// Sense the inverters on the TCK and TDO pins of the JTAG controller port.
void LPTJTAG::SenseInverters(void)
{
	// Not currently implemented for the LPTJTAG object.
	;
}


/// Set the level on the JTAG TCK pin.
void LPTJTAG::SetTCK(unsigned int b)
{
	tckVal = b;
	Out(b,posTCK,posTCK);
}


///< Get the level on the JTAG TCK pin.
///\return the level on the JTAG TCK pin
unsigned int LPTJTAG::GetTCK(void)
{
	tckVal = In(posTCK,posTCK);
	return tckVal;
}


/// Output a number of pulses on the JTAG TCK pin.
void LPTJTAG::PulseTCK(unsigned int numTCKPulses) ///< number of pulses to generate
{
	if(numTCKPulses == 0)
		return;
	
	assert(GetTCK()==0);  // quiescent state of TCK should be zero
	
	if(traceFlag)
		*osTrace << GetTAPStateLabel(currentTAPState).c_str() << "\t"
		<< GetTMS() << " "
		<< GetTDI()	<< " "
		<< GetTDO() << endl;

	if(numTCKPulses >= DO_DELAY_THRESHOLD)
		InsertDelay(numTCKPulses,MICROSECONDS);
	else
	{
		GetTDO();  // get value on TDO before clock pulse (see SendRcvBit)
		UpdateTAPState(numTCKPulses);
		for(unsigned int i=numTCKPulses; i>0; i--)
		{
			SetTCK(~GetTCK());  // toggle TCK output
			SetTCK(~GetTCK());	// toggle it again
		}
	}
}


/// Set the level on the JTAG TMS pin.
void LPTJTAG::SetTMS(unsigned int b)
{
	tmsVal = b;
	Out(b,posTMS,posTMS);
}


/// Get the level on the JTAG TMS pin.
///\return the level on the JTAG TMS pin
unsigned int LPTJTAG::GetTMS(void)
{
	tmsVal = In(posTMS,posTMS);
	return tmsVal;
}


/// Set the level on the JTAG TDI pin.
void LPTJTAG::SetTDI(unsigned int b)
{
	tdiVal = b;
	Out(b,posTDI,posTDI);
}


/// Get the level on the JTAG TDI pin.
///\return the level on the JTAG TDI pin
unsigned int LPTJTAG::GetTDI(void)
{
	tdiVal = In(posTDI,posTDI);
	return tdiVal;
}


/// Get the level on the JTAG TDO pin.
///\return the level on the JTAG TDO pin
unsigned int LPTJTAG::GetTDO(void)
{
	tdoVal = In(posTDO,posTDO);
	return tdoVal;
}


/// Set the level on the FPGA PROG# pin.
///\ return success or failure.
int LPTJTAG::SetPROG(unsigned int b)
{
	Out(b,posPROG,posPROG);
	return true;
}


/// Enable/disable the flash chip.
///\ return success or failure.
int LPTJTAG::SetFlashEnable(unsigned int b)
{
	return false;
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
///SendBits and RcvBits can point to the same bitstream without causing problems.
///
/// The LSB of a bitstream has an index of 0.
void LPTJTAG::SendRcvBitstream(Bitstream& sendBits, ///< bits to send out TDI pin 
					Bitstream& rcvBits,	///< bits received through TDO pin
					bool goToExit) ///< if true, leave shift state and enter exit state when finished 
{
	assert(currentTAPState==ShiftIR || currentTAPState==ShiftDR);
	unsigned int length = MAX(sendBits.GetLength(),rcvBits.GetLength());
	assert(length>0);
	
	// lower the TMS line to make sure the TAP controller stays
	// in the Shift-IR or Shift-DR state for the first length-1 cycles
	SetTMS(0);

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


/// Apply test vectors and collect the responses.
/// This method sends byte-wide test vectors to the port and collects the responses.
///\return the response to a single test vector (if numVectors==1).
unsigned char LPTJTAG::ApplyTestVectors(unsigned char singleVector, unsigned char mask,
										unsigned char *vector, unsigned char *response, unsigned int numVectors)
{
	assert(numVectors!=0);
	assert((numVectors>1 && vector!=NULL) || (numVectors==1 && vector==NULL));
	assert((numVectors>1 && response!=NULL) || (numVectors==1 && response==NULL));
	
	if(numVectors==1)
	{
		return 0;
	}
	else
	{
		return 0;
	}
}


/// Get test vector currently applied to the FPGA.
///\return the test vector.
unsigned char LPTJTAG::GetTestVector(void)
{
	return 0;
}


/// Set the programmable oscillator frequency.
///\return True if the oscillator frequency was set.
bool LPTJTAG::SetOscFrequency(int div,	///< divisor for master frequency
					bool extOscPresent)	///< if true, master frequency arrives from external source, otherwise use internal 100 MHz source in DS1075
{
	return false;	// this method is not operational!!
}


#define MAX_NUM_LPT_DEV		4
static LPTJTAG *primaryLPTJTAG[MAX_NUM_LPT_DEV] = {NULL,};
static LPTJTAG *secondaryLPTJTAG[MAX_NUM_LPT_DEV] = {NULL,};
static int numActivePorts = 0;


/// Scan LPT ports for XESS Boards
///\return the number of XESS Boards found
int ScanLPTJTAG(XSError *err)  ///< pointer to error reporting object
{
	numActivePorts = 0;

	// remove any objects whose LPT connections are no longer working
	int i;
	for(i=0; i<MAX_NUM_LPT_DEV; i++)
	{
		if(primaryLPTJTAG[i] != NULL)
		{
			delete primaryLPTJTAG[i];
			primaryLPTJTAG[i] = NULL;
		}
		if(secondaryLPTJTAG[i] != NULL)
		{
			delete secondaryLPTJTAG[i];
			secondaryLPTJTAG[i] = NULL;
		}
	}

	// scan for and add any new LPT connections
	bool portNumExists[5];	// LPT1 thru LPT4
	int numLPTJTAG = ScanPPort(err,portNumExists);
	for(i=1; i<sizeof(portNumExists)/sizeof(bool); i++)
	{
		if(portNumExists[i])
		{
			DEBUG_STMT("\ntesting lpt "<<i+1)
			if(primaryLPTJTAG[i] == NULL)
			{
				primaryLPTJTAG[i] = new LPTJTAG(err,i+1,0x0b8000,1,2,0,12,7);
				if(primaryLPTJTAG[i]->SetLPTNum(i+1) == false)
				{
					delete primaryLPTJTAG[i];
					primaryLPTJTAG[i] = NULL;
				}
				else
				{
					DEBUG_STMT("lpt "<<i+1<<" was found!!")
						secondaryLPTJTAG[i] = new LPTJTAG(err,i+1,0x0b8000,17,18,19,15,0);
					if(secondaryLPTJTAG[i]->SetLPTNum(i+1) == false)
					{
						delete secondaryLPTJTAG[i];
						secondaryLPTJTAG[i] = NULL;
						delete primaryLPTJTAG[i];
						primaryLPTJTAG[i] = NULL;
					}
					else
						numActivePorts++;
				}
			}
		}
	}
	DEBUG_STMT("Number of active LPT ports =  "<<numActivePorts)
	return numActivePorts;
}


/// Get a pointer to an active LPT port object
LPTJTAG* GetLPTJTAG(int portNum, ///< port index between 1 and # of active ports
					int endptNum) ///< endpoint: 1 for primary endpoint, 2 for secondary endpoint
{
	cerr << "portNum = " << portNum << "\nnumActivePorts = " << numActivePorts << endl;
	portNum--;
	if(portNum>=numActivePorts)
	{
		cerr << "port number too large!!\n";
		return NULL;
	}

	switch(endptNum)
	{
	case 1:  return primaryLPTJTAG[portNum];
	case 2:  return secondaryLPTJTAG[portNum];
	default: return NULL;
	}
}

