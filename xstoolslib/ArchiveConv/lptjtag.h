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


#ifndef LPTJTAG_H
#define LPTJTAG_H

#include "bitstrm.h"
#include "xserror.h"
#include "jtagport.h"
#include "pport.h"


/**
Performs JTAG operations through the parallel port.

This object inherits from the JTAG port object and the parallel port object
to create an object that supports JTAG operations through the parallel port.
*/
class LPTJTAG : public JTAGPort, public PPort
{
	public:

	LPTJTAG();

	LPTJTAG(XSError* e, unsigned int portNum, unsigned int invMask,
		unsigned int pos_tck, unsigned int pos_tms, unsigned int pos_tdi,
		unsigned int pos_tdo, unsigned int pos_prog);

	bool Setup(XSError* e, unsigned int portNum, unsigned int invMask,
		unsigned int pos_tck, unsigned int pos_tms, unsigned int pos_tdi,
		unsigned int pos_tdo, unsigned int pos_prog);

	bool Close(void) { return true; }

	void SetErr(XSError* e) { JTAGPort::SetErr(e); }

	XSError& GetErr(void) { return JTAGPort::GetErr(); }

	void SenseInverters(void);

	int StartBuffer(void) { return PPort::StartBuffer(); };

	int FlushBuffer(void) { return PPort::FlushBuffer(); };

	bool IsBufferOn(void) { return PPort::IsBufferOn(); }

	void SetTCK(unsigned int b);

	unsigned int GetTCK(void);

	void PulseTCK(unsigned int numTCKPulses=1);

	void SetTMS(unsigned int b);

	unsigned int GetTMS(void);

	void SetTDI(unsigned int b);

	unsigned int GetTDI(void);

	unsigned int GetTDO(void);

	int SetPROG(unsigned int b);

	int SetFlashEnable(unsigned int b);

	void SendRcvBitstream(Bitstream& sendBits, Bitstream& rcvBits, bool goToExit=true);

	unsigned char ApplyTestVectors(unsigned char singleVector, unsigned char mask,
		unsigned char *vector=NULL, unsigned char *response=NULL, unsigned int numVectors=1);

	unsigned char GetTestVector(void);

	bool SetOscFrequency(int div, bool extOscPresent);


	private:

	unsigned int posTCK;	///< position of JTAG clock pin
	unsigned int posTMS;	///< position of JTAG mode pin
	unsigned int posTDI;	///< position of JTAG data input pin
	unsigned int posTDO;	///< position of JTAG data output pin
	unsigned int posPROG;	///< position of FPGA PROG# pin
};


int ScanLPTJTAG(XSError *err);

LPTJTAG* GetLPTJTAG(int portNum, int endptNum);

#endif
