# Microsoft Developer Studio Generated NMAKE File, Based on xstoolslib.dsp
!IF "$(CFG)" == ""
CFG=xstoolslib - Win32 Debug
!MESSAGE No configuration specified. Defaulting to xstoolslib - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "xstoolslib - Win32 Release" && "$(CFG)" != "xstoolslib - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "xstoolslib - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\xstoolslib.lib"

!ELSE 

ALL : "iolib - Win32 Release" "$(OUTDIR)\xstoolslib.lib"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"iolib - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\akcdcprt.obj"
	-@erase "$(INTDIR)\am29fprt.obj"
	-@erase "$(INTDIR)\at17prt.obj"
	-@erase "$(INTDIR)\at49fprt.obj"
	-@erase "$(INTDIR)\bitstrm.obj"
	-@erase "$(INTDIR)\cnfgport.obj"
	-@erase "$(INTDIR)\f28port.obj"
	-@erase "$(INTDIR)\flashprt.obj"
	-@erase "$(INTDIR)\hex.obj"
	-@erase "$(INTDIR)\hexrecrd.obj"
	-@erase "$(INTDIR)\i2cport.obj"
	-@erase "$(INTDIR)\i2cportjtag.obj"
	-@erase "$(INTDIR)\i2cportlpt.obj"
	-@erase "$(INTDIR)\jramprt.obj"
	-@erase "$(INTDIR)\jtagport.obj"
	-@erase "$(INTDIR)\libusb_wrapper.obj"
	-@erase "$(INTDIR)\lptjtag.obj"
	-@erase "$(INTDIR)\Markup.obj"
	-@erase "$(INTDIR)\mchpport.obj"
	-@erase "$(INTDIR)\osccyprt.obj"
	-@erase "$(INTDIR)\oscport.obj"
	-@erase "$(INTDIR)\pbusport.obj"
	-@erase "$(INTDIR)\pport.obj"
	-@erase "$(INTDIR)\precision_timer.obj"
	-@erase "$(INTDIR)\progress.obj"
	-@erase "$(INTDIR)\ramport.obj"
	-@erase "$(INTDIR)\regiojtag.obj"
	-@erase "$(INTDIR)\saa711x.obj"
	-@erase "$(INTDIR)\testport.obj"
	-@erase "$(INTDIR)\usbjtag.obj"
	-@erase "$(INTDIR)\usbport.obj"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\winusb_wrapper.obj"
	-@erase "$(INTDIR)\xc2sprt.obj"
	-@erase "$(INTDIR)\xc3sprt.obj"
	-@erase "$(INTDIR)\xc4kprt.obj"
	-@erase "$(INTDIR)\xc95kprt.obj"
	-@erase "$(INTDIR)\xcbsdr.obj"
	-@erase "$(INTDIR)\xcvprt.obj"
	-@erase "$(INTDIR)\xs40brd.obj"
	-@erase "$(INTDIR)\xs95brd.obj"
	-@erase "$(INTDIR)\xsa200brd.obj"
	-@erase "$(INTDIR)\xsa3sbrd.obj"
	-@erase "$(INTDIR)\xsaboard.obj"
	-@erase "$(INTDIR)\xsbboard.obj"
	-@erase "$(INTDIR)\xsboard.obj"
	-@erase "$(INTDIR)\xscboard.obj"
	-@erase "$(INTDIR)\xserror.obj"
	-@erase "$(INTDIR)\xsjtagbrd.obj"
	-@erase "$(INTDIR)\xsnullboard.obj"
	-@erase "$(INTDIR)\xsvboard.obj"
	-@erase "$(INTDIR)\xulabrd.obj"
	-@erase "$(OUTDIR)\xstoolslib.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\tvichw32" /I "..\xstoolslib" /I "..\dlportio" /I "..\uniio\uniio\include" /I "..\MCHPFSUSB\Pc\Mpusbapi\Dll\Borland_C" /I "..\markuplib" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "WIN32" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\xstoolslib.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\xstoolslib.lib" 
