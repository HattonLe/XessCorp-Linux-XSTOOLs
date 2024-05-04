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


#ifndef JTAGPORT_H
#define JTAGPORT_H


#include "xserror.h"
#include "bitstrm.h"
#include "progress.h"


#define TRACEJTAG false


/// Identifiers for all possible TAP states.
typedef enum
{
	TestLogicReset=0,	///< Test-Logic-Reset TAP state 
	RunTestIdle=1,	///< Run-Test/Idle TAP state
	SelectDRScan=2,	///< Select-DR-Scan TAP state
	SelectIRScan=3,	///< Select-IR-Scan TAP state
	CaptureDR=4,	///< Capture-DR TAP state
	CaptureIR=5,	///< Capture-IR TAP state
	ShiftDR=6,		///< Shift-DR TAP state
	ShiftIR=7,		///< Shift-IR TAP state
	Exit1DR=8,		///< Exit1-DR TAP state
	Exit1IR=9,		///< Exit1-IR TAP state
	PauseDR=10,		///< Pause-DR TAP state
	PauseIR=11,		///< Pause-IR TAP state
	Exit2DR=12,		///< Exit2-DR TAP state
	Exit2IR=13,		///< Exit2-IR TAP state
	UpdateDR=14,	///< Update-DR TAP state
	UpdateIR=15,	///< Update-IR TAP state
	InvalidTAPState=16	///< Invalid TAP state
}TAPState;


/**
 Low-level and high-level JTAG operations.
 
This object provides JTAG capabilities. It stores the 
state of the JTAG TAP state machine and updates the state as the TMS and 
TCK signals change state. It provides methods that make it easier to 
move between states of the TAP machine and to access the boundary scan 
instruction and data registers.

This object will be inherited along with either the parallel port or USB port object to
provide JTAG capabilities to an actual physical port.

*/
class JTAGPort
{
	public:

	JTAGPort();

	JTAGPort(XSError* e);

	bool Setup(XSError *e, bool traceOnFlag = false, ostream& traceOstream = cerr);

	void SetErr(XSError* e);

	XSError& GetErr(void);

	/// Close the port
    virtual bool Open(unsigned int Param) = 0;
    virtual bool Close(void) = 0;

	/// Start collecting commands into the buffer.
	virtual int StartBuffer(void) = 0;

	/// Stop collecting commands into the buffer and transmit the buffer.
	virtual int FlushBuffer(void) = 0;

	/// Determine if command buffer is on or off
	virtual bool IsBufferOn(void) = 0;

	/// Sense the inverters on the TCK and TDO pins of the JTAG controller port.
	virtual void SenseInverters(void) = 0;

	/// Set the level on the JTAG TCK pin.
	virtual void SetTCK(unsigned int b) = 0;

	/// Get the level on the JTAG TCK pin.
	///\return the level on the JTAG TCK pin
	virtual unsigned int GetTCK(void) = 0;

	/// Output a number of pulses on the JTAG TCK pin.
	virtual void PulseTCK(unsigned int numTCKPulses=1) = 0;

	/// Set the level on the JTAG TMS pin.
	virtual void SetTMS(unsigned int b) = 0;

	/// Get the level on the JTAG TMS pin.
	///\return the level on the JTAG TMS pin
	virtual unsigned int GetTMS(void) = 0;

	/// Set the level on the JTAG TDI pin.
	virtual void SetTDI(unsigned int b) = 0;

	/// Get the level on the JTAG TDI pin.
	///\return the level on the JTAG TDI pin
	virtual unsigned int GetTDI(void) = 0;

	/// Get the level on the JTAG TDO pin.
	///\return the level on the JTAG TDO pin
	virtual unsigned int GetTDO(void) = 0;

	/// Set the level on the FPGA PROG# pin.
	virtual int SetPROG(unsigned int b) = 0;

	/// Enable/disable the flash chip.
	virtual int SetFlashEnable(unsigned int b) = 0;

	/// Send a test vector to the FPGA and receive a response
	virtual unsigned char ApplyTestVectors(unsigned char singleVector, unsigned char mask,
		unsigned char *vector=NULL, unsigned char *response=NULL, unsigned int numVectors=1) = 0;

	/// Get the current test vector applied to the FPGA
	virtual unsigned char GetTestVector(void) = 0;

	/// Set the programmable oscillator frequency and clock source.
	virtual bool SetOscFrequency(int div, bool extOscPresent) = 0;

	void InitTAP(void);

	void SetTraceOnOff(bool f, ostream& os);

	string GetTAPStateLabel(TAPState s);

	TAPState LabelToTAPState(string label);

	void UpdateTAPState(unsigned int numTCKPulses=1);

	void GotoNextTAPState(TAPState nextState);

	void GoThruTAPSequence(TAPState nextState, ...);

	void GotoTAPState(TAPState finalState);

	bool TAPPathsOK(void);

	unsigned int SendRcvBit(unsigned int sendBit);

	/// Output a bitstream through TDI while receiving a bitstream through TDO.
	virtual void SendRcvBitstream(Bitstream& sendBits, Bitstream& rcvBits, bool goToExit=true) = 0;

	void LoadBSIRthenBSDR(Bitstream& instruction, Bitstream& send, Bitstream& recv);

	int GetNumberOfDevices(void);

	int GetIDCODEs(Bitstream ***idcodes);

    bool DownloadSVF(istream& is, const char *fileName, bool *UserCancelled);

    bool DownloadSVF(const char *fileName, bool *UserCancelled);

    bool DownloadBitstream(istream& is, bool *UserCancelled);

    bool DownloadBitstream(const char *fileName, bool *UserCancelled);

    bool DownloadConfigFile(const char *fileName, bool *UserCancelled);


	protected:

	TAPState currentTAPState;	///< state of Test Access Port
	bool traceFlag;				///< trace JTAG states on/off
	ostream* osTrace;			///< output stream for trace info
	unsigned int tckVal;  		///< current value of TCK signal
	unsigned int tmsVal;  		///< current value of TMS signal
	unsigned int tdiVal;  		///< current value of TDI signal
	unsigned int tdoVal;  		///< current value of TDO signal
	int numDevices;				///< number of devices in JTAG chain


	private:

	XSError		*err;			///< error-reporting object
	Progress	*progressGauge;	///< indicates progress of operations

private:
    bool SendSVF(char *svfStart, bool *Successful);
    bool SendBitstream(istream& is, bool *Successful);
};

#endif
