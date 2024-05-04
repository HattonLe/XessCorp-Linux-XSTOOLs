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

#include "mpusbapi_wrapper.h"

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

#ifdef USE_LIBUSB

// libusb stuff

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



#endif


///////////////////////////////////////////////////////////////
// Subroutines to wrap WINUSB API so it looks like mpusbapi
///////////////////////////////////////////////////////////////

#ifdef USE_WINUSB

// Windows WINUSB driver stuff
#include <strsafe.h>
#include <Usb100.h>
#include <WinUsb.h>
#include <SETUPAPI.h>
#define MAX_DEVPATH_LENGTH 256


// global unique ID for XSUSB interface through WINUSB driver
// {19326627-91F6-49c8-9E9F-58B540B79DF2}
static GUID XSUSBWinUsbGuid
    = { 0x19326627, 0x91f6, 0x49c8, { 0x9e, 0x9f, 0x58, 0xb5, 0x40, 0xb7, 0x9d, 0xf2 } };

// array of "extended" handles that contain the device and WINUSB handles and the endpoint pipes.
#define MAX_XSUSB_HANDLES 10
#define MAX_ENDPTS 256
typedef struct
{
    HANDLE devHandle;               // device handle
    HANDLE winUsbHandle;            // WINUSB handle
    UCHAR  pipeId[MAX_ENDPTS];      // endpoint pipe IDs indexed by endpoint number
} ExtendedWinUsbHandle;
static ExtendedWinUsbHandle xsusbHandle[MAX_XSUSB_HANDLES];
static BOOL not_initialized = TRUE; // initialize the extended handles array if this is true


/// Get the path to a device given its interface GUID and device instance.
static BOOL WinUsbGetDevicePath( LPGUID InterfaceGuid, ///< interface GUID
                                 DWORD instance,       ///< device instance
                                 PCHAR DevicePath,     ///< pointer to buffer to hold the path
                                 size_t BufLen )       ///< length of buffer
{
    BOOL bResult                                = FALSE;
    HDEVINFO deviceInfo;
    SP_DEVICE_INTERFACE_DATA interfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA detailData = NULL;
    ULONG length;
    ULONG requiredLength                        = 0;
    HRESULT hr;

    // get info about the device
    deviceInfo = SetupDiGetClassDevs( InterfaceGuid, NULL, NULL,
                                      DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );
    if ( deviceInfo == INVALID_HANDLE_VALUE )
        return FALSE;

    // iterate until the requested instance of the interface is found
    interfaceData.cbSize = sizeof( SP_DEVICE_INTERFACE_DATA );
    for ( DWORD inst = 0; inst <= instance; inst++ )
    {
        bResult = SetupDiEnumDeviceInterfaces( deviceInfo, NULL, InterfaceGuid, inst, &interfaceData );

        // exit if the requested instance of the interface is not found
        if ( bResult == FALSE )
        {
            SetupDiDestroyDeviceInfoList( deviceInfo );
            return FALSE;
        }
    }

    // get the size of the device path.  this is supposed to fail, but it gives length of path.
    SetupDiGetDeviceInterfaceDetail( deviceInfo, &interfaceData, NULL, 0, &requiredLength, NULL );
    if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
    {
        SetupDiDestroyDeviceInfoList( deviceInfo );
        return FALSE;
    }

    // allocate memory to store the device path
    detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)
                 LocalAlloc( LMEM_FIXED, requiredLength );
    if ( NULL == detailData )
    {
        SetupDiDestroyDeviceInfoList( deviceInfo );
        return FALSE;
    }

    // get the device path
    detailData->cbSize = sizeof( SP_DEVICE_INTERFACE_DETAIL_DATA );
    length             = requiredLength;
    bResult            = SetupDiGetDeviceInterfaceDetail( deviceInfo, &interfaceData,
                                                          detailData, length, &requiredLength, NULL );
    if ( FALSE == bResult )
    {
        SetupDiDestroyDeviceInfoList( deviceInfo );
        LocalFree( detailData );
        return FALSE;
    }

    // copy the device path to the output string
    hr = StringCchCopy( DevicePath, BufLen, detailData->DevicePath );

    // free any allocated memory
    SetupDiDestroyDeviceInfoList( deviceInfo );
    LocalFree( detailData );

    if ( SUCCEEDED( hr ) )
        return TRUE;
    else
        return FALSE;
} // WinUsbGetDevicePath



