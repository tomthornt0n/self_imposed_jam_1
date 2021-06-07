#include <windows.h>
#include <windowsx.h>

#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>

#include <stdio.h>

#include "jam_game_platform.c"

typedef struct
{
    size_t window_w;
    size_t window_h;
    Pixel pixels[348364800];
    BITMAPINFO bitmap_info;
} W32_GraphicsContext;

static W32_GraphicsContext W32_graphicsContext = {0};

static PLT_GameInput W32_os = {0};

static int W32_isGameInitialised = 0;

static void
W32_RendererInitialise(void)
{
    BITMAPINFOHEADER *bih = &W32_graphicsContext.bitmap_info.bmiHeader;
    bih->biSize = sizeof(BITMAPINFOHEADER);
    bih->biPlanes = 1;
    bih->biBitCount = 32;
    bih->biCompression = BI_RGB;
    bih->biSizeImage = 0;
    bih->biClrUsed = 0;
    bih->biClrImportant = 0;
    
    W32_os.pixels = W32_graphicsContext.pixels;
}

typedef struct
{
    int is_initialised;
    IMMDeviceEnumerator *device_enumerator;
    IMMDevice *device;
    IAudioClient *audio_client;
    IAudioRenderClient *audio_render_client;
    REFERENCE_TIME sound_buffer_duration;
    UINT32 buffer_frame_count;
    WORD channels;
    UINT32 samples_per_second;
    UINT32 latency_frame_count;
} W32_AudioState;

static void
W32_DrawPixelsToWindow(HDC device_context_handle, Pixel *pixels, size_t w, size_t h)
{
    W32_graphicsContext.bitmap_info.bmiHeader.biWidth = w;
    W32_graphicsContext.bitmap_info.bmiHeader.biHeight = -h;
    StretchDIBits(device_context_handle,
                  0, 0, W32_graphicsContext.window_w, W32_graphicsContext.window_h,
                  0, 0, w, h,
                  pixels,
                  &W32_graphicsContext.bitmap_info,
                  DIB_RGB_COLORS,
                  SRCCOPY);
}

static void W32_UpdateWindowSize(HWND window_handle)
{
    RECT rc;
    GetClientRect(window_handle, &rc);
    W32_graphicsContext.window_w = rc.right - rc.left;
    W32_graphicsContext.window_h = rc.bottom - rc.top;
}

LRESULT CALLBACK W32_EventCallback(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param);

int WINAPI
WinMain(HINSTANCE instance_handle,
        HINSTANCE prev_instance_handle,
        LPSTR command_line,
        int show_mode)
{
    W32_RendererInitialise();
    
    char *window_class_name = "JamGame";
    
    WNDCLASSEX window_class = {0};
    window_class.cbSize = sizeof(WNDCLASSEX);
    window_class.style = CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc = W32_EventCallback;
    window_class.hInstance = instance_handle;
    window_class.hCursor = LoadCursorA(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)COLOR_WINDOW;
    window_class.lpszClassName = window_class_name;
    RegisterClassExA(&window_class);
    
    DWORD window_styles = WS_OVERLAPPEDWINDOW;
    int window_x = CW_USEDEFAULT;
    int window_y = CW_USEDEFAULT;
    int window_w = PLT_gameFixedW;
    int window_h = PLT_gameFixedH;
    
    RECT window_rect = {0, 0, window_w, window_h};
    AdjustWindowRect(&window_rect, window_styles, FALSE);
    
    HWND window_handle = CreateWindowA(window_class_name,
                                       PLT_windowTitle,
                                       window_styles,
                                       window_x, window_y,
                                       window_rect.right-window_rect.left,
                                       window_rect.bottom-window_rect.top,
                                       NULL,
                                       NULL,
                                       instance_handle,
                                       NULL);
    ShowWindow(window_handle, SW_SHOWMAXIMIZED);
    
    W32_UpdateWindowSize(window_handle);
    
    W32_os.window_w = PLT_gameFixedW;
    W32_os.window_h = PLT_gameFixedH;
    
    GME_Initialise(&W32_os);
    W32_isGameInitialised = 1;
    
    LARGE_INTEGER freq, start_t = {0}, end_t = {0};
    QueryPerformanceFrequency(&freq);
    
    int exit_code;
    
    BOOL running = 1;
    while (running)
    {
        QueryPerformanceCounter(&start_t);
        
        MSG msg = {0};
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message  == WM_QUIT)
            {
                running = 0;
                exit_code = msg.wParam;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            GME_UpdateAndRender(&W32_os);
        }
        
        HDC device_context_handle = GetDC(window_handle);
        {
            W32_DrawPixelsToWindow(device_context_handle, W32_graphicsContext.pixels, W32_os.window_w, W32_os.window_h);
        } ReleaseDC(window_handle, device_context_handle);
        
        QueryPerformanceCounter(&end_t);
        W32_os.dt = (double)(end_t.QuadPart - start_t.QuadPart) / (double)freq.QuadPart;
    }
    
    W32_isGameInitialised = 0;
    
    return exit_code;
}

