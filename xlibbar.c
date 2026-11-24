/*****************************************************************\                                                                                             
*      xlibbar - minimal Xlib status bar for NetBSD               *                    
*      2026 Anlu Rodrigues De Sousa <anludesousa@gmail.com>       *                                                                                                   
\*****************************************************************/

                                                                                               
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#define BAR_HEIGHT 28
#define FONT_NAME "fixed"

static Display *dpy;
static Window win;
static GC gc;
static XFontStruct *font;
static int screen_num;
static unsigned int screen_width;
static char net_status[4] = "-X-";
static char audio_status[5] = "<x";
static char clock_status[6] = "00:00";
static int running = 1;

static void
run_command(const char *cmd, char *buf, size_t size)
{
    FILE *fp;
    size_t n;

    if (size == 0)
        return;
    buf[0] = '\0';
    fp = popen(cmd, "r");
    if (fp == NULL)
        return;
    n = fread(buf, 1, size - 1, fp);
    buf[n] = '\0';
    (void)pclose(fp);
}

static void
update_clock(void)
{
    time_t now = time(NULL);
    struct tm *tmv = localtime(&now);

    if (tmv != NULL)
        (void)strftime(clock_status, sizeof(clock_status), "%H:%M", tmv);
}

static void
update_network(void)
{
    char output[8192];
    char *line;

    /* A default IPv4 route is a practical indication of network connectivity. */
    run_command("/usr/bin/netstat -rn -f inet 2>/dev/null", output,
        sizeof(output));

    (void)strlcpy(net_status, "-X-", sizeof(net_status));
    line = strtok(output, "\n");
    while (line != NULL) {
        if (strncmp(line, "default", 7) == 0 ||
            strncmp(line, "0.0.0.0", 7) == 0) {
            (void)strlcpy(net_status, "-C-", sizeof(net_status));
            break;
        }
        line = strtok(NULL, "\n");
    }
}

static void
update_audio(void)
{
    char output[8192];
    char *p, *end;
    int left, right, maxv;

    /*
     * mixerctl is part of the NetBSD base system. aiomixer itself is
     * interactive and does not provide machine-readable status output.
     */
    run_command("mixerctl -a -v 2>/dev/null", output, sizeof(output));

    p = strstr(output, "outputs.master=");
    if (p == NULL) {
        (void)strlcpy(audio_status, "<x", sizeof(audio_status));
        return;
    }

    p += strlen("outputs.master=");
    left = (int)strtol(p, &end, 10);
    if (end == p) {
        (void)strlcpy(audio_status, "<x", sizeof(audio_status));
        return;
    }

    right = left;
    if (*end == ',')
        right = (int)strtol(end + 1, NULL, 10);

    if (left < 0) left = 0;
    if (right < 0) right = 0;
    maxv = left > right ? left : right;

    if (maxv == 0)
        (void)strlcpy(audio_status, "<", sizeof(audio_status));
    else if (maxv <= 85)
        (void)strlcpy(audio_status, "<)", sizeof(audio_status));
    else if (maxv <= 170)
        (void)strlcpy(audio_status, "<))", sizeof(audio_status));
    else
        (void)strlcpy(audio_status, "<)))", sizeof(audio_status));
}

static int
text_width(const char *s)
{
    return XTextWidth(font, s, (int)strlen(s));
}

static void
draw_bar(void)
{
    int y = (BAR_HEIGHT - (font->ascent + font->descent)) / 2
        + font->ascent;
    int right_x;

    XSetForeground(dpy, gc, BlackPixel(dpy, screen_num));
    XFillRectangle(dpy, win, gc, 0, 0, screen_width, BAR_HEIGHT);

    XSetForeground(dpy, gc, WhitePixel(dpy, screen_num));
    XDrawString(dpy, win, gc, 10, y, clock_status,
        (int)strlen(clock_status));

    right_x = (int)screen_width - 10 - text_width(audio_status);
    XDrawString(dpy, win, gc, right_x, y, audio_status,
        (int)strlen(audio_status));

    right_x -= 16 + text_width(net_status);
    XDrawString(dpy, win, gc, right_x, y, net_status,
        (int)strlen(net_status));

    XFlush(dpy);
}

static void
handle_signal(int sig)
{
    (void)sig;
    running = 0;
}

int
main(void)
{
    XSetWindowAttributes attrs;
    XEvent event;
    struct timespec delay = { 0, 200000000L };
    int elapsed = 0;

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        fprintf(stderr, "xlibbar: cannot open X display\n");
        return 1;
    }

    screen_num = DefaultScreen(dpy);
    screen_width = (unsigned int)DisplayWidth(dpy, screen_num);

    attrs.override_redirect = True;
    attrs.background_pixel = BlackPixel(dpy, screen_num);
    attrs.event_mask = ExposureMask | StructureNotifyMask;

    win = XCreateWindow(dpy, RootWindow(dpy, screen_num),
        0, 0, screen_width, BAR_HEIGHT, 0,
        CopyFromParent, InputOutput, CopyFromParent,
        CWOverrideRedirect | CWBackPixel | CWEventMask, &attrs);

    XStoreName(dpy, win, "xlibbar");
    font = XLoadQueryFont(dpy, FONT_NAME);
    if (font == NULL)
        font = XLoadQueryFont(dpy, "7x13");
    if (font == NULL) {
        fprintf(stderr, "xlibbar: cannot load X11 font\n");
        XDestroyWindow(dpy, win);
        XCloseDisplay(dpy);
        return 1;
    }

    gc = XCreateGC(dpy, win, 0, NULL);
    XSetFont(dpy, gc, font->fid);
    XMapRaised(dpy, win);

    update_clock();
    update_network();
    update_audio();
    draw_bar();

    while (running) {
        while (XPending(dpy)) {
            XNextEvent(dpy, &event);
            if (event.type == Expose)
                draw_bar();
        }

        (void)nanosleep(&delay, NULL);
        elapsed += 1;
        if (elapsed >= 5) {
            elapsed = 0;
            update_clock();
            update_network();
            update_audio();
            draw_bar();
        }
    }

    XFreeGC(dpy, gc);
    XFreeFont(dpy, font);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
    return 0;
}
