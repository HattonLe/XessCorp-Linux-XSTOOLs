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
#include <wtypes.h>
using namespace std;

#include "libusb_wrapper.h"
#include "winusb_wrapper.h"
#include "mpusbapi.h"
#include "_mpusbapi.h"
#include "bitstrm.h"
#include "usbport.h"
#include "usbcmd.h"

// I needed these declarations from utils.h, but directly including utils.h brings too much baggage...
string GetXSTOOLSParameter(char *name);
string GetXSTOOLSParameter(string& name);


#define USB_INVALID_INSTANCE -1
#define USB_INVALID_ENDPOINT -1
#define USB_TIMEOUT INFINITE

// positions and size of fields in packets returned by USB device
#define OPCODE_START 0
#define OPCODE_SIZE 1

// positions and sizes of single TMS-TDI packet sent to USB device
#define TMS_TDI_BITS_START ( OPCODE_START + OPCODE_SIZE )
#define TMS_TDI_BITS_SIZE 1

// positions and sizes of fields in JTAG TDI packet sent to USB device
#define NUM_TDI_BITS_START ( OPCODE_START + OPCODE_SIZE )
#define NUM_TDI_BITS_SIZE 4
#define TDI_BITS_START ( NUM_TDI_BITS_START + NUM_TDI_BITS_SIZE )
#define TDI_BITS_SIZE ( MAX_PACKET_SIZE - TDI_BITS_START )

//static char *mpusb_vid_pid = "vid_04d8&pid_000c";    // VID,PID for USB device (Microchip)
static char *mpusbVidPid                   = "vid_04d8&pid_ff8c"; // VID,PID for USB device (Microchip VID and XESS PID)

static const char *mpusbEndpointNamePrefix = "\\MCHP_EP";

static HINSTANCE mpusbapi_handle           = NULL; // init API handle to NULL until the associated DLL is loaded


// List of micro manufacturers used in the XESS USB peripherals.
static char *manufacturerTbl []
    = {
    "UNKNOWN",
    "Microchip",
    };


/// Create a USB port object.
USBPort::USBPort( void )
{
    instance   = USB_INVALID_INSTANCE;
    endpoint   = USB_INVALID_ENDPOINT;
    epOut      = epIn = INVALID_HANDLE_VALUE;
    buff.on    = false;
    buff.data  = NULL;
    buff.len   = 0;
    buff.index = 0;
    LoadMPUSBAPIDLL();
}



/// Create a USB port object.
USBPort::USBPort( XSError *e,   ///< error reporting channel
                  int portNum,  ///< number of USB port
                  int endptNum ) ///< endpoint number
{
    Setup( e, portNum, endptNum );
    LoadMPUSBAPIDLL();
}



/// Destroy a USB port object.
USBPort::~USBPort( void )
{
    Close();
}



/// Initialize a USB port object.
///\return USB_SUCCESS or USB_FAILURE
int USBPort::Setup( XSError *e, ///< error reporting channel
                    int portNum, ///< number of USB port
                    int endptNum, ///< endpoint number
                    int pcktSz ) ///< maximum packet size
{
    SetErr( e );
    instance   = portNum;
    endpoint   = endptNum;
    sprintf( endpointName, "%s%d", mpusbEndpointNamePrefix, endptNum );
    epOut      = epIn = INVALID_HANDLE_VALUE;
    packetSize = pcktSz;
    buff.on    = false;
    buff.data  = NULL;
    buff.len   = 0;
    buff.index = 0;
    return USB_SUCCESS;
}



/// Set the error reporting channel.
void USBPort::SetErr( XSError *e )
{
    err = e;
}



/// Provide access to the error reporting channel.
XSError &USBPort::GetErr( void )
{
    return *err;
}



static char versionString[6];