LRESULT CALLBACK
W32_EventCallback(HWND window_handle,
                  UINT message,
                  WPARAM w_param,
                  LPARAM l_param)
{
    if (WM_DESTROY == message)
    {
        PostQuitMessage(0);
        return 0;
    }
    else if (WM_PAINT == message)
    {
        PAINTSTRUCT ps;
        HDC device_context_handle = BeginPaint(window_handle, &ps);
        
        W32_DrawPixelsToWindow(device_context_handle, W32_graphicsContext.pixels, W32_os.window_w, W32_os.window_h);
        
        return EndPaint(window_handle, &ps);
    }
    else if (WM_SIZE == message)
    {
        W32_UpdateWindowSize(window_handle);
        if (W32_isGameInitialised)
        {
            GME_UpdateAndRender(&W32_os);
        }
        return 0;
    }
    else if (WM_KEYDOWN == message ||
             WM_KEYUP == message ||
             WM_SYSKEYDOWN == message ||
             WM_SYSKEYUP == message)
    {
        PLT_Key key_input = 0;
        int is_down = !(l_param & (1 << 31));;
        
        if(w_param >= 'A' && w_param <= 'Z')
        {
            key_input = PLT_Key_a + (w_param - 'A');
        }
        else if (w_param >= '0' && w_param <= '9')
        {
            key_input = PLT_Key_0 + (w_param - '0');
        }
        else
        {
            if(w_param == VK_ESCAPE)
            {
                key_input = PLT_Key_esc;
            }
            else if(w_param >= VK_F1 && w_param <= VK_F12)
            {
                key_input = PLT_Key_f1 + w_param - VK_F1;
            }
            else if(w_param == VK_OEM_3)
            {
                key_input = PLT_Key_graveAccent;
            }
            else if(w_param == VK_OEM_MINUS)
            {
                key_input = PLT_Key_minus;
            }
            else if(w_param == VK_OEM_PLUS)
            {
                key_input = PLT_Key_equal;
            }
            else if(w_param == VK_BACK)
            {
                key_input = PLT_Key_backspace;
            }
            else if(w_param == VK_TAB)
            {
                key_input = PLT_Key_tab;
            }
            else if(w_param == VK_SPACE)
            {
                key_input = PLT_Key_space;
            }
            else if(w_param == VK_RETURN)
            {
                key_input = PLT_Key_enter;
            }
            else if(w_param == VK_CONTROL)
            {
                key_input = PLT_Key_ctrl;
            }
            else if(w_param == VK_SHIFT)
            {
                key_input = PLT_Key_shift;
            }
            else if(w_param == VK_MENU)
            {
                key_input = PLT_Key_alt;
            }
            else if(w_param == VK_UP)
            {
                key_input = PLT_Key_up;
            }
            else if(w_param == VK_LEFT)
            {
                key_input = PLT_Key_left;
            }
            else if(w_param == VK_DOWN)
            {
                key_input = PLT_Key_down;
            }
            else if(w_param == VK_RIGHT)
            {
                key_input = PLT_Key_right;
            }
            else if(w_param == VK_DELETE)
            {
                key_input = PLT_Key_delete;
            }
            else if(w_param == VK_PRIOR)
            {
                key_input = PLT_Key_pageUp;
            }
            else if(w_param == VK_NEXT)
            {
                key_input = PLT_Key_pageDown;
            }
            else if(w_param == VK_HOME)
            {
                key_input = PLT_Key_home;
            }
            else if(w_param == VK_END)
            {
                key_input = PLT_Key_end;
            }
            else if(w_param == VK_OEM_2)
            {
                key_input = PLT_Key_forwardSlash;
            }
            else if(w_param == VK_OEM_PERIOD)
            {
                key_input = PLT_Key_period;
            }
            else if(w_param == VK_OEM_COMMA)
            {
                key_input = PLT_Key_comma;
            }
            else if(w_param == VK_OEM_7)
            {
                key_input = PLT_Key_quote;
            }
            else if(w_param == VK_OEM_4)
            {
                key_input = PLT_Key_leftBracket;
            }
            else if(w_param == VK_OEM_6)
            {
                key_input = PLT_Key_rightBracket;
            }
        }
        
        W32_os.is_key_down[key_input] = is_down;
        
        return DefWindowProc(window_handle, message, w_param, l_param);
    }
    else if (WM_LBUTTONDOWN == message)
    {
        W32_os.is_key_down[PLT_Key_mouseLeft] = 1;
    }
    else if (WM_LBUTTONUP == message)
    {
        W32_os.is_key_down[PLT_Key_mouseLeft] = 0;
    }
    else if (WM_MBUTTONDOWN == message)
    {
        W32_os.is_key_down[PLT_Key_mouseMiddle] = 1;
    }
    else if (WM_MBUTTONUP == message)
    {
        W32_os.is_key_down[PLT_Key_mouseMiddle] = 0;
    }
    else if (WM_RBUTTONDOWN == message)
    {
        W32_os.is_key_down[PLT_Key_mouseRight] = 1;
    }
    else if (WM_RBUTTONUP == message)
    {
        W32_os.is_key_down[PLT_Key_mouseRight] = 0;
    }
    else if (WM_MOUSEMOVE == message)
    {
        POINT mouse;
        GetCursorPos(&mouse);
        ScreenToClient(window_handle, &mouse);
        W32_os.mouse_x = PLT_gameFixedW * ((float)mouse.x / (float)W32_graphicsContext.window_w);
        W32_os.mouse_y = PLT_gameFixedH * ((float)mouse.y / (float)W32_graphicsContext.window_h);
    }
    else
    {
        return DefWindowProc(window_handle, message, w_param, l_param);
    }
    return 0;
}

