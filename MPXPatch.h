#ifndef PK_H
#define PK_H

#include <X11/extensions/XInput2.h>


Display *dpy = NULL;
Window root;



void listen();
void swapMasters(XIDeviceEvent *devev);
void setClientPointerForWindow(XIDeviceEvent *devev);

#endif
