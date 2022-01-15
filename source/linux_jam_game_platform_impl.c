
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "jam_game_platform.c"

typedef struct
{
    size_t window_w;
    size_t window_h;
    Pixel pixels[PLT_gameFixedW * PLT_gameFixedH];
    GC gc;
} LNX_GraphicsContext;

static LNX_GraphicsContext LNX_graphicsContext = {0};
static PLT_GameInput LNX_os = {0};

static void
LNX_DrawPixelsToWindow(Display *display, Window window, Pixel *pixels, size_t w, size_t h)
{
#if 1
    Pixel *scaled = malloc(LNX_graphicsContext.window_w * LNX_graphicsContext.window_h * sizeof(*scaled));
    float scale_x = (float)w / (float)LNX_graphicsContext.window_w;
    float scale_y = (float)h / (float)LNX_graphicsContext.window_h;
    for (int y = 0;
         y < LNX_graphicsContext.window_h;
         y += 1)
    {
        for (int x = 0;
             x < LNX_graphicsContext.window_w;
             x += 1)
        {
            int dst = x + y * LNX_graphicsContext.window_w;
            int src = (int)(x * scale_x) + (int)(y * scale_y) * w;
            if(src > w * h)
            {
                src = w * h;
            }
            scaled[dst] = pixels[src];
        }
    }
    
    XImage image;
    image.width = LNX_graphicsContext.window_w;
    image.height = LNX_graphicsContext.window_h;
    image.format = 2;
    image.data = (char *)scaled;
    image.byte_order = 0;
    image.bitmap_unit = 32;
    image.bitmap_bit_order = 0;
    image.bitmap_pad = 32;
    image.depth = 24;
    image.bytes_per_line = LNX_graphicsContext.window_w * 4;
    image.bits_per_pixel = 32;
    XInitImage(&image);
    XPutImage(display, window, LNX_graphicsContext.gc, &image, 0, 0, 0, 0, LNX_graphicsContext.window_w, LNX_graphicsContext.window_h);
    
    free(scaled);
#else
    XImage image;
    image.width = w;
    image.height = h;
    image.format = 2;
    image.data = (char *C)pixels;
    image.byte_order = 0;
    image.bitmap_unit = 32;
    image.bitmap_bit_order = 0;
    image.bitmap_pad = 32;
    image.depth = 24;
    image.bytes_per_line = w * 4;
    image.bits_per_pixel = 32;
    XInitImage(&image);
    XPutImage(display, window, LNX_graphicsContext.gc, &image, 0, 0, 0, 0, w, h);
#endif
}

static void LNX_UpdateWindowSize(Display *display, Window window)
{
    int border, x, y, w, h, depth;
    XGetGeometry(display, window, &window, &x, &y, &w, &h, &border, &depth);
    LNX_graphicsContext.window_w = w;
    LNX_graphicsContext.window_h = h;
}

int
main(int arguments_count, char **arguments)
{
    int is_running = 1;
    
    
    Display *display = XOpenDisplay(NULL);
    if (NULL != display)
    {
        int screen = DefaultScreen(display);
        Window window = XCreateSimpleWindow(display, RootWindow(display, screen), 0, 0, PLT_gameFixedW, PLT_gameFixedH, 1,
                                            BlackPixel(display, screen), WhitePixel(display, screen));
        XSelectInput(display, window, ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | PointerMotionMask);
        XMapWindow(display, window);
        
        Atom wm_delete_message = XInternAtom(display, "WM_DELETE_WINDOW", 0);
        XSetWMProtocols(display, window, &wm_delete_message, 1);
        
        LNX_graphicsContext.gc = XCreateGC(display, window, 0, NULL);
        LNX_os.pixels = LNX_graphicsContext.pixels;
        
        GME_Initialise(&LNX_os);
        
        struct timespec ts;
        
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        size_t frame_begin = ts.tv_sec * 1000000000 + ts.tv_nsec;
        
        LNX_UpdateWindowSize(display, window);
        
        for (;is_running;)
        {
            while(XPending(display))
            {
                XEvent e;
                XNextEvent(display, &e);
                switch(e.type)
                {
                    default: break;
                    
                    case (ConfigureNotify):
                    {
                        LNX_UpdateWindowSize(display, window);
                    } break;
                    
                    case (Expose):
                    {
                        GME_UpdateAndRender(&LNX_os);
                        LNX_DrawPixelsToWindow(display, window, LNX_graphicsContext.pixels, PLT_gameFixedW, PLT_gameFixedH);
                    } break;
                    
                    case (KeyPress):
                    case (KeyRelease):
                    {
                        static PLT_Key key_lut[256] =
                        {
#include "linux_key_lut.c"
                        };
                        int is_down = (e.type == KeyPress);
                        LNX_os.is_key_down[key_lut[e.xkey.keycode - 8]] = is_down;
                    } break;
                    
                    case (ButtonPress):
                    case (ButtonRelease):
                    {
                        static PLT_Key button_lut[3] =
                        {
                            PLT_Key_mouseLeft,
                            PLT_Key_mouseMiddle,
                            PLT_Key_mouseRight
                        };
                        int is_down = (e.type == ButtonPress);
                        LNX_os.is_key_down[button_lut[e.xbutton.button - 1]] = is_down;
                    } break;
                    
                    case (ClientMessage):
                    {
                        if (wm_delete_message == e.xclient.data.l[0])
                        {
                            is_running = 0;
                        }
                    } break;
                    
                    case (MotionNotify):
                    {
                        LNX_os.mouse_x = ((float)e.xmotion.x / LNX_graphicsContext.window_w) * PLT_gameFixedW;
                        LNX_os.mouse_y = ((float)e.xmotion.y / LNX_graphicsContext.window_h) * PLT_gameFixedH;
                    } break;
                }
            }
            
            GME_UpdateAndRender(&LNX_os);
            LNX_DrawPixelsToWindow(display, window, LNX_graphicsContext.pixels, PLT_gameFixedW, PLT_gameFixedH);
            
            clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
            size_t frame_end = ts.tv_sec * 1000000000 + ts.tv_nsec;
            LNX_os.dt = (frame_end - frame_begin) / 1000000000.0;
            frame_begin = frame_end;
        }
        XFreeGC(display, LNX_graphicsContext.gc);
        XCloseDisplay(display);
    }
}