LIB32_OBJS= \
	"$(INTDIR)\akcdcprt.obj" \
	"$(INTDIR)\am29fprt.obj" \
	"$(INTDIR)\at17prt.obj" \
	"$(INTDIR)\at49fprt.obj" \
	"$(INTDIR)\bitstrm.obj" \
	"$(INTDIR)\cnfgport.obj" \
	"$(INTDIR)\f28port.obj" \
	"$(INTDIR)\flashprt.obj" \
	"$(INTDIR)\hex.obj" \
	"$(INTDIR)\hexrecrd.obj" \
	"$(INTDIR)\i2cport.obj" \
	"$(INTDIR)\i2cportjtag.obj" \
	"$(INTDIR)\i2cportlpt.obj" \
	"$(INTDIR)\jramprt.obj" \
	"$(INTDIR)\jtagport.obj" \
	"$(INTDIR)\lptjtag.obj" \
	"$(INTDIR)\Markup.obj" \
	"$(INTDIR)\mchpport.obj" \
	"$(INTDIR)\osccyprt.obj" \
	"$(INTDIR)\oscport.obj" \
	"$(INTDIR)\pbusport.obj" \
	"$(INTDIR)\pport.obj" \
	"$(INTDIR)\precision_timer.obj" \
	"$(INTDIR)\progress.obj" \
	"$(INTDIR)\ramport.obj" \
	"$(INTDIR)\regiojtag.obj" \
	"$(INTDIR)\saa711x.obj" \
	"$(INTDIR)\testport.obj" \
	"$(INTDIR)\usbjtag.obj" \
	"$(INTDIR)\usbport.obj" \
	"$(INTDIR)\utils.obj" \
	"$(INTDIR)\xc2sprt.obj" \
	"$(INTDIR)\xc3sprt.obj" \
	"$(INTDIR)\xc4kprt.obj" \
	"$(INTDIR)\xc95kprt.obj" \
	"$(INTDIR)\xcbsdr.obj" \
	"$(INTDIR)\xcvprt.obj" \
	"$(INTDIR)\xs40brd.obj" \
	"$(INTDIR)\xs95brd.obj" \
	"$(INTDIR)\xsa200brd.obj" \
	"$(INTDIR)\xsa3sbrd.obj" \
	"$(INTDIR)\xsaboard.obj" \
	"$(INTDIR)\xsbboard.obj" \
	"$(INTDIR)\xsboard.obj" \
	"$(INTDIR)\xscboard.obj" \
	"$(INTDIR)\xserror.obj" \
	"$(INTDIR)\xsjtagbrd.obj" \
	"$(INTDIR)\xsnullboard.obj" \
	"$(INTDIR)\xsvboard.obj" \
	"$(INTDIR)\xulabrd.obj" \
	"$(INTDIR)\winusb_wrapper.obj" \
	"$(INTDIR)\libusb_wrapper.obj" \
	"..\iolib\Release\iolib.lib"

"$(OUTDIR)\xstoolslib.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "xstoolslib - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\xstoolslib.lib"

!ELSE 

ALL : "iolib - Win32 Debug" "$(OUTDIR)\xstoolslib.lib"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"iolib - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\akcdcprt.obj"
	-@erase "$(INTDIR)\am29fprt.obj"
	-@erase "$(INTDIR)\at17prt.obj"
	-@erase "$(INTDIR)\at49fprt.obj"
	-@erase "$(INTDIR)\bitstrm.obj"
	-@erase "$(INTDIR)\cnfgport.obj"
	-@erase "$(INTDIR)\f28port.obj"
	-@erase "$(INTDIR)\flashprt.obj"
	-@erase "$(INTDIR)\hex.obj"
	-@erase "$(INTDIR)\hexrecrd.obj"
	-@erase "$(INTDIR)\i2cport.obj"
	-@erase "$(INTDIR)\i2cportjtag.obj"
	-@erase "$(INTDIR)\i2cportlpt.obj"
	-@erase "$(INTDIR)\jramprt.obj"
	-@erase "$(INTDIR)\jtagport.obj"
	-@erase "$(INTDIR)\libusb_wrapper.obj"
	-@erase "$(INTDIR)\lptjtag.obj"
	-@erase "$(INTDIR)\Markup.obj"
	-@erase "$(INTDIR)\mchpport.obj"
	-@erase "$(INTDIR)\osccyprt.obj"
	-@erase "$(INTDIR)\oscport.obj"
	-@erase "$(INTDIR)\pbusport.obj"
	-@erase "$(INTDIR)\pport.obj"
	-@erase "$(INTDIR)\precision_timer.obj"
	-@erase "$(INTDIR)\progress.obj"
	-@erase "$(INTDIR)\ramport.obj"
	-@erase "$(INTDIR)\regiojtag.obj"
	-@erase "$(INTDIR)\saa711x.obj"
	-@erase "$(INTDIR)\testport.obj"
	-@erase "$(INTDIR)\usbjtag.obj"
	-@erase "$(INTDIR)\usbport.obj"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\winusb_wrapper.obj"
	-@erase "$(INTDIR)\xc2sprt.obj"
	-@erase "$(INTDIR)\xc3sprt.obj"
	-@erase "$(INTDIR)\xc4kprt.obj"
	-@erase "$(INTDIR)\xc95kprt.obj"
	-@erase "$(INTDIR)\xcbsdr.obj"
	-@erase "$(INTDIR)\xcvprt.obj"
	-@erase "$(INTDIR)\xs40brd.obj"
	-@erase "$(INTDIR)\xs95brd.obj"
	-@erase "$(INTDIR)\xsa200brd.obj"
	-@erase "$(INTDIR)\xsa3sbrd.obj"
	-@erase "$(INTDIR)\xsaboard.obj"
	-@erase "$(INTDIR)\xsbboard.obj"
	-@erase "$(INTDIR)\xsboard.obj"
	-@erase "$(INTDIR)\xscboard.obj"
	-@erase "$(INTDIR)\xserror.obj"
	-@erase "$(INTDIR)\xsjtagbrd.obj"
	-@erase "$(INTDIR)\xsnullboard.obj"
	-@erase "$(INTDIR)\xsvboard.obj"
	-@erase "$(INTDIR)\xulabrd.obj"
	-@erase "$(OUTDIR)\xstoolslib.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /GX /Z7 /Od /I "..\tvichw32" /I "..\xstoolslib" /I "..\dlportio" /I "..\uniio\uniio\include" /I "..\MCHPFSUSB\Pc\Mpusbapi\Dll\Borland_C" /I "..\markuplib" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "WIN32" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\xstoolslib.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\xstoolslib.lib" 
