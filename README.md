#MPX-Patch
MPX support has been in built into X11 for years but so many applications/WM do not have full support. MPX-Patch aims to provide a MPX experience in these unfortunate situations.

##Features
###Clicking on a window sets the client pointer
Some applications like Thunar, incorrectly rely on commands like XQueryPointer to find the user's mouse. X11 returns the client pointer, which is often just the default Virtual Core Pointer (VCP) which may not even be focused on the window. So MPX-Patch sets the client pointer correctly everything time you click on a window.

###Swap Master Pointers
Despite everything we may try to do, there will probably be some situations were we fail. This is the fail safe. By default, whenever Super_L is pressed, the associated pointer/keyboard will be seamlessly swapped with the VC pointer/keyboard and become the default master. The user will notice nothing except that applications will correctly use the target mouse.

## Requirements

Building requires:

* Xlib header files (Debian: libx11-dev, Arch: libx11)
* GNU make
* a C99 compiler