/// Open a device with the given interface GUID and instance.
///\return Handle to the device or INVALID_HANDLE_VALUE.
static HANDLE WinUsbOpenDevice( LPGUID InterfaceGuid,  ///< interface GUID
                                DWORD instance )       ///< device instance
{
    HANDLE devHandle = NULL;
    char devicePath[MAX_DEVPATH_LENGTH];

    if ( FALSE == WinUsbGetDevicePath( InterfaceGuid, instance,
                                      devicePath, sizeof( devicePath ) ) )
        return INVALID_HANDLE_VALUE;

    devHandle = CreateFile( devicePath,
                            GENERIC_WRITE | GENERIC_READ,
                            FILE_SHARE_WRITE | FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                            NULL );

    if ( INVALID_HANDLE_VALUE == devHandle )
        return INVALID_HANDLE_VALUE;

    return devHandle;
}



/// Close a device with the given handle.
///\return TRUE or FALSE depending upon whether the device was closed or not.
static BOOL WinUsbCloseDevice( HANDLE devHandle ) ///< device handle returned by WinUsbOpenDevice
{
    // already closed devices return TRUE by default
    if ( ( INVALID_HANDLE_VALUE == devHandle ) || ( NULL == devHandle ) )
        return TRUE;
    return CloseHandle( devHandle ); // close the device and return the result
}



/// Claim the USB device for I/O operations.
///\return TRUE if the device is claimed, FALSE if not.
static BOOL WinUsbClaim( ExtendedWinUsbHandle *extHndl ) ///< extended handle for USB device
{
    if ( INVALID_HANDLE_VALUE == extHndl->devHandle )
        return FALSE; // can't claim a closed USB device

    if ( INVALID_HANDLE_VALUE != extHndl->winUsbHandle )
        return FALSE; // somebody already claimed this device

     // device has not been claimed, yet, so try to claim it
    if ( !WinUsb_Initialize( extHndl->devHandle, &extHndl->winUsbHandle ) )
    {
        // couldn't claim it, so return failure
        extHndl->winUsbHandle = INVALID_HANDLE_VALUE;
        return FALSE;
    }

    return TRUE;     // claimed it, so return success
}



/// Release the USB device so someone else can use it
///\return TRUE if the device is released, FALSE if not.
static BOOL WinUsbRelease( ExtendedWinUsbHandle *extHndl ) ///< extended handle for USB device
{
    if ( INVALID_HANDLE_VALUE == extHndl->devHandle )
    {
        if ( INVALID_HANDLE_VALUE == extHndl->winUsbHandle )
            return TRUE;
        else
            return FALSE;
    }
    if ( extHndl->winUsbHandle != INVALID_HANDLE_VALUE )
    {
        if ( !WinUsb_Free( extHndl->winUsbHandle ) )
            return FALSE;
        extHndl->winUsbHandle = INVALID_HANDLE_VALUE;
    }
    return TRUE;
}



/// Get the version of the WINUSB DLL.
///\return Always returns 0.
DWORD WinUsbGetDLLVersion( void )
{
    return 0;
}



/// Get the number of WINUSB devices with the given USB vendor and product IDs.
///\return Number of devices found.
DWORD WinUsbGetDeviceCount( PCHAR pVID_PID ) ///< USB vendor and product IDs
{
    unsigned vid, pid;

    if ( !ExtractVIDPID( pVID_PID, &vid, &pid ) )
        return 0;

    LPGUID InterfaceGuid = &XSUSBWinUsbGuid;

    // get info on all the attached devices with this GUID
    HDEVINFO deviceInfo  = SetupDiGetClassDevs( InterfaceGuid, NULL, NULL,
                                                DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );
    if ( INVALID_HANDLE_VALUE == deviceInfo )
        return 0;

    // count up the attached devices
    SP_DEVICE_INTERFACE_DATA interfaceData;
    interfaceData.cbSize = sizeof( SP_DEVICE_INTERFACE_DATA );
    for ( DWORD devCnt = 0; ; devCnt++ )
    {
        if ( FALSE == SetupDiEnumDeviceInterfaces( deviceInfo, NULL, InterfaceGuid, devCnt, &interfaceData ) )
        {
            break; // release the device information structure
        }
    }
    SetupDiDestroyDeviceInfoList( deviceInfo );

    // return the number of devices found
    return devCnt;
} // WinUsbGetDeviceCount