LIB32_OBJS= \
	"$(INTDIR)\akcdcprt.obj" \
	"$(INTDIR)\am29fprt.obj" \
	"$(INTDIR)\at17prt.obj" \
	"$(INTDIR)\at49fprt.obj" \
	"$(INTDIR)\bitstrm.obj" \
	"$(INTDIR)\cnfgport.obj" \
	"$(INTDIR)\f28port.obj" \
	"$(INTDIR)\flashprt.obj" \
	"$(INTDIR)\hex.obj" \
	"$(INTDIR)\hexrecrd.obj" \
	"$(INTDIR)\i2cport.obj" \
	"$(INTDIR)\i2cportjtag.obj" \
	"$(INTDIR)\i2cportlpt.obj" \
	"$(INTDIR)\jramprt.obj" \
	"$(INTDIR)\jtagport.obj" \
	"$(INTDIR)\lptjtag.obj" \
	"$(INTDIR)\Markup.obj" \
	"$(INTDIR)\mchpport.obj" \
	"$(INTDIR)\osccyprt.obj" \
	"$(INTDIR)\oscport.obj" \
	"$(INTDIR)\pbusport.obj" \
	"$(INTDIR)\pport.obj" \
	"$(INTDIR)\precision_timer.obj" \
	"$(INTDIR)\progress.obj" \
	"$(INTDIR)\ramport.obj" \
	"$(INTDIR)\regiojtag.obj" \
	"$(INTDIR)\saa711x.obj" \
	"$(INTDIR)\testport.obj" \
	"$(INTDIR)\usbjtag.obj" \
	"$(INTDIR)\usbport.obj" \
	"$(INTDIR)\utils.obj" \
	"$(INTDIR)\xc2sprt.obj" \
	"$(INTDIR)\xc3sprt.obj" \
	"$(INTDIR)\xc4kprt.obj" \
	"$(INTDIR)\xc95kprt.obj" \
	"$(INTDIR)\xcbsdr.obj" \
	"$(INTDIR)\xcvprt.obj" \
	"$(INTDIR)\xs40brd.obj" \
	"$(INTDIR)\xs95brd.obj" \
	"$(INTDIR)\xsa200brd.obj" \
	"$(INTDIR)\xsa3sbrd.obj" \
	"$(INTDIR)\xsaboard.obj" \
	"$(INTDIR)\xsbboard.obj" \
	"$(INTDIR)\xsboard.obj" \
	"$(INTDIR)\xscboard.obj" \
	"$(INTDIR)\xserror.obj" \
	"$(INTDIR)\xsjtagbrd.obj" \
	"$(INTDIR)\xsnullboard.obj" \
	"$(INTDIR)\xsvboard.obj" \
	"$(INTDIR)\xulabrd.obj" \
	"$(INTDIR)\winusb_wrapper.obj" \
	"$(INTDIR)\libusb_wrapper.obj" \
	"..\iolib\Debug\iolib.lib"

"$(OUTDIR)\xstoolslib.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("xstoolslib.dep")
!INCLUDE "xstoolslib.dep"
!ELSE 
!MESSAGE Warning: cannot find "xstoolslib.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "xstoolslib - Win32 Release" || "$(CFG)" == "xstoolslib - Win32 Debug"
SOURCE=.\akcdcprt.cpp

"$(INTDIR)\akcdcprt.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\am29fprt.cpp

"$(INTDIR)\am29fprt.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\at17prt.cpp

"$(INTDIR)\at17prt.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\at49fprt.cpp

