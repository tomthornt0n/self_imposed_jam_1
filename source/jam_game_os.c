struct Colour
{
 unsigned char b;
 unsigned char g;
 unsigned char r;
 unsigned char a;
};
typedef struct Colour Colour;
typedef struct Colour Pixel;

#define ColLit(_b, _g, _r) ((Colour[]){ (Colour){ _b, _g, _r, 255 } })

enum OS_Constants
{
 OS_gameFixedW = 480,
 OS_gameFixedH = 270,
};

static const char *OS_windowTitle = "Falling Sand";

#define OS_GamePixelIndex(_x, _y) ((_x) + (_y) * OS_gameFixedW)

typedef enum
{
 OS_Key_none,
 OS_Key_esc,
 OS_Key_f1,
 OS_Key_f2,
 OS_Key_f3,
 OS_Key_f4,
 OS_Key_f5,
 OS_Key_f6,
 OS_Key_f7,
 OS_Key_f8,
 OS_Key_f9,
 OS_Key_f10,
 OS_Key_f11,
 OS_Key_f12,
 OS_Key_graveAccent,
 OS_Key_0,
 OS_Key_1,
 OS_Key_2,
 OS_Key_3,
 OS_Key_4,
 OS_Key_5,
 OS_Key_6,
 OS_Key_7,
 OS_Key_8,
 OS_Key_9,
 OS_Key_minus,
 OS_Key_equal,
 OS_Key_backspace,
 OS_Key_delete,
 OS_Key_tab,
 OS_Key_a,
 OS_Key_b,
 OS_Key_c,
 OS_Key_d,
 OS_Key_e,
 OS_Key_f,
 OS_Key_g,
 OS_Key_h,
 OS_Key_i,
 OS_Key_j,
 OS_Key_k,
 OS_Key_l,
 OS_Key_m,
 OS_Key_n,
 OS_Key_o,
 OS_Key_p,
 OS_Key_q,
 OS_Key_r,
 OS_Key_s,
 OS_Key_t,
 OS_Key_u,
 OS_Key_v,
 OS_Key_w,
 OS_Key_x,
 OS_Key_y,
 OS_Key_z,
 OS_Key_space,
 OS_Key_enter,
 OS_Key_ctrl,
 OS_Key_shift,
 OS_Key_alt,
 OS_Key_up,
 OS_Key_left,
 OS_Key_down,
 OS_Key_right,
 OS_Key_pageUp,
 OS_Key_pageDown,
 OS_Key_home,
 OS_Key_end,
 OS_Key_forwardSlash,
 OS_Key_period,
 OS_Key_comma,
 OS_Key_quote,
 OS_Key_leftBracket,
 OS_Key_rightBracket,
 
 OS_Key_mouseLeft,
 OS_Key_mouseMiddle,
 OS_Key_mouseRight,
 
 OS_Key_MAX,
} OS_Key;

typedef struct
{
 Pixel *pixels;
 size_t window_w;
 size_t window_h;
 double dt;
 int mouse_x;
 int mouse_y;
 int is_key_down[OS_Key_MAX];
} OS_GameInput;

void GME_Initialise(void);
void GME_UpdateAndRender(const OS_GameInput *input);

typedef enum
{
 OS_ResourceID_bg,
} OS_ResourceID;

void *OS_ResourceGet(OS_ResourceID id);