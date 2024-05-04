# Microsoft Developer Studio Project File - Name="xstoolslib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=xstoolslib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "xstoolslib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "xstoolslib.mak" CFG="xstoolslib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "xstoolslib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "xstoolslib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "xstoolslib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\tvichw32" /I "..\xstoolslib" /I "..\dlportio" /I "..\uniio\uniio\include" /I "..\MCHPFSUSB\Pc\Mpusbapi\Dll\Borland_C" /I "..\markuplib" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "WIN32" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "xstoolslib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "..\tvichw32" /I "..\xstoolslib" /I "..\dlportio" /I "..\uniio\uniio\include" /I "..\MCHPFSUSB\Pc\Mpusbapi\Dll\Borland_C" /I "..\markuplib" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "WIN32" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "xstoolslib - Win32 Release"
# Name "xstoolslib - Win32 Debug"
# Begin Group "source"

# PROP Default_Filter "*.c;*.cpp"
# Begin Source File

SOURCE=.\akcdcprt.cpp
# End Source File
# Begin Source File

SOURCE=.\am29fprt.cpp
# End Source File
# Begin Source File

SOURCE=.\at17prt.cpp
# End Source File
# Begin Source File

SOURCE=.\at49fprt.cpp
# End Source File
# Begin Source File

SOURCE=.\bitstrm.cpp
# End Source File
# Begin Source File

SOURCE=.\cnfgport.cpp
# End Source File
# Begin Source File

SOURCE=.\f28port.cpp
# End Source File
# Begin Source File

SOURCE=.\flashprt.cpp
# End Source File
# Begin Source File

SOURCE=.\hex.cpp
# End Source File
# Begin Source File

SOURCE=.\hexrecrd.cpp
# End Source File
# Begin Source File

SOURCE=.\i2cport.cpp
# End Source File
# Begin Source File

SOURCE=.\i2cportjtag.cpp
# End Source File
# Begin Source File

SOURCE=.\i2cportlpt.cpp
# End Source File
# Begin Source File

SOURCE=.\jramprt.cpp
# End Source File
# Begin Source File

SOURCE=.\jtagport.cpp
# End Source File
# Begin Source File

SOURCE=.\libusb_wrapper.cpp
# End Source File
# Begin Source File

SOURCE=.\lptjtag.cpp
# End Source File
# Begin Source File

SOURCE=.\Markup.cpp
# End Source File
# Begin Source File

SOURCE=.\mchpport.cpp
# End Source File
# Begin Source File

SOURCE=.\osccyprt.cpp
# End Source File
# Begin Source File

SOURCE=.\oscport.cpp
# End Source File
# Begin Source File

SOURCE=.\pbusport.cpp
# End Source File
# Begin Source File

SOURCE=.\pport.cpp
# End Source File
# Begin Source File

SOURCE=.\precision_timer.cpp
# End Source File
# Begin Source File

SOURCE=.\progress.cpp
# End Source File
# Begin Source File

SOURCE=.\ramport.cpp
# End Source File
# Begin Source File

SOURCE=.\regiojtag.cpp
# End Source File
# Begin Source File

SOURCE=.\saa711x.cpp
# End Source File
# Begin Source File

SOURCE=.\testport.cpp
# End Source File
# Begin Source File

SOURCE=.\usbjtag.cpp
# End Source File
# Begin Source File

SOURCE=.\usbport.cpp
# End Source File
# Begin Source File

SOURCE=.\utils.cpp
# End Source File
# Begin Source File

SOURCE=.\winusb_wrapper.cpp
# End Source File
# Begin Source File

SOURCE=.\xc2sprt.cpp
# End Source File
# Begin Source File

SOURCE=.\xc3sprt.cpp
# End Source File
# Begin Source File

SOURCE=.\xc4kprt.cpp
# End Source File
# Begin Source File

SOURCE=.\xc95kprt.cpp
# End Source File
# Begin Source File

SOURCE=.\xcbsdr.cpp
# End Source File
# Begin Source File

SOURCE=.\xcvprt.cpp
# End Source File
# Begin Source File

SOURCE=.\xs40brd.cpp
# End Source File
# Begin Source File

SOURCE=.\xs95brd.cpp
# End Source File
# Begin Source File

SOURCE=.\xsa200brd.cpp
# End Source File
# Begin Source File

SOURCE=.\xsa3sbrd.cpp
# End Source File
# Begin Source File

SOURCE=.\xsaboard.cpp
# End Source File
# Begin Source File

SOURCE=.\xsbboard.cpp
# End Source File
# Begin Source File

SOURCE=.\xsboard.cpp
# End Source File
# Begin Source File

SOURCE=.\xscboard.cpp
# End Source File
# Begin Source File

SOURCE=.\xserror.cpp
# End Source File
# Begin Source File

SOURCE=.\xsjtagbrd.cpp
# End Source File
# Begin Source File

SOURCE=.\xsnullboard.cpp
# End Source File
# Begin Source File

SOURCE=.\xsvboard.cpp
# End Source File
# Begin Source File

SOURCE=.\xulabrd.cpp
# End Source File
# End Group
# Begin Group "include"

# PROP Default_Filter "*.h"
# End Group
# End Target
# End Project
