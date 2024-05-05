# XessCorp-Linux-XSTOOLs
XessCorp Win32 XSTOOLs ported to Linux x64 for ParallelPort control of XSA-3S with XSTend expansion board via Qt GUI frontend.

My contribution to technological archaeology - Weeks spent getting my mid 2000's era XessCorp XSA-3S Spartan3 system to communicate with my Dell M6700 using the parallel port interface.

The original Windows GUI based XessCorp tooling supplied with their excellent FPGA boards (such as the Spartan3 based XSA3-S board) controlled the FPGA board via a parallel cable interface, allowing user reconfiguration of both the onboard CPLD and FPGA via JTAG protocol. Later on the company introduced a USB interface called XSUSB.

Unfortunately the Win32 based tooling does not run on 64-bit Windows (due to the use of a third party 32 bit DLL). Neither will they work in a VirtualBox Windows 7 VM because VirtualBox won't support ParallelPort pass-thru any more (too lazy I guess).

Luckly I came across an old SourceForge dump of the Xess tooling Source code that the owner kindly made available for future generations. I have now ported this tooling across to x64 Linux using Qt5.15.3 for the font end GUI. Ubuntu 22.04 and Qt Creator 6.0.2 was used as the development platform.

You need to run the tooling as root for Parallel Port hardware access, so here is how to get Ubuntu 22.04 desktop to launch applications as root.
(the desktop shortcut files are in the folder "UbuntuDesktop").

Copy the shortcut files to your desktop folder.
IMPORTANT - use the following terminal command to put each of those desktop shortcuts into group root otherwise the launcher won't work properly!
sudo chgrp root GXS*

Then if required edit the paths in them and change the argument to gxsload\gxstest\gxsport to be user user name (in the files it is currently set to main).
Then mark each desktop shortcut to allow launching.

The "XessData" folder should be copied into your ~/Documents area. This folder holds Xess parameter files and in addition it will be the location of any data files created via uploading from the FPGA board.

I have also fixed a number of faults\errors in the original code base whilst porting it. I have only proven the port for the XSA-3000 board using the Parallel port interface. So far downloading to the CPLD and FPGA work. Also RAM and Flash uploading appear to work as well. I have not yet tested RAM and Flash downloading.

Work to do would be to finish the source code port for USB interfacing and to do the same for the other board types. (the files are present but excluded from the project builds just to get something working quickly). I don't have a USB interface or another type of Xess board, so I can't easily prove the port for these areas.

Enjoy.
