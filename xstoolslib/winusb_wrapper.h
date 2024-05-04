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


#ifndef WINUSB_WRAPPER_H
#define WINUSB_WRAPPER_H

/**
   Wrap to MPUSBAPI.

   This wrapper makes the winusb API subroutines look like the Microchip USB API.

 */

DWORD WinUsbGetDLLVersion( void );

DWORD WinUsbGetDeviceCount( PCHAR pVID_PID );

HANDLE WinUsbOpen( DWORD instance,
                   PCHAR pVID_PID,
                   PCHAR pEP,
                   DWORD dwDir,
                   DWORD dwReserved );

DWORD WinUsbRead( HANDLE handle,
                  PVOID pData,
                  DWORD dwLen,
                  PDWORD pLength,
                  DWORD dwMilliseconds );

DWORD WinUsbWrite( HANDLE handle,
                   PVOID pData,
                   DWORD dwLen,
                   PDWORD pLength,
                   DWORD dwMilliseconds );

DWORD WinUsbReadInt( HANDLE handle,
                     PVOID pData,
                     DWORD dwLen,
                     PDWORD pLength,
                     DWORD dwMilliseconds );

BOOL WinUsbClose( HANDLE handle );

#endif
