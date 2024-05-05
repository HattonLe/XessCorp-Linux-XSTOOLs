These desktop shortcuts for Ubuntu 22.04 allow you to run the XSTOOLs as root.

On my system I have the tools in "/opt" together with the desktop icon, but you can edit the paths in the desktop files to point to where you put the xsload, xstest and xsport applications.

I have proven that the syntax used in these files for pkexec works across Ubuntu reboots.
Ignore any web info that says XAUTHORITY=... as that doesn't work. (the XAUTHORITY environment variable changes across reboots (not sure about logging off\on as I've not checked that), so using a hard coded value will only work for your current desktop session.
