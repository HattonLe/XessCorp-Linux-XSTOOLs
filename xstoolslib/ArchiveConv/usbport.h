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


#ifndef USBPORT_H
#define USBPORT_H

// Can't use wtypes.h because compiler complains about including windows.h in MFC programs.
//#include <wtypes.h>
// Just directly define the HANDLE type we need below in the same way as wtypes.h
typedef void* HANDLE;

#include "xserror.h"

#define USB_SUCCESS		0
#define USB_FAILURE		1

// This is the maximum possible size of USB bulk-transfer packets, but the device may
// be using something smaller (like 32 for the XuLA Boards).
#define	USB_MAX_PACKET_SIZE		64

/**
Low-level interface to a USB port.

This object handles a USB port.  It lets you send and receive arbitrarily-sized
packets of data through a given USB endpoint.

*/
class USBPort
{
	public:

	USBPort();

	USBPort(XSError *e, int portNum, int endptNum);

	~USBPort();

	int Setup(XSError *e, int portNum, int endptNum, int pcktSz = USB_MAX_PACKET_SIZE);

	void SetErr(XSError* e);

	XSError& GetErr(void);

	char* GetMPUSBAPIDLLVersion(void);

	int Open(unsigned int num_trials = 1);

	int Close(void);

	int StartBuffer(void);

	int FlushBuffer(void);

	bool IsBufferOn(void);

	int SendRcvPacket( unsigned char* sendData, unsigned long  sendLength,
		unsigned char* rcvData, unsigned long* rcvLength, bool checkFirstByte);

	int GetInfo(void);

	ostream& ReportInfo(ostream& os);

	int SetPROG(unsigned int level);

	int SetFlashEnable(unsigned int level);
	
	private:

	XSError *err;	///< error reporting channel
	int  instance;	///< particular instance of this object among all such objects
	int  endpoint;	///< USB endpoint associated with this object
	HANDLE epIn;	///< handle for endpoint that receives data from a USB peripheral
	HANDLE epOut;	///< handle for endpoint that sends data to a USB peripheral
	char endpointName[20];	///< endpoint name
	char info[USB_MAX_PACKET_SIZE+1]; ///< USB device info (plus room for string-terminator)
	int packetSize; ///< maximum size of USB packets for this device
	struct
	{
		bool on;				///< buffer is ON or OFF
		unsigned char *data;	///< buffer for collecting multiple commands before transmission
		unsigned len;			///< current size of command buffer
		unsigned index;			///< index to next location free location in command buffer
	} buff;			///< buffer for collecting multiple commands before transmission
};


int LoadMPUSBAPIDLL(void);

int GetUSBPortCount(void);


#endif
