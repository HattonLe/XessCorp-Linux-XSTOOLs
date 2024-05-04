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

	©1997-2011 - X Engineering Software Systems Corp.
----------------------------------------------------------------------------------*/

#include <string>
#include <fstream>
#include <cassert>
#include <ctime>
#include <cstdarg>
#include <wtypes.h>
using namespace std;

#include "libusb_wrapper.h"

// Microchip USB stuff
#include "_mpusbapi.h"

// USB transfer timeout (in milliseconds)
#define TIMEOUT 5000



static bool ExtractVIDPID( PCHAR VID_PID, unsigned *vid, unsigned *pid )
{
    return sscanf( (char *)VID_PID, "vid_%x&pid_%x", vid, pid ) == 2;
}



///////////////////////////////////////////////////////////////
// Subroutines to wrap libusb API so it looks like mpusbapi
///////////////////////////////////////////////////////////////

#include "../libusb-win32/include/usb.h"

#define LIBUSB_INFINITE TIMEOUT    // timeout in milliseconds

static usb_dev_handle *dev[MAX_NUM_MPUSB_DEV] = { NULL, };



/// Initialize libusb.
void libusb_init( void )
{
    usb_init();
}



/// Get the version of the libusb DLL.
///\return Always returns 0.
DWORD LIBUSBGetDLLVersion( void )
{
    return 0;
}



/// Get the number of libusb devices with the given USB vendor and product IDs.
///\return Number of devices found.
DWORD LIBUSBGetDeviceCount( PCHAR pVID_PID ) ///< USB vendor and product IDs
{
    unsigned vid, pid;

    if ( !ExtractVIDPID( pVID_PID, &vid, &pid ) )
        return 0;

    usb_find_busses();
    usb_find_devices();

    struct usb_bus *bus;
    struct usb_device *dev;
    DWORD dev_cnt = 0;

    for ( bus = usb_get_busses(); bus; bus = bus->next )
        for ( dev = bus->devices; dev; dev = dev->next )
            if ( ( dev->descriptor.idVendor == vid )
                && ( dev->descriptor.idProduct == pid ) )
                dev_cnt++;
    return dev_cnt;
}



// Extended handle that holds the libusb device handle and the endpoint.
typedef struct
{
    struct usb_dev_handle *dev_handle;
    int                   endpoint;
} USB_CHANNEL;


/// Open the requested instance of a libusb device with the given USB vendor and product IDs.
///\return Extended handle to the USB device or INVALID_HANDLE_VALUE if it couldn't be opened.
HANDLE LIBUSBOpen( DWORD instance,    ///< instance of device
                   PCHAR pVID_PID,    ///< USB vendor and product IDs
                   PCHAR pEP,         ///< endpoint with format "\\MCHP_EP%d"
                   DWORD dwDir,       ///< I/O direction
                   DWORD dwReserved ) ///< reserved
{
    unsigned vid, pid;

    if ( !ExtractVIDPID( pVID_PID, &vid, &pid ) )
        return INVALID_HANDLE_VALUE;

    usb_find_busses();
    usb_find_devices();

    struct usb_bus *bus;
    struct usb_device *dev;
    DWORD dev_cnt = 0;

    for ( bus = usb_get_busses(); bus; bus = bus->next )
        for ( dev = bus->devices; dev; dev = dev->next )
            if ( ( dev->descriptor.idVendor == vid )
                && ( dev->descriptor.idProduct == pid ) )
            {
                if ( dev_cnt == instance )
                {
                    USB_CHANNEL *usb_channel = new USB_CHANNEL;
                    if ( !( usb_channel->dev_handle = usb_open( dev ) ) )
                        return INVALID_HANDLE_VALUE;

                    sscanf( pEP, "\\MCHP_EP%d", &( usb_channel->endpoint ) );
                    if ( dwDir == MP_READ )
                        usb_channel->endpoint += 0x80;
                    if ( usb_set_configuration( usb_channel->dev_handle, 1 ) < 0 )
                    {
                        usb_close( usb_channel->dev_handle );
                        return INVALID_HANDLE_VALUE;
                    }
                    return (HANDLE)usb_channel;
                }
                dev_cnt++;
            }

    return INVALID_HANDLE_VALUE;
} // LIBUSBOpen



/// Read from a USB device into a buffer.
///\return MPUSB_FAIL or MPUSB_SUCCESS.
DWORD LIBUSBRead( HANDLE handle,          ///< extended handle to device
                  PVOID pData,            ///< pointer to buffer to hold data read from device
                  DWORD dwLen,            ///< requested number of bytes to read
                  PDWORD pLength,         ///< pointer to counter for actual number of bytes read
                  DWORD dwMilliseconds )  ///< timeout
{
    assert( handle != INVALID_HANDLE_VALUE );
    USB_CHANNEL *usb_channel = (USB_CHANNEL *)handle;

    if ( usb_claim_interface( usb_channel->dev_handle, 0 ) < 0 )
        return MPUSB_FAIL;

    dwMilliseconds = dwMilliseconds == INFINITE ? LIBUSB_INFINITE : dwMilliseconds;

    // The timeout is set to 10000 because setting it to a lower value causes problems with
    // the expected and actual received packet lengths are different.
    *pLength       = usb_bulk_read( usb_channel->dev_handle, usb_channel->endpoint, (char *)pData, dwLen, 10000 );
    usb_release_interface( usb_channel->dev_handle, 0 );

    return MPUSB_SUCCESS;
}



/// Write to a USB device from a buffer.
///\return MPUSB_FAIL or MPUSB_SUCCESS.
DWORD LIBUSBWrite( HANDLE handle,         ///< extended handle to device
                   PVOID pData,           ///< pointer to buffer holding data to write to device
                   DWORD dwLen,           ///< requested number of bytes to write
                   PDWORD pLength,        ///< pointer to counter for actual number of bytes written
                   DWORD dwMilliseconds ) ///< timeout
{
    assert( handle != INVALID_HANDLE_VALUE );
    USB_CHANNEL *usb_channel = (USB_CHANNEL *)handle;

    if ( usb_claim_interface( usb_channel->dev_handle, 0 ) < 0 )
        return MPUSB_FAIL;

    dwMilliseconds = dwMilliseconds == INFINITE ? LIBUSB_INFINITE : dwMilliseconds;

    *pLength       = usb_bulk_write( usb_channel->dev_handle, usb_channel->endpoint, (char *)pData, dwLen, dwMilliseconds );
    usb_release_interface( usb_channel->dev_handle, 0 );

    return MPUSB_SUCCESS;
}



/// Read an integer from the USB device.
///\return Always returns MPUSB_FAIL.
DWORD LIBUSBReadInt( HANDLE handle,         ///< extended handle to device
                     PVOID pData,           ///< pointer to buffer to hold data read from device
                     DWORD dwLen,           ///< requested number of integers to read
                     PDWORD pLength,        ///< pointer to counter for actual number of integers read
                     DWORD dwMilliseconds ) ///< timeout
{
    return MPUSB_FAIL;
}



/// Close the libusb device with the given extended handle.
///\return MPUSB_FAIL or MPUSB_SUCCESS.
BOOL LIBUSBClose( HANDLE handle ) ///< extended handle for the USB device
{
    if ( handle == INVALID_HANDLE_VALUE )
        return 1;

    USB_CHANNEL *usb_channel = (USB_CHANNEL *)handle;
    int result               = usb_close( usb_channel->dev_handle );
    delete usb_channel;

    return result < 0 ? MPUSB_FAIL : MPUSB_SUCCESS;
}
