#include <iostream>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <thread>
#include <mutex>
#include <vector>

#include <X11/Xlib.h>
#include <X11/keysym.h> // for Keysym stuff
#include <X11/Xutil.h> // for XLookupString
#include <stdio.h>

#define WIDTH 640
#define HEIGHT 480

extern "C" void iterate8(float *a_cr, float *a_ci, uint64_t a_max_iterations, uint64_t *a_itertbl);

Display *dpy;
int blackColor, whiteColor;
Window w;
GC gc;
Pixmap osb;
std::mutex x_mutex;

float cxmax;
float cxmin;
float cymax;
float cymin;
float frac_aspectr;
float frac_size = 1.7;
float frac_centerx = -0.5;
float frac_centery = 0.0;
unsigned int frac_width = WIDTH;
unsigned int frac_height = HEIGHT;
unsigned int max_iterations = 1024;

void mlvector();
void mlvector_run(int a_thread_id, int a_num_threads);

void redraw(void)
{
	XCopyArea(dpy, osb, w, gc, 0, 0, frac_width, frac_height, 0, 0);
}

int main(int argc, char **argv)
{
	int runFlag=1;
	int ShiftState = 0, ControlState = 0, AltState = 0;
	dpy = XOpenDisplay(0);
	assert(dpy);

	blackColor = BlackPixel(dpy, DefaultScreen(dpy));
	whiteColor = WhitePixel(dpy, DefaultScreen(dpy));

	if (argc == 3) {
		std::cout << "Overriding default image size." << std::endl;
		frac_width = std::atoi(argv[1]);
		frac_height = std::atoi(argv[2]);
	}
	frac_aspectr = (float)(frac_width) / (float)(frac_height);

	std::cout << "Creating " << frac_width << "x" << frac_height << " window..." << std::endl;
	w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, frac_width, frac_height, 0, blackColor, blackColor);

	// constrict window to set size
	XSizeHints sizehints;
	sizehints.flags=PSize|PMinSize|PMaxSize;
	sizehints.min_width = frac_width;
	sizehints.max_width = frac_width;
	sizehints.min_height = frac_height;
	sizehints.max_height = frac_height;
	XSetWMNormalHints(dpy, w, &sizehints);

	// we want to get MapNotify events
	XSelectInput(dpy, w, StructureNotifyMask | ExposureMask | ButtonPressMask | PointerMotionMask | KeyPressMask | KeyReleaseMask | ButtonReleaseMask);

	// create off-screen bitmap
	osb = XCreatePixmap(dpy, w, frac_width, frac_height, 24);
	
	Atom wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", 1);
	XSetWMProtocols(dpy, w, &wm_delete, 1);

	// set the window's title
	XStoreName(dpy, w, "Mandelf");
	
	// "map" the window (make it appear)
	std::cout << "Mapping window..." << std::endl;
	XMapWindow(dpy, w);

	gc = XCreateGC(dpy, osb, 0, 0);

	for(;;)
	{
		XEvent e;
		XNextEvent(dpy, &e);
		if (e.type == MapNotify)
			break;
	}

	mlvector();
	redraw();
		
	// wait for an event
	while (runFlag == 1)
	{
		XEvent e;
		XNextEvent(dpy, &e);
		//KeySym key_symbol = XKeycodeToKeysym(dpy, e.xkey.keycode, 0);
		KeySym key_symbol;
		char xlat[10];
		if ((e.type == KeyPress) || (e.type == KeyRelease))
			XLookupString(&e.xkey, xlat, 10, &key_symbol, NULL);

		switch(e.type)
		{
			case ConfigureNotify:
				printf("ConfigureNotify event\n");
				break;
			case Expose:
				redraw();
				printf("Expose event\n");
				break;
			case KeyPress:
			{
				switch(key_symbol) {
					case XK_Shift_L:
					case XK_Shift_R:
						ShiftState = 1;
						break;
					case XK_Control_L:
					case XK_Control_R:
						ControlState = 1;
						break;
					case XK_Alt_L:
					case XK_Alt_R:
						AltState = 1;
						break;
					default:
					printf("Key: %04X ShiftState: %d ControlState: %d AltState: %d XLookupString '%s' (0x%02X)\n",key_symbol, ShiftState, ControlState, AltState, xlat, xlat[0]);
				}
				if (key_symbol == 0x2b) {
					max_iterations *= 2;
					std::cout << "Iterations *=2: " << max_iterations << std::endl;
					mlvector();
					redraw();

				} else if (key_symbol == 0x2d) {
					max_iterations /= 2;
					std::cout << "Iterations /=2: " << max_iterations << std::endl;
					mlvector();
					redraw();
				}
			}
				break;
			case KeyRelease:
				switch(key_symbol) {
					case XK_Shift_L:
					case XK_Shift_R:
						ShiftState = 0;
						break;
					case XK_Control_L:
					case XK_Control_R:
						ControlState = 0;
						break;
					case XK_Alt_L:
					case XK_Alt_R:
						AltState = 0;
						break;
				}
				break;
			case ButtonPress:
				std::cout << "Button " << e.xbutton.button << " at: X" << e.xbutton.x << ", Y" << e.xbutton.y;
				if ((e.xbutton.button == 1) || (e.xbutton.button == 4) || (e.xbutton.button == 5)) {
					if (e.xbutton.button == 1) {
						float ncx = cxmin + (e.xbutton.x / ((float)frac_width - 1.0)) * (cxmax - cxmin);
						float ncy = cymin + (e.xbutton.y / ((float)frac_height - 1.0)) * (cymax - cymin);

						std::cout << " - frac_centerx: " << frac_centerx << " frac_centery: " << frac_centery << " ncx: " << ncx << " ncy: " << ncy;

						// recenter the image
						frac_centerx = ncx;
						frac_centery = ncy;
					}

					if (e.xbutton.button == 4)
						frac_size *= (8.5 / 10.0);
					if (e.xbutton.button == 5)
						frac_size *= (10.0 / 8.5);

					std::cout << " - frac_size: " << frac_size;

					mlvector();
					redraw();
				}
				std::cout << std::endl;
				break;
			case ClientMessage:
				char *str = XGetAtomName(dpy, e.xclient.message_type);
				printf("ClientMessage: %s\n", str);
				if (!std::strcmp(str, "WM_PROTOCOLS"))
					runFlag = 0;
				XFree(str);
				break;
		}
	}

	printf("Freeing resources...\n");
	XFreePixmap(dpy, osb);
	XFreeGC(dpy, gc);
	XCloseDisplay(dpy);
	return 0;
}

