#include <X11/Xlib.h>
#include <X11/extensions/Xinerama.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xinerama.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    Display* display;
    Window window;
    int width;
    int height;
    int x;
    int y;
    unsigned long color;
} WindowData;

bool exit_enabled = true;  // Global flag to control exit

void* render_loop(void* arg) {
    WindowData* data = (WindowData*)arg;
    XEvent event;

    while (1) {
        XNextEvent(data->display, &event);
        if (event.type == KeyPress) {
            XKeyEvent* key_event = (XKeyEvent*)&event;
            if (exit_enabled &&
                ((key_event->keycode == XKeysymToKeycode(data->display, XStringToKeysym("Q")) && (key_event->state & ControlMask)) ||
                 (key_event->keycode == XKeysymToKeycode(data->display, XStringToKeysym("space"))))) {
                XDestroyWindow(data->display, data->window);
                XCloseDisplay(data->display);
                exit(0);
            }
        }
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    // Initialize Xlib for thread safety
    XInitThreads();

    // Parse command-line arguments
    if (argc > 1 && strcmp(argv[1], "exit_disabled") == 0) {
        fprintf(stderr, "exit disabled\n");
        exit_enabled = false;
    }

    Display* display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Unable to open X display\n");
        return -1;
    }

    int screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);

    if (!XineramaIsActive(display)) {
        fprintf(stderr, "Xinerama is not active.\n");
        XCloseDisplay(display);
        return -1;
    }

    int num_screens;
    XineramaScreenInfo* screens = XineramaQueryScreens(display, &num_screens);

    if (num_screens < 2) {
        fprintf(stderr, "Multiple monitors are not available.\n");
        XFree(screens);
        XCloseDisplay(display);
        return -1;
    }

    WindowData data[2];
    pthread_t thread1, thread2;

    for (int i = 0; i < 2; i++) {
        data[i].display = display;
        data[i].width = screens[i].width;
        data[i].height = screens[i].height;
        data[i].x = screens[i].x_org;
        data[i].y = screens[i].y_org;
        data[i].color = (i == 0) ? 0xFF0000 : 0x0000FF;

        data[i].window = XCreateSimpleWindow(display, root, data[i].x, data[i].y, data[i].width, data[i].height, 1, BlackPixel(display, screen), data[i].color);

        XSelectInput(display, data[i].window, ExposureMask | KeyPressMask | StructureNotifyMask);
        XMapWindow(display, data[i].window);

        // Wait for the window to be mapped
        XEvent event;
        do {
            XNextEvent(display, &event);
        } while (event.type != MapNotify || event.xmap.window != data[i].window);

        XSetInputFocus(data[i].display, data[i].window, RevertToParent, CurrentTime);
        XGrabKeyboard(data[i].display, data[i].window, True, GrabModeAsync, GrabModeAsync, CurrentTime);
        XGrabPointer(data[i].display, data[i].window, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, data[i].window, None, CurrentTime);
    }

    pthread_create(&thread1, NULL, render_loop, &data[0]);
    pthread_create(&thread2, NULL, render_loop, &data[1]);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    XFree(screens);
    XCloseDisplay(display);
    return 0;
}