"$(INTDIR)\at49fprt.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\bitstrm.cpp

"$(INTDIR)\bitstrm.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cnfgport.cpp

"$(INTDIR)\cnfgport.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\f28port.cpp

"$(INTDIR)\f28port.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\flashprt.cpp

"$(INTDIR)\flashprt.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\hex.cpp

"$(INTDIR)\hex.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\hexrecrd.cpp

"$(INTDIR)\hexrecrd.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\i2cport.cpp

"$(INTDIR)\i2cport.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\i2cportjtag.cpp

"$(INTDIR)\i2cportjtag.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\i2cportlpt.cpp

"$(INTDIR)\i2cportlpt.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\jramprt.cpp

"$(INTDIR)\jramprt.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\jtagport.cpp

"$(INTDIR)\jtagport.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\libusb_wrapper.cpp

"$(INTDIR)\libusb_wrapper.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\lptjtag.cpp

"$(INTDIR)\lptjtag.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Markup.cpp

"$(INTDIR)\Markup.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mchpport.cpp

"$(INTDIR)\mchpport.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\osccyprt.cpp

"$(INTDIR)\osccyprt.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\oscport.cpp

"$(INTDIR)\oscport.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\pbusport.cpp

"$(INTDIR)\pbusport.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\pport.cpp

"$(INTDIR)\pport.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\precision_timer.cpp

"$(INTDIR)\precision_timer.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\progress.cpp

"$(INTDIR)\progress.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ramport.cpp

"$(INTDIR)\ramport.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\regiojtag.cpp

"$(INTDIR)\regiojtag.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\saa711x.cpp

"$(INTDIR)\saa711x.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\testport.cpp

"$(INTDIR)\testport.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\usbjtag.cpp

"$(INTDIR)\usbjtag.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\usbport.cpp

"$(INTDIR)\usbport.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\utils.cpp

"$(INTDIR)\utils.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\winusb_wrapper.cpp

"$(INTDIR)\winusb_wrapper.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xc2sprt.cpp

"$(INTDIR)\xc2sprt.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xc3sprt.cpp

"$(INTDIR)\xc3sprt.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xc4kprt.cpp

"$(INTDIR)\xc4kprt.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xc95kprt.cpp

"$(INTDIR)\xc95kprt.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xcbsdr.cpp

"$(INTDIR)\xcbsdr.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xcvprt.cpp

"$(INTDIR)\xcvprt.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xs40brd.cpp

"$(INTDIR)\xs40brd.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xs95brd.cpp

"$(INTDIR)\xs95brd.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xsa200brd.cpp

"$(INTDIR)\xsa200brd.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xsa3sbrd.cpp

"$(INTDIR)\xsa3sbrd.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xsaboard.cpp

"$(INTDIR)\xsaboard.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xsbboard.cpp

"$(INTDIR)\xsbboard.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xsboard.cpp

"$(INTDIR)\xsboard.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xscboard.cpp

"$(INTDIR)\xscboard.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xserror.cpp

"$(INTDIR)\xserror.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xsjtagbrd.cpp

"$(INTDIR)\xsjtagbrd.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xsnullboard.cpp

"$(INTDIR)\xsnullboard.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xsvboard.cpp

"$(INTDIR)\xsvboard.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xulabrd.cpp

"$(INTDIR)\xulabrd.obj" : $(SOURCE) "$(INTDIR)"


!IF  "$(CFG)" == "xstoolslib - Win32 Release"

"iolib - Win32 Release" : 
   cd "\xesscorp\PRODUCTS\xstools_xula\iolib"
   $(MAKE) /$(MAKEFLAGS) /F .\iolib.mak CFG="iolib - Win32 Release" 
   cd "..\xstoolslib"

"iolib - Win32 ReleaseCLEAN" : 
   cd "\xesscorp\PRODUCTS\xstools_xula\iolib"
   $(MAKE) /$(MAKEFLAGS) /F .\iolib.mak CFG="iolib - Win32 Release" RECURSE=1 CLEAN 
   cd "..\xstoolslib"

!ELSEIF  "$(CFG)" == "xstoolslib - Win32 Debug"

"iolib - Win32 Debug" : 
   cd "\xesscorp\PRODUCTS\xstools_xula\iolib"
   $(MAKE) /$(MAKEFLAGS) /F .\iolib.mak CFG="iolib - Win32 Debug" 
   cd "..\xstoolslib"

"iolib - Win32 DebugCLEAN" : 
   cd "\xesscorp\PRODUCTS\xstools_xula\iolib"
   $(MAKE) /$(MAKEFLAGS) /F .\iolib.mak CFG="iolib - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\xstoolslib"

!ENDIF 


!ENDIF 

