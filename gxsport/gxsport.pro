QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../xstoolslib/Markup.cpp \
    ../xstoolslib/akcdcprt.cpp \
    ../xstoolslib/am29fprt.cpp \
    ../xstoolslib/at17prt.cpp \
    ../xstoolslib/at49fprt.cpp \
    ../xstoolslib/bitstrm.cpp \
    ../xstoolslib/cnfgport.cpp \
    ../xstoolslib/f28port.cpp \
    ../xstoolslib/flashprt.cpp \
    ../xstoolslib/guitools.cpp \
    ../xstoolslib/hex.cpp \
    ../xstoolslib/hexrecrd.cpp \
    ../xstoolslib/i2cport.cpp \
    ../xstoolslib/i2cportlpt.cpp \
    ../xstoolslib/io.cpp \
    ../xstoolslib/jramprt.cpp \
    ../xstoolslib/jtagport.cpp \
    ../xstoolslib/lptjtag.cpp \
    ../xstoolslib/mchpport.cpp \
    ../xstoolslib/osccyprt.cpp \
    ../xstoolslib/oscport.cpp \
    ../xstoolslib/parameters.cpp \
    ../xstoolslib/parport.cpp \
    ../xstoolslib/pbusport.cpp \
    ../xstoolslib/pport.cpp \
    ../xstoolslib/progress.cpp \
    ../xstoolslib/ramport.cpp \
    ../xstoolslib/saa711x.cpp \
    ../xstoolslib/testport.cpp \
    ../xstoolslib/transport.cpp \
    ../xstoolslib/transports.cpp \
    ../xstoolslib/utils.cpp \
    ../xstoolslib/xc2sprt.cpp \
    ../xstoolslib/xc3sprt.cpp \
    ../xstoolslib/xc4kprt.cpp \
    ../xstoolslib/xc95kprt.cpp \
    ../xstoolslib/xcbsdr.cpp \
    ../xstoolslib/xcvprt.cpp \
    ../xstoolslib/xs40brd.cpp \
    ../xstoolslib/xs95brd.cpp \
    ../xstoolslib/xsa3sbrd.cpp \
    ../xstoolslib/xsaboard.cpp \
    ../xstoolslib/xsboard.cpp \
    ../xstoolslib/xserror.cpp \
    ../xstoolslib/xsnullboard.cpp \
    main.cpp \
    gxsportdlg.cpp

HEADERS += \
    ../xstoolslib/CppTimer.h \
    ../xstoolslib/CppTimerCallback.h \
    ../xstoolslib/Markup.h \
    ../xstoolslib/StdAfx.h \
    ../xstoolslib/akcdcprt.h \
    ../xstoolslib/am29fprt.h \
    ../xstoolslib/at17prt.h \
    ../xstoolslib/at49fprt.h \
    ../xstoolslib/bitstrm.h \
    ../xstoolslib/cnfgport.h \
    ../xstoolslib/eeprom_flags.h \
    ../xstoolslib/f28port.h \
    ../xstoolslib/flashprt.h \
    ../xstoolslib/guitools.h \
    ../xstoolslib/hex.h \
    ../xstoolslib/hexrecrd.h \
    ../xstoolslib/i2cport.h \
    ../xstoolslib/i2cportlpt.h \
    ../xstoolslib/io.h \
    ../xstoolslib/jramprt.h \
    ../xstoolslib/jtagport.h \
    ../xstoolslib/lptjtag.h \
    ../xstoolslib/mchpport.h \
    ../xstoolslib/osccyprt.h \
    ../xstoolslib/oscport.h \
    ../xstoolslib/parameters.h \
    ../xstoolslib/pbusport.h \
    ../xstoolslib/pport.h \
    ../xstoolslib/progress.h \
    ../xstoolslib/ramport.h \
    ../xstoolslib/saa711x.h \
    ../xstoolslib/testport.h \
    ../xstoolslib/transport.h \
    ../xstoolslib/transports.h \
    ../xstoolslib/usbcmd.h \
    ../xstoolslib/utils.h \
    ../xstoolslib/xc2sprt.h \
    ../xstoolslib/xc3sprt.h \
    ../xstoolslib/xc4kprt.h \
    ../xstoolslib/xc95kprt.h \
    ../xstoolslib/xcbsdr.h \
    ../xstoolslib/xcvprt.h \
    ../xstoolslib/xs40brd.h \
    ../xstoolslib/xs95brd.h \
    ../xstoolslib/xsa3sbrd.h \
    ../xstoolslib/xsaboard.h \
    ../xstoolslib/xsallbrds.h \
    ../xstoolslib/xsboard.h \
    ../xstoolslib/xserror.h \
    ../xstoolslib/xsnullboard.h \
    FixLH.h \
    gxsportdlg.h \
    wtypes.h

FORMS += \
    gxsportdlg.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=
