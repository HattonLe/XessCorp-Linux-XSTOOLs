# XessCorp-Linux-XSTOOLs
XessCorp Win32 XSTOOLs ported to Linux x64 for ParallelPort control of XSA-3S board via Qt GUI frontend.

My contribution to history - Weeks spent getting my mid 2000's era XessCorp XSA-3S Spartan3 system to communicate with my laptop using the parallel port interface.

The original Windows GUI based XessCorp tooling supplied with their excellent FPGA boards (such as the Spartan3 based XSA3-S board) controlled the FPGA board via a parallel cable interface, allowing user reconfiguration of both the onboard CPLD and FPGA via JTAG protocol. Later on the company introduced a USB interface called XSUSB.

Unfortunately the Win32 based tooling does not run on modern 64-bit Windows (due to the use of a third party 32 bit DLL). Neither will they work in a VirtualBox Windows 7 VM because VirtualBox won't support ParallelPort pass-thru any more (too lazy I guess).

Luckly I came across an old SourceForge dump of the Xess tooling Source code that the owner kindly made available for future generations. I have now ported this tooling across to x64 Linux using Qt3 for the font end GUI. Ubuntu 22.04 and Qt Creator was used as the development platform.

I have also fixed a number of faults\errors in the original code base whilst I was at it.

I have only proven the port for the XSA-3000 board using the Parallel port interface. You need to run the tooling as root in order to access the Parallel Port hardware.

Work to do would be to finish the source code port for USB interfacing and for the other board types.

Enjoy.