/// Get the version of the Microchip USB driver API DLL.
///\return string with the version number
char *USBPort::GetMPUSBAPIDLLVersion( void )
{
    if ( LoadMPUSBAPIDLL() != USB_SUCCESS )
    {
        XSError err( cerr );
        string msg( "Unable to load mpusbapi.dll." );
        err.SimpleMsg( XSErrorFatal, msg );
    }

    DWORD version = MPUSBGetDLLVersion();
    sprintf( versionString, "%d.%d", version >> 16, version & 0xFFFF );
    return versionString;
}



/// Open input and output endpoints of a USB port object.
///\return USB_SUCCESS or USB_FAILURE
int USBPort::Open( unsigned int num_trials ) ///< try this number of times to open the device
{
    unsigned int i;

    // open channel used to send to the USB device if it is not already opened
    if ( epOut == INVALID_HANDLE_VALUE )
    {
        for ( i = num_trials;
              i != 0
              && ( epOut = MPUSBOpen( instance, mpusbVidPid, endpointName, MP_WRITE, 0 ) ) == INVALID_HANDLE_VALUE;
              i-- )
        {
            ;   // try for a while to see if the USB endpoint will open
        }
        if ( epOut == INVALID_HANDLE_VALUE )
            return USB_FAILURE;
    }

    // open channel used to receive from the USB device if it is not already opened
    if ( epIn == INVALID_HANDLE_VALUE )
    {
        for ( i = num_trials;
              i != 0
              && ( epIn = MPUSBOpen( instance, mpusbVidPid, endpointName, MP_READ, 0 ) ) == INVALID_HANDLE_VALUE;
              i-- )
        {
            ;   // try for a while to see if the USB endpoint will open
        }
        if ( epIn == INVALID_HANDLE_VALUE )
        {
            MPUSBClose( epOut ); // close first endpoint that was successfully opened
            epOut = INVALID_HANDLE_VALUE;
            return USB_FAILURE;
        }
    }

    return USB_SUCCESS;
} // Open



/// Close the input and output endpoints of a USB port object.
///\return USB_SUCCESS or USB_FAILURE
int USBPort::Close( void )
{
    int outClosed = MPUSBClose( epOut );
    epOut = INVALID_HANDLE_VALUE;
    int inClosed  = MPUSBClose( epIn );
    epIn  = INVALID_HANDLE_VALUE;
    return ( outClosed && inClosed ) ? USB_SUCCESS : USB_FAILURE;
}



/// Start collecting commands into the buffer.
///\return USB_SUCCESS or USB_FAILURE
int USBPort::StartBuffer( void )
{
    assert( buff.data == NULL );
    assert( buff.len == 0 );
    assert( buff.index == 0 );
    buff.on = true;
    unsigned char cmd []    = { DISABLE_RETURN_CMD };
    unsigned long rcvLength = 0;
    return SendRcvPacket( cmd, sizeof( cmd ), NULL, &rcvLength, false );
}



/// Stop collecting commands into the buffer and transmit the buffer.
///\return USB_SUCCESS or USB_FAILURE
int USBPort::FlushBuffer( void )
{
    int status = USB_SUCCESS;
    if ( buff.on )
        if ( ( buff.data != NULL ) && ( buff.index > 0 ) )
        {
            unsigned char cmd []    = { ENABLE_RETURN_CMD };
            unsigned long rcvLength = 0;
            SendRcvPacket( cmd, sizeof( cmd ), NULL, &rcvLength, false );
            buff.on = false;
            status  = SendRcvPacket( buff.data, buff.len, NULL, &rcvLength, false );
        }

    buff.on    = false;
    if ( buff.data != NULL )
        free( (void *)buff.data );
    buff.data  = NULL;
    buff.len   = 0;
    buff.index = 0;
    return status;
}



/// Determine if command buffer is on or off.
///\return true or false if buffering is on or off, respectively.
bool USBPort::IsBufferOn( void )
{
    return buff.on;
}



