#include "MPXPatch.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <X11/extensions/XInput2.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>


static int handleError(Display *dpy, XErrorEvent *event)
{
	char buff[100];
	XGetErrorText(dpy,event->error_code,buff,40);
	printf("Ignoring Xlib error: error code %d request code %d %s\n",
		   event->error_code,
		   event->request_code,buff) ;

	return 0;
}

void checkXServerVersion(){
	int opcode, event, error;
	if (!XQueryExtension(dpy, "XInputExtension", &opcode, &event, &error)) {
	   printf("X Input extension not available.\n");
	   exit(1);
	}

	/* Which version of XI2? We support 2.0 */
	int major = 2, minor = 0;
	if (XIQueryVersion(dpy, &major, &minor) == BadRequest) {
	  printf("XI2 not available. Server supports %d.%d\n", major, minor);
	  exit(1);
	}
}
void init(){
	dpy = XOpenDisplay(NULL);
	checkXServerVersion();

	if (!dpy)
		exit(2);

	root = DefaultRootWindow(dpy);
	swapKeyCode=XKeysymToKeycode(dpy, XK_Escape);

	//XSelectInput(dpy, root, KeyPressMask|KeyReleaseMask);
	XSetErrorHandler(handleError);
	listen();


}
void listen(){
	XIEventMask eventmask;
	unsigned char mask[1] = { 0 }; /* the actual mask */

	eventmask.deviceid = XIAllDevices;
	eventmask.mask_len = sizeof(mask); /* always in bytes */
	eventmask.mask = mask;
	/* now set the mask */
	XISetMask(mask, XI_ButtonPress);
	XISetMask(mask, XI_KeyPress);

	XISelectEvents(dpy, root, &eventmask, 1);

}

int main(){
	init();


	XEvent event;
	XIDeviceEvent *devev;
	XGenericEventCookie *cookie;
	printf("starting");
	while(True){
    	XNextEvent(dpy,&event);
		cookie = &event.xcookie;
		
		if(XGetEventData(dpy, cookie)){
			devev = cookie->data;
			if(cookie->evtype==XI_ButtonPress){
				if(devev->detail == Button1)	
					setClientPointerForWindow(devev);
			}
			else{
				if(devev->detail == swapKeyCode)
					swapMasters(devev);
			}
		}
		XFreeEventData(dpy, cookie);
		printf("\n");
	}
	printf("exiting\n");

}

int getAssociatedMasterDevice(int deviceId){
	int ndevices;
	XIDeviceInfo *masterDevices;
	int id;
	masterDevices = XIQueryDevice(dpy, deviceId, &ndevices);
	id=masterDevices[0].attachment;
	XIFreeDeviceInfo(masterDevices);
	return id;
}

int endsWith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}
void swapMasters(XIDeviceEvent *devev){
	int masterPointer;
	XIGetClientPointer(dpy, 0, &masterPointer);
	int masterKeyboard=getAssociatedMasterDevice(masterPointer);
	int pointer=getAssociatedMasterDevice(getAssociatedMasterDevice(devev->deviceid));
	int keyboard=getAssociatedMasterDevice(pointer);
	
	printf("%d %d %d %d",masterPointer,masterKeyboard,pointer,keyboard);
	//only act if device isn't already default master
	if(masterKeyboard==keyboard||masterPointer==pointer)
		return;
	if(masterKeyboard==pointer || masterPointer==keyboard)
		return;

	//swap keyboard focus
	Window masterKeyboardFocus;
	Window keyboardFocus;
	XIGetFocus(dpy, masterKeyboard, &masterKeyboardFocus);
	XIGetFocus(dpy, keyboard, &keyboardFocus);

	if(masterKeyboardFocus==0 ||keyboardFocus==0)
		return;

	printf("Window master:%lu other:%lu",masterKeyboardFocus,keyboardFocus);

	XISetFocus(dpy, keyboard, masterKeyboardFocus, CurrentTime);
	XISetFocus(dpy,  masterKeyboard, keyboardFocus,CurrentTime);


	//swap mouse position
	double ignore ;
	Window wIgnore;
	XIButtonState       buttons;
	XIModifierState     mods;
	XIGroupState        group;
	double x,y;
	double masterX,masterY;
	XIQueryPointer(dpy, pointer,root, &wIgnore, &wIgnore, &x, &y, &ignore, &ignore, &buttons, &mods, &group);
	XIQueryPointer(dpy, masterPointer, root, &wIgnore, &wIgnore, &masterX, &masterY,&ignore, &ignore, &buttons, &mods, &group);
	printf("POS master:%f,%f other:%f,%f",x,y,masterX,masterY);
	XIWarpPointer(dpy, masterPointer, None, root,0,0,0,0, x,y);
	XIWarpPointer(dpy, pointer, None, root,0,0,0,0, masterX, masterY);



	int ndevices;
	XIDeviceInfo *devices, device;
	devices = XIQueryDevice(dpy, XIAllDevices, &ndevices);
	XIAnyHierarchyChangeInfo changes[ndevices];
	int actualChanges=0;
	//swap slave devices
	for (int i = 0; i < ndevices; i++) {
		device = devices[i];

		switch(device.use) {
		   case XISlavePointer:
			   if(device.attachment!=masterPointer && device.attachment!=pointer)
				   continue;
			   if(endsWith(device.name,"XTEST pointer"))
				   continue;

			   changes[actualChanges].type=XIAttachSlave;
			   changes[actualChanges].attach.deviceid=device.deviceid;
			   changes[actualChanges].attach.new_master=device.attachment==masterPointer?pointer:masterPointer;
			   actualChanges++;
			   break;

		   case XISlaveKeyboard:
			   if(device.attachment!=masterKeyboard && device.attachment!=keyboard)
				   continue;
			   if(endsWith(device.name,"XTEST keyboard"))
				   continue;
			   changes[actualChanges].type=XIAttachSlave;
			   changes[actualChanges].attach.deviceid=device.deviceid;
			   changes[actualChanges].attach.new_master=device.attachment==masterKeyboard?keyboard:masterKeyboard;
			   actualChanges++;
			   break;
		}
	}
	XIChangeHierarchy(dpy, changes, actualChanges);
	XIFreeDeviceInfo(devices);

	XFlush(dpy);

	printf("\n");
}

void setClientPointerForWindow(XIDeviceEvent *devev){
	int pointerId=getAssociatedMasterDevice(devev->deviceid);
	int keyboardId=getAssociatedMasterDevice(pointerId);
	Window clientWindow;
	/*
	double ignore;
	Window wIgnore,clientWindow;
	XIButtonState       buttons;
	XIModifierState     mods;
	XIGroupState        group;
	XIQueryPointer(dpy, pointerId, root, &wIgnore,&clientWindow, &ignore, &ignore,&ignore, &ignore, &buttons, &mods, &group);
	*/
	XIGetFocus(dpy, keyboardId, &clientWindow);
	//XISetFocus(dpy, keyboardId, clientWindow, 0);
	XISetClientPointer(dpy,clientWindow,pointerId);
}