/// Open the requested instance of a WINUSB device with the given USB vendor and product IDs.
///\return Extended handle to the USB device or INVALID_HANDLE_VALUE if it couldn't be opened.
HANDLE WinUsbOpen( DWORD instance,    ///< instance of device
                   PCHAR pVID_PID,    ///< USB vendor and product IDs
                   PCHAR pEP,         ///< endpoint with format "\\MCHP_EP%d"
                   DWORD dwDir,       ///< I/O direction
                   DWORD dwReserved ) ///< reserved
{
    if ( instance >= MAX_XSUSB_HANDLES )
    {
        return INVALID_HANDLE_VALUE; // out-of-range of allowable number of USB devices
    }
    // initialize extended WINUSB handles upon first entry
    if ( not_initialized )
    {
        for ( int i = 0; i < MAX_XSUSB_HANDLES; i++ )
        {
            xsusbHandle[i].devHandle    = INVALID_HANDLE_VALUE;
            xsusbHandle[i].winUsbHandle = INVALID_HANDLE_VALUE;
        }
        not_initialized = FALSE;
    }

    unsigned vid, pid;
    if ( !ExtractVIDPID( pVID_PID, &vid, &pid ) )
        return MPUSB_FAIL;

    ExtendedWinUsbHandle *xsusb = &xsusbHandle[instance];

    // open USB device if it hasn't been opened previously
    if ( xsusb->devHandle == INVALID_HANDLE_VALUE )
    {
        xsusb->devHandle = WinUsbOpenDevice( &XSUSBWinUsbGuid, instance );
        if ( INVALID_HANDLE_VALUE == xsusb->devHandle )
            return INVALID_HANDLE_VALUE;
    }

    // initialize WINUSB for this device if it hasn't been done previously
    if ( !WinUsbClaim( xsusb ) )
        return INVALID_HANDLE_VALUE;

    // get interface descriptor with the endpoint information
    USB_INTERFACE_DESCRIPTOR ifaceDescriptor;
    if ( !WinUsb_QueryInterfaceSettings( xsusb->winUsbHandle, 0, &ifaceDescriptor ) )
    {
        WinUsbRelease( xsusb );
        return INVALID_HANDLE_VALUE;
    }

    // get endpoint number and set the upper bit if its a READ pipe
    unsigned endpoint;
    sscanf( pEP, "\\MCHP_EP%d", &endpoint );
    if ( dwDir == MP_READ )
        endpoint |= 0x80;
    // look through the interface descriptor for the requested endpoint
    for ( int i = 0; i < ifaceDescriptor.bNumEndpoints; i++ )
    {
        WINUSB_PIPE_INFORMATION pipeInfo;
        if ( !WinUsb_QueryPipe( xsusb->winUsbHandle, 0, (UCHAR)i, &pipeInfo ) )
        {
            WinUsbRelease( xsusb );
            return INVALID_HANDLE_VALUE;
        }

        // found the endpoint!  Return a handle that encodes the index into
        // the array of xsusb handles and the endpoint
        if ( pipeInfo.PipeId == endpoint )
        {
            WinUsbRelease( xsusb );
            return ( void * )( ( instance << 8 ) | endpoint );
        }
    }

    // didn't find the endpoint.
    WinUsbRelease( xsusb );
    return INVALID_HANDLE_VALUE;
} // WinUsbOpen



