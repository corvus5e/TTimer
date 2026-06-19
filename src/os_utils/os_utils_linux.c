#include "os_utils.h"

#include <X11/Xlib.h>
#include <X11/extensions/scrnsaver.h>
#include <unistd.h>
#include <stdio.h>

XScreenSaverInfo *info = NULL; //TODO: Put in better place

static Display *get_display() {
	static int inited = 0;
	static Display *display = NULL;

	if (!inited) {
		inited = 1; // Mark as initialized so we don't repeat this block on failure

		if (!(display = XOpenDisplay(NULL))) {
			fprintf(stderr, "Cannot open display. Are you running under X11?\n");
			return NULL;
		}

		info = XScreenSaverAllocInfo();
		if (!info) {
			fprintf(stderr, "Out of memory allocating XScreenSaverInfo.\n");
			XCloseDisplay(display);
			display = NULL;
			return NULL;
		}

		// Check if XScreenSaver extension is available
		int event_base, error_base;
		if (!XScreenSaverQueryExtension(display, &event_base, &error_base)) {
			fprintf(stderr, "XScreenSaver extension not supported by the X server.\n");
			XCloseDisplay(display);
			display = NULL;
			return NULL;
		}
	}

	return display;
}

float idle_time() {

	Display *display = get_display();
	if (!display) {
		return 0.f;
	}

	// Query idle time
	if (XScreenSaverQueryInfo(display, DefaultRootWindow(display), info)) {
		// info->idle is in milliseconds
		return (float)info->idle / 1000.0f;
	} else {
		fprintf(stderr, "Failed to query XScreenSaverInfo\n");
	}

	return 0.f;
}

void dispose_os_resources()
{
	if (info) {
		XFree(info);
	}

	Display *display = get_display();
	if (display) {
		XCloseDisplay(display);
	}
}
