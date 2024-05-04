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


#ifndef USBJTAG_H
#define USBJTAG_H

#include "bitstrm.h"
#include "xserror.h"
#include "jtagport.h"
#include "usbport.h"


/**
Performs JTAG operations through a USB port.

This object inherits from the JTAG port object and the USB port object
to create an object that supports JTAG operations through the parallel port.
*/
class USBJTAG : public JTAGPort, public USBPort
{
	public:

	USBJTAG();

	USBJTAG(XSError* e, unsigned int portNum, unsigned int endptNum);

	bool Setup(XSError* e, unsigned int portNum, unsigned int endptNum);

	bool Close(void) { return USBPort::Close() == USB_SUCCESS ? true : false; }

	void SetErr(XSError* e);

	XSError& GetErr(void);

	void SenseInverters(void);

	int StartBuffer(void) { return USBPort::StartBuffer(); };

	int FlushBuffer(void) { return USBPort::FlushBuffer(); };

	bool IsBufferOn(void) { return USBPort::IsBufferOn(); }

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


	protected:


	private:

	int SingleIO(int tms, int tdi, int readTDO);

	int BulkIO(unsigned int length, unsigned char* tdi, unsigned char* tdo);

	int RunTest(unsigned int numTCKPulses);

};


int ScanUSBJTAG(XSError *err, unsigned int num_trials = 1);

USBJTAG* GetUSBJTAG(int portNum, int endptNum);


#endif