/// Send and (possibly) receive packets of data to/from a USB peripheral.
///\return USB_SUCCESS or USB_FAILURE
int USBPort::SendRcvPacket( unsigned char *sendData, ///< packet of bytes to send to USB peripheral
                            unsigned long sendLength, ///< number of bytes to send
                            unsigned char *rcvData, ///< gets loaded with bytes received from the USB peripheral
                            unsigned long *rcvLength, ///< number of bytes received
                            bool checkFirstByte ) /// if true, sendData[0] is compared to rcvData[0] to see if they match
{
    unsigned long sentDataLength;

    if ( buff.on )
    {
        if ( ( sendData != NULL ) && ( sendLength > 0 ) )
        {
            // determine the number of USB packets needed to hold the data to be sent.
            unsigned long allocLen = ( ( sendLength + packetSize - 1 ) / packetSize ) * packetSize;
            assert( allocLen % packetSize == 0 );
            if ( buff.data == NULL )
            {
                buff.data  = (unsigned char *)malloc( allocLen * sizeof( unsigned char ) );
                buff.len   = allocLen;
                buff.index = 0;
            }
            else
            {
                buff.data = (unsigned char *)realloc( (void *)buff.data, ( buff.len + allocLen ) * sizeof( unsigned char ) );
                buff.len += allocLen;
            }
            memcpy( ( void * )( buff.data + buff.index ), (void *)sendData, sendLength * sizeof( unsigned char ) );
            buff.index = buff.len;
            assert( buff.index % packetSize == 0 );
        }
        if ( ( rcvData != NULL ) && ( *rcvLength > 0 ) )
            // if caller is expecting to receive something, tell them it isn't going to happen with buffering on.
            *rcvLength = 0;
        return USB_SUCCESS;
    }
    else
    {
        Open(); // try to open the USB endpoint

        if ( ( epOut == INVALID_HANDLE_VALUE ) || ( epIn == INVALID_HANDLE_VALUE ) )
        {
            GetErr().SimpleMsg( XSErrorMinor, "USB endpoint not open." );
            // Close();    // close anything that might have been opened
            return USB_FAILURE;
        }

        if ( ( sendData != NULL ) && ( sendLength > 0 ) )
            if ( !MPUSBWrite( epOut, sendData, sendLength, &sentDataLength, USB_TIMEOUT ) )
            {
                TCHAR szBuf[80];
                LPVOID lpMsgBuf;
                DWORD dw = GetLastError();

                FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    dw,
                    MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                    (LPTSTR)&lpMsgBuf,
                    0, NULL );

                wsprintf( szBuf,
                          "failed with error %d: %s",
                          dw, lpMsgBuf );

                TCHAR errMsg[256];
                sprintf( errMsg, "MPUSBWrite() failed: %s", lpMsgBuf );
                string msg( errMsg );
                GetErr().SimpleMsg( XSErrorMinor, msg );
                // Close();    // always close the USB endpoint after using it to allow other apps to open it
                return USB_FAILURE;
            }

        if ( rcvData != NULL )
        {
            unsigned long expectedRcvLength = *rcvLength;
            if ( expectedRcvLength > 0 )
            {
                if ( MPUSBRead( epIn, rcvData, expectedRcvLength, rcvLength, USB_TIMEOUT ) )
                {
                    if ( *rcvLength == expectedRcvLength )
                    {
                        if ( ( sendData == NULL ) || ( sendLength == 0 ) || !checkFirstByte )
//							Close();	// always close the USB endpoint after using it to allow other apps to open it
                            return USB_SUCCESS;
                        else if ( rcvData[0] == sendData[0] )
//							Close();	// always close the USB endpoint after using it to allow other apps to open it
                            return USB_SUCCESS;
                        else
                        {
                            string msg( "Command byte in received USB packet does not match." );
                            GetErr().SimpleMsg( XSErrorMinor, msg );
                            // Close();    // always close the USB endpoint after using it to allow other apps to open it
                            return USB_FAILURE;
                        }
                    }
                    else
                    {
                        string msg( "Received USB packet is too short." );
                        GetErr().SimpleMsg( XSErrorMinor, msg );
                        // Close();    // always close the USB endpoint after using it to allow other apps to open it
                        return USB_FAILURE;
                    }
                }
                else
                {
                    TCHAR szBuf[80];
                    LPVOID lpMsgBuf;
                    DWORD dw = GetLastError();

                    FormatMessage(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER
                        | FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        dw,
                        MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                        (LPTSTR)&lpMsgBuf,
                        0, NULL );

                    wsprintf( szBuf,
                              "failed with error %d: %s",
                              dw, lpMsgBuf );

                    TCHAR errMsg[256];
                    sprintf( errMsg, "MPUSBRead() failed: %s", lpMsgBuf );
                    string msg( errMsg );
                    GetErr().SimpleMsg( XSErrorMinor, msg );
                    // Close();    // always close the USB endpoint after using it to allow other apps to open it
                    return USB_FAILURE;
                }
            }
        }

//		Close();	// always close the USB endpoint after using it to allow other apps to open it
        return USB_SUCCESS;
    }
} // SendRcvPacket



