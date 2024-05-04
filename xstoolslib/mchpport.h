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


#ifndef MCHPPORT_H
#define MCHPPORT_H

#include "hexrecrd.h"
#include "xserror.h"
// make sure Progress.h comes before PPort.h or else "MFC apps should not include windows.h" error occurs
#include "progress.h"
#include "pport.h"


/**
MchpPort objects add Microchip ICSP capabilities to a parallel port object.

A Microchip port can control the MCLR, PGM, PGC and PGD pins.
*/ 
class MchpPort : public PPort
{
	public:

	/* begin MchpPort methods */

	/* method: MchpPort */
	// Create a Microchip port
	MchpPort();

	/* method: MchpPort */
	// Create a parallel port-based Microchip port
	MchpPort(XSError* e, // pointer to error reporting object
		unsigned int portNum); // parallel port number

	/* method: Setup */
	// Setup a parallel port-based JTAG controller port
	bool Setup(XSError* e, // pointer to error reporting object
		unsigned int portNum); // parallel port number

	/* method: SetTraceOnOff */
	// enables/disables trace of JTAG signals
	void SetTraceOnOff(bool f, // trace if true, no trace if false
		ostream& os // trace info goes to this stream
		);

	/* method: SetMCLR */
	// Set the value of the MCLR_N bit
	void SetMCLR_N(unsigned int b);

	/* method: GetMCLR */
	// Get the value output on the MCLR_N bit
	unsigned int GetMCLR_N(void);

	/* method: SetPGM */
	// Set the value of the PGM bit
	void SetPGM(unsigned int b);

	/* method: GetPGM */
	// Get the value output on the PGM bit
	unsigned int GetPGM(void);

	/* method: SetPGC */
	// Set the value of the PGC bit
	void SetPGC(unsigned int b);

	/* method: GetPGC */
	// Get the value output on the PGC bit
	unsigned int GetPGC(void);

	/* method: SetPGD */
	// Set the value of the PGD bit
	void SetPGD(unsigned int b);

	/* method: GetPGD */
	// Get the value output on the PGD bit
	unsigned int GetPGD(void);

	/* method: EnterICSP */
	// Enter the Microchip in-circuit programming state
	void EnterICSP(void);

	/* method: ExitICSP */
	// Exit the Microchip in-circuit programming state
	void ExitICSP(void);

	/* method: ICSPIO */
	// Output a command+operand to the Microchip device and return the 8-bit result
	unsigned int ICSPIO(
		unsigned int cmd,
		unsigned int operand
		);

	/* method: WaitWhileProgramming */
	// Output NOP instruction but hold PGC high for programming to complete
	void WaitWhileProgramming(void);

	/* method: BulkErase */
	// Erase area of Microchip device
	void BulkErase(unsigned int area);

	/* method: RowErase */
	// Erase rows of program memory in the Microchip device
	void RowErase(unsigned int loAddr, unsigned int hiAddr);


	/* method: Program */
	// Program Microchip device with data from hex record
	bool Program(HexRecord& hx);

	/* method: Read */
	// Read contents of Microchip device into a hex record
	bool Read(
		HexRecord& hx,			// hex record to store data in
		unsigned long loAddr,	// begin reading at this address
		unsigned long hiAddr	// end reading at this address
		);

	/* method: Download */
	// download the Microchip device with the contents of a HEX file
	bool Download(string& hexfileName);

	/* method: Download */
	// download the Microchip device with the contents arriving through a stream
	bool Download(istream& is);

	/* method: Upload */
	// upload the Microchip device to a hex file
	bool Upload(
		string& hexfileName,	// dump uploaded data to this file
		const char* format,		// hex file format
		unsigned long loAddr,	// start fetching data from this address
		unsigned long hiAddr	// stop at this address
		);

	/* method: Upload */
	// upload the Microchip device to a stream
	bool Upload(
		ostream& os,			// dump uploaded data to this stream
		const char* format,		// hex file format
		unsigned long loAddr,	// start fetching data from this address
		unsigned long hiAddr	// stop at this address
		);

	/* method: Erase */
	// Erase contents of Microchip device
	bool Erase(void);

	/* end MchpPort methods */

	protected:

	Progress *progressGauge;	// indicates progress of operations


	private:

	bool traceFlag;					// trace JTAG states on/off
	ostream* osTrace;				// output stream for trace info

	// These store the current values of the Microchip programming signals
	unsigned int oscVal;	// current value of OSC signal
	unsigned int mclr_nVal;	// current value of MCLR_N signal
	unsigned int pgmVal;	// current value of PGM signal
	unsigned int pgcVal;	// current value of PGC signal
	unsigned int pgd_oVal;	// current value of PGD output signal
	unsigned int pgd_iVal;	// current value of PGD input signal
};

/* begin MchpPort functions */

/* end MchpPort functions */

#endif
