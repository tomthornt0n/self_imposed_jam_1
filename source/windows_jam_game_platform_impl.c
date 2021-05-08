#include <windows.h>
#include <windowsx.h>

#include <stdio.h>

#include "jam_game_platform.c"

typedef struct
{
 size_t window_w;
 size_t window_h;
 Pixel pixels[348364800];
 
 BITMAPINFO bitmap_info;
 
 PLT_GameInput game_input;
 
 int is_game_intialised;
} Windows_GlobalState;
Windows_GlobalState Windows_globalState;

static void
Windows_InitialiseRendering(void)
{
 BITMAPINFOHEADER *bih = &Windows_globalState.bitmap_info.bmiHeader;
 bih->biSize = sizeof(BITMAPINFOHEADER);
 bih->biPlanes = 1;
 bih->biBitCount = 32;
 bih->biCompression = BI_RGB;
 bih->biSizeImage = 0;
 bih->biClrUsed = 0;
 bih->biClrImportant = 0;
 
 Windows_globalState.game_input.pixels = Windows_globalState.pixels;
}

static void
Windows_DrawPixelsToWindow(HDC device_context_handle, Pixel *pixels, size_t w, size_t h)
{
 Windows_globalState.bitmap_info.bmiHeader.biWidth = w;
 Windows_globalState.bitmap_info.bmiHeader.biHeight = -h;
 StretchDIBits(device_context_handle,
               0, 0, Windows_globalState.window_w, Windows_globalState.window_h,
               0, 0, w, h,
               pixels,
               &Windows_globalState.bitmap_info,
               DIB_RGB_COLORS,
               SRCCOPY);
}

static void Windows_UpdateWindowSize(HWND window_handle)
{
 RECT rc;
 GetClientRect(window_handle, &rc);
 Windows_globalState.window_w = rc.right - rc.left;
 Windows_globalState.window_h = rc.bottom - rc.top;
}

LRESULT CALLBACK Windows_EventCallback(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param);

int
WINAPI WinMain(HINSTANCE instance_handle,
               HINSTANCE prev_instance_handle,
               LPSTR command_line,
               int show_mode)
{
 
 Windows_InitialiseRendering();
 
 char *window_class_name = "JamGame";
 
 WNDCLASSEX window_class = {0};
 window_class.cbSize = sizeof(WNDCLASSEX);
 window_class.style = CS_HREDRAW|CS_VREDRAW;
 window_class.lpfnWndProc = Windows_EventCallback;
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
 ShowWindow(window_handle, SW_SHOW);
 
 Windows_UpdateWindowSize(window_handle);
 
 Windows_globalState.game_input.window_w = PLT_gameFixedW;
 Windows_globalState.game_input.window_h = PLT_gameFixedH;
 
 GME_Initialise();
 Windows_globalState.is_game_intialised = 1;
 
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
   GME_UpdateAndRender(&Windows_globalState.game_input);
  }
  
  HDC device_context_handle = GetDC(window_handle);
  {
   Windows_DrawPixelsToWindow(device_context_handle, Windows_globalState.pixels, Windows_globalState.game_input.window_w, Windows_globalState.game_input.window_h);
  } ReleaseDC(window_handle, device_context_handle);
  
  QueryPerformanceCounter(&end_t);
  Windows_globalState.game_input.dt = (double)(end_t.QuadPart - start_t.QuadPart) / (double)freq.QuadPart;
 }
 return exit_code;
}

LRESULT CALLBACK
Windows_EventCallback(HWND window_handle,
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
  
  Windows_DrawPixelsToWindow(device_context_handle, Windows_globalState.pixels, Windows_globalState.game_input.window_w, Windows_globalState.game_input.window_h);
  
  return EndPaint(window_handle, &ps);
 }
 else if (WM_SIZE == message)
 {
  Windows_UpdateWindowSize(window_handle);
  if (Windows_globalState.is_game_intialised)
  {
   GME_UpdateAndRender(&Windows_globalState.game_input);
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
  
  Windows_globalState.game_input.is_key_down[key_input] = is_down;
  
  return DefWindowProc(window_handle, message, w_param, l_param);
 }
 else if (WM_LBUTTONDOWN == message)
 {
  Windows_globalState.game_input.is_key_down[PLT_Key_mouseLeft] = 1;
 }
 else if (WM_LBUTTONUP == message)
 {
  Windows_globalState.game_input.is_key_down[PLT_Key_mouseLeft] = 0;
 }
 else if (WM_MBUTTONDOWN == message)
 {
  Windows_globalState.game_input.is_key_down[PLT_Key_mouseMiddle] = 1;
 }
 else if (WM_MBUTTONUP == message)
 {
  Windows_globalState.game_input.is_key_down[PLT_Key_mouseMiddle] = 0;
 }
 else if (WM_RBUTTONDOWN == message)
 {
  Windows_globalState.game_input.is_key_down[PLT_Key_mouseRight] = 1;
 }
 else if (WM_RBUTTONUP == message)
 {
  Windows_globalState.game_input.is_key_down[PLT_Key_mouseRight] = 0;
 }
 else if (WM_MOUSEMOVE == message)
 {
  POINT mouse;
  GetCursorPos(&mouse);
  ScreenToClient(window_handle, &mouse);
  Windows_globalState.game_input.mouse_x = PLT_gameFixedW * ((float)mouse.x / (float)Windows_globalState.window_w);
  Windows_globalState.game_input.mouse_y = PLT_gameFixedH * ((float)mouse.y / (float)Windows_globalState.window_h);
 }
 else
 {
  return DefWindowProc(window_handle, message, w_param, l_param);
 }
 return 0;
}