/// Get information from the USB peripheral and store it in the USB port object.
///\return USB_SUCCESS or USB_FAILURE
int USBPort::GetInfo( void )
{
    // send the command the makes the USB device return its info
    unsigned char infoCmd[USB_MAX_PACKET_SIZE] = { INFO_CMD };
    unsigned char infoRcvd[USB_MAX_PACKET_SIZE];
    unsigned long numBytesReceived             = USB_MAX_PACKET_SIZE;
    if ( SendRcvPacket( infoCmd, sizeof( infoCmd ), infoRcvd, &numBytesReceived, true ) == USB_FAILURE )
    {
        XSError err( cerr );
        string msg( "failed to get USB device info" );
        err.SimpleMsg( XSErrorMinor, msg );
        return USB_FAILURE;
    }

    // extract the information from the packet and store it
    strcpy( info, (char *)infoRcvd + 1 );

    return USB_SUCCESS;
}



/// Print the USB peripheral information.
///\return reference to the output stream
ostream &USBPort::ReportInfo( ostream &os ) ///< output stream that receives the information report
{
    os << "Instance of USBJTAG device : " << instance << endl;
    os << "  Info: " << info << endl;
    return os;
}



/// Set the level of the FPGA PROG# pin.
///\return USB_SUCCESS or USB_FAILURE
int USBPort::SetPROG( unsigned int b ) ///< level for the PROG# pin (1 or 0)
{
    unsigned char progCmd [] = { PROG_CMD, 0 };
    progCmd[1] = b;
    if ( SendRcvPacket( progCmd, sizeof( progCmd ), NULL, NULL, false ) == USB_FAILURE )
    {
        XSError err( cerr );
        string msg( "failed to set FPGA PROG# pin" );
        err.SimpleMsg( XSErrorMinor, msg );
        return USB_FAILURE;
    }
    return USB_SUCCESS;
}



/// Enable/disable the flash chip.
///\return USB_SUCCESS or USB_FAILURE
int USBPort::SetFlashEnable( unsigned int b ) ///< 1 to enable, 0 to disable
{
    unsigned char flashEnableCmd[64] = { FLASH_ONOFF_CMD, 0 };
    unsigned char rtnPacket[64];
    unsigned long rtnPacketSize      = 2;
    flashEnableCmd[1] = b;
    if ( SendRcvPacket( flashEnableCmd, (unsigned long)sizeof( flashEnableCmd ), rtnPacket, &rtnPacketSize, false ) == USB_FAILURE )
    {
        XSError err( cerr );
        string msg( "failed to enable/disable the flash" );
        err.SimpleMsg( XSErrorMinor, msg );
        return USB_FAILURE;
    }
    return USB_SUCCESS;
}



