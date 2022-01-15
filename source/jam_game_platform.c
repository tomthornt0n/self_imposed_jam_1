//
// NOTE(tbt): interface between platform layer and game
//

#include <stdint.h>
#include <stddef.h>

#define ArrayCount(_a) (sizeof(_a) / sizeof(_a[0]))

#define Bit(_a) (1 << (_a))

struct Colour
{
    unsigned char b;
    unsigned char g;
    unsigned char r;
    unsigned char a;
};
typedef struct Colour Colour;
typedef struct Colour Pixel;

#define ColLit(_b, _g, _r, _a) ((Colour[]){ (Colour){ _b, _g, _r, _a } })

enum PLT_Constants
{
    // NOTE(tbt): game runs at fixed resolution - gets stretched by platform layer
    PLT_gameFixedW = 480,
    PLT_gameFixedH = 270,
};

static const char *PLT_windowTitle = "Falling Sand";

#define PLT_GamePixelIndex(_x, _y) ((_x) + (_y) * PLT_gameFixedW)
#define PLT_GameHasPixel(_x, _y) ((_x) >= 0 && (_x) < PLT_gameFixedW && (_y) >= 0 && (_y) < PLT_gameFixedH)

typedef enum
{
    PLT_Key_none,
    PLT_Key_esc,
    PLT_Key_f1,
    PLT_Key_f2,
    PLT_Key_f3,
    PLT_Key_f4,
    PLT_Key_f5,
    PLT_Key_f6,
    PLT_Key_f7,
    PLT_Key_f8,
    PLT_Key_f9,
    PLT_Key_f10,
    PLT_Key_f11,
    PLT_Key_f12,
    PLT_Key_graveAccent,
    PLT_Key_0,
    PLT_Key_1,
    PLT_Key_2,
    PLT_Key_3,
    PLT_Key_4,
    PLT_Key_5,
    PLT_Key_6,
    PLT_Key_7,
    PLT_Key_8,
    PLT_Key_9,
    PLT_Key_minus,
    PLT_Key_equal,
    PLT_Key_backspace,
    PLT_Key_delete,
    PLT_Key_tab,
    PLT_Key_a,
    PLT_Key_b,
    PLT_Key_c,
    PLT_Key_d,
    PLT_Key_e,
    PLT_Key_f,
    PLT_Key_g,
    PLT_Key_h,
    PLT_Key_i,
    PLT_Key_j,
    PLT_Key_k,
    PLT_Key_l,
    PLT_Key_m,
    PLT_Key_n,
    PLT_Key_o,
    PLT_Key_p,
    PLT_Key_q,
    PLT_Key_r,
    PLT_Key_s,
    PLT_Key_t,
    PLT_Key_u,
    PLT_Key_v,
    PLT_Key_w,
    PLT_Key_x,
    PLT_Key_y,
    PLT_Key_z,
    PLT_Key_space,
    PLT_Key_enter,
    PLT_Key_ctrl,
    PLT_Key_shift,
    PLT_Key_alt,
    PLT_Key_up,
    PLT_Key_left,
    PLT_Key_down,
    PLT_Key_right,
    PLT_Key_pageUp,
    PLT_Key_pageDown,
    PLT_Key_home,
    PLT_Key_end,
    PLT_Key_forwardSlash,
    PLT_Key_period,
    PLT_Key_comma,
    PLT_Key_quote,
    PLT_Key_leftBracket,
    PLT_Key_rightBracket,
    
    PLT_Key_mouseLeft,
    PLT_Key_mouseMiddle,
    PLT_Key_mouseRight,
    
    PLT_Key_MAX,
} PLT_Key;

typedef struct
{
    Pixel *pixels;
    
    double dt;
    
    int mouse_x;
    int mouse_y;
    int is_key_down[PLT_Key_MAX];
} PLT_GameInput;

void GME_Initialise(const PLT_GameInput *input);
void GME_UpdateAndRender(const PLT_GameInput *input);