void mlvector()
{
	XFillRectangle(dpy, osb, gc, 0, 0, frac_width, frac_height);

	std::vector<std::thread> run_threads;
	int num_threads = std::thread::hardware_concurrency();
	run_threads.reserve(num_threads);

	for (int i = 0; i < num_threads; ++i) {
		run_threads.emplace_back(std::thread(mlvector_run, i, num_threads));
	}

	for (int i = 0; i < num_threads; ++i) {
		run_threads[i].join();
	}

	XFlush(dpy);
}

void mlvector_run(int a_thread_id, int a_num_threads)
{
	std::size_t const ixsize = frac_width;
	std::size_t const iysize = frac_height;

	cxmax = frac_centerx + (frac_size * frac_aspectr);
	cxmin = frac_centerx - (frac_size * frac_aspectr);;
	cymax = frac_centery + frac_size;
	cymin = frac_centery - frac_size;

	for (std::size_t ix = 0; ix < ixsize; ix += 8)
		for (std::size_t iy = 0 + a_thread_id; iy < iysize; iy += a_num_threads)
		{
			float cr[8], ci[8];
			for (int offset = 0; offset < 8; ++offset) {
				cr[offset] = cxmin + (ix + offset)/(ixsize - 1.0) * (cxmax - cxmin);
				ci[offset] = cymin + iy/(iysize - 1.0) * (cymax - cymin);
			}

			uint64_t itertbl[8];
			iterate8(cr, ci, max_iterations, itertbl);

			for (int offset = 0; offset < 8; ++offset) {
				x_mutex.lock();
				XSetForeground(dpy, gc, (itertbl[offset] % 256) + (itertbl[offset] % 256) * 256);
				XDrawPoint(dpy, osb, gc, ix + offset, iy);
				x_mutex.unlock();
			}
	        }

	return;
}