/// Load the DLL containing API subroutines that interface to the Microchip USB driver.
///\return USB_SUCCESS or USB_FAILURE
int LoadMPUSBAPIDLL( void )
{
    if ( mpusbapi_handle != NULL )
        return USB_SUCCESS;

#ifdef USE_MCHPUSB
    if ( ( mpusbapi_handle = LoadLibrary( "mpusbapi" ) ) == NULL )
    {
        XSError err( cerr );
        string msg( "Unable to load mpusbapi.dll." );
        err.SimpleMsg( XSErrorFatal, msg );
    }

    MPUSBGetDLLVersion  = ( DWORD( * ) ( void ) )\
                          GetProcAddress( mpusbapi_handle, "_MPUSBGetDLLVersion" );
    MPUSBGetDeviceCount = ( DWORD( * ) ( PCHAR ) )\
                          GetProcAddress( mpusbapi_handle, "_MPUSBGetDeviceCount" );
    MPUSBOpen           = ( HANDLE( * ) ( DWORD, PCHAR, PCHAR, DWORD, DWORD ) )\
                          GetProcAddress( mpusbapi_handle, "_MPUSBOpen" );
    MPUSBWrite          = ( DWORD( * ) ( HANDLE, PVOID, DWORD, PDWORD, DWORD ) )\
                          GetProcAddress( mpusbapi_handle, "_MPUSBWrite" );
    MPUSBRead           = ( DWORD( * ) ( HANDLE, PVOID, DWORD, PDWORD, DWORD ) )\
                          GetProcAddress( mpusbapi_handle, "_MPUSBRead" );
    MPUSBReadInt        = ( DWORD( * ) ( HANDLE, PVOID, DWORD, PDWORD, DWORD ) )\
                          GetProcAddress( mpusbapi_handle, "_MPUSBReadInt" );
    MPUSBClose          = ( BOOL( * ) ( HANDLE ) )
                          GetProcAddress( mpusbapi_handle, "_MPUSBClose" );
#endif

	mpusbapi_handle = (struct HINSTANCE__ *)1;

    if ( GetXSTOOLSParameter( "USBDRIVER" ) == "LIBUSB" )
    {
        libusb_init();
        MPUSBGetDLLVersion  = LIBUSBGetDLLVersion;
        MPUSBGetDeviceCount = LIBUSBGetDeviceCount;
        MPUSBOpen           = LIBUSBOpen;
        MPUSBWrite          = LIBUSBWrite;
        MPUSBRead           = LIBUSBRead;
        MPUSBReadInt        = LIBUSBReadInt;
        MPUSBClose          = LIBUSBClose;
    }
    else // WINUSB is being used
    {
        MPUSBGetDLLVersion  = WinUsbGetDLLVersion;
        MPUSBGetDeviceCount = WinUsbGetDeviceCount;
        MPUSBOpen           = WinUsbOpen;
        MPUSBWrite          = WinUsbWrite;
        MPUSBRead           = WinUsbRead;
        MPUSBReadInt        = WinUsbReadInt;
        MPUSBClose          = WinUsbClose;
    }

    if ( ( MPUSBGetDeviceCount == NULL ) || ( MPUSBOpen == NULL )
        || ( MPUSBWrite == NULL ) || ( MPUSBRead == NULL )
        || ( MPUSBClose == NULL ) || ( MPUSBGetDLLVersion == NULL )
        || ( MPUSBReadInt == NULL ) )
    {
        XSError err( cerr );
        string msg( "Failed to find all needed procedures in mpusbapi.dll." );
        err.SimpleMsg( XSErrorFatal, msg );
    }

    return USB_SUCCESS;
} // LoadMPUSBAPIDLL



/// Get the number of XSUSB devices currently plugged into USB ports
///\return the number of XSUSB devices
int GetUSBPortCount( void )
{
    LoadMPUSBAPIDLL();
    return MPUSBGetDeviceCount( mpusbVidPid );
}