/// Read from a USB device into a buffer.
///\return MPUSB_FAIL or MPUSB_SUCCESS.
DWORD WinUsbRead( HANDLE handle,          ///< extended handle to device
                  PVOID pData,            ///< pointer to buffer to hold data read from device
                  DWORD dwLen,            ///< requested number of bytes to read
                  PDWORD pLength,         ///< pointer to counter for actual number of bytes read
                  DWORD dwMilliseconds )  ///< timeout
{
    if ( INVALID_HANDLE_VALUE == handle )
        return MPUSB_FAIL;

    // get endpoint and instance from extended handle
    int endpoint                = (int)handle & 0xff;
    int instance                = ( (int)handle >> 8 ) & 0xff;

    // get pointer to data structure for extended handle
    ExtendedWinUsbHandle *xsusb = &xsusbHandle[instance];

    // claim the USB interface for I/O
    if ( !WinUsbClaim( xsusb ) )
    {
        return MPUSB_FAIL; // couldn't claim it, so return failure
    }
    // set timeout for I/O operation
    WinUsb_SetPipePolicy( xsusb->winUsbHandle, endpoint, PIPE_TRANSFER_TIMEOUT, sizeof( DWORD ), &dwMilliseconds );

    // read data from the USB device
    ULONG numBytesRead;
    BOOL result = WinUsb_ReadPipe( xsusb->winUsbHandle, endpoint, (PUCHAR)pData, dwLen, &numBytesRead, NULL );
    *pLength = numBytesRead;
    WinUsbRelease( xsusb ); // I/O is done, so release our claim on the USB device
    return result ? MPUSB_SUCCESS : MPUSB_FAIL;
}



/// Write to a USB device from a buffer.
///\return MPUSB_FAIL or MPUSB_SUCCESS.
DWORD WinUsbWrite( HANDLE handle,         ///< extended handle to device
                   PVOID pData,           ///< pointer to buffer holding data to write to device
                   DWORD dwLen,           ///< requested number of bytes to write
                   PDWORD pLength,        ///< pointer to counter for actual number of bytes written
                   DWORD dwMilliseconds ) ///< timeout
{
    if ( INVALID_HANDLE_VALUE == handle )
        return MPUSB_FAIL;

    // get endpoint and instance from extended handle
    int endpoint                = (int)handle & 0xff;
    int instance                = ( (int)handle >> 8 ) & 0xff;

    // get pointer to data structure for extended handle
    ExtendedWinUsbHandle *xsusb = &xsusbHandle[instance];

    // claim the USB interface for I/O
    if ( !WinUsbClaim( xsusb ) )
        return MPUSB_FAIL;

    // set timeout for I/O operation
    WinUsb_SetPipePolicy( xsusb->winUsbHandle, endpoint, PIPE_TRANSFER_TIMEOUT, sizeof( DWORD ), &dwMilliseconds );

    // write data to the USB device
    ULONG numBytesWritten;
    BOOL result = WinUsb_WritePipe( xsusb->winUsbHandle, endpoint, (PUCHAR)pData, dwLen, &numBytesWritten, NULL );
    *pLength = numBytesWritten;
    WinUsbRelease( xsusb ); // I/O is done, so release our claim on the USB device
    return result ? MPUSB_SUCCESS : MPUSB_FAIL;
}



/// Read an integer from the USB device.
///\return Always returns MPUSB_FAIL.
DWORD WinUsbReadInt( HANDLE handle,         ///< extended handle to device
                     PVOID pData,           ///< pointer to buffer to hold data read from device
                     DWORD dwLen,           ///< requested number of integers to read
                     PDWORD pLength,        ///< pointer to counter for actual number of integers read
                     DWORD dwMilliseconds ) ///< timeout
{
    return MPUSB_FAIL;
}



/// Close the WINUSB device with the given extended handle.
///\return MPUSB_FAIL or MPUSB_SUCCESS.
BOOL WinUsbClose( HANDLE handle ) ///< extended handle for the USB device
{
    if ( handle == INVALID_HANDLE_VALUE )
        return MPUSB_FAIL;

    int instance                = ( (int)handle >> 8 ) & 0xff;

    ExtendedWinUsbHandle *xsusb = &xsusbHandle[instance];

    WinUsbRelease( xsusb );
    WinUsbCloseDevice( xsusb->devHandle );
    xsusb->devHandle = INVALID_HANDLE_VALUE;

    return MPUSB_SUCCESS;
}



#endif
