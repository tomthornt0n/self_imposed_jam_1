//
// NOTE(tbt): simple, kind of hacky, falling sand sim
//

typedef enum
{
    FLS_CellKind_empty,
    FLS_CellKind_sand,
    FLS_CellKind_water,
    FLS_CellKind_stone,
    FLS_CellKind_gas,
    FLS_CellKind_dirt,
    
    FLS_CellKind_MAX,
} FLS_CellKind;

typedef unsigned int FLS_CellFlags;
typedef enum
{
    FLS_CellFlags_fallVertical   = 1 << 0,
    FLS_CellFlags_fallDiagonal   = 1 << 1,
    FLS_CellFlags_fallHorizontal = 1 << 2,
    FLS_CellFlags_rise           = 1 << 3,
    FLS_CellFlags_solid          = 1 << 4,
    FLS_CellFlags_destructable   = 1 << 5,
    FLS_CellFlags_exists         = 1 << 6,
} FLS_CellFlags_ENUM;

typedef struct
{
    FLS_CellKind cells[PLT_gameFixedW * PLT_gameFixedH];
    Pixel texture_buffer[PLT_gameFixedW * PLT_gameFixedH];
    RES_Texture texture;
} FLS_State;

#define FLS_CellAt(_state, _x, _y) ((_state)->cells[PLT_GamePixelIndex(MTH_ClampI(_x, 0, PLT_gameFixedW - 1), MTH_ClampI(_y, 0, PLT_gameFixedH - 1))])

#define FLS_IsCellInBounds(_x, _y) PLT_GameHasPixel(_x, _y)

typedef void ( *FLS_GetColourCallback) (const FLS_State *state, Colour *result, int x, int y);

static void FLS_SandGetColour(const FLS_State *state, Colour *result, int x, int y);
static void FLS_WaterGetColour(const FLS_State *state, Colour *result, int x, int y);
static void FLS_StoneGetColour(const FLS_State *state, Colour *result, int x, int y);
static void FLS_GasGetColour(const FLS_State *state, Colour *result, int x, int y);
static void FLS_DirtGetColour(const FLS_State *state, Colour *result, int x, int y);

struct FLS_Cell
{
    FLS_GetColourCallback get_colour;
    FLS_CellFlags flags;
    int density;
} FLS_cellTable[FLS_CellKind_MAX] =
{
    [FLS_CellKind_sand] =
    {
        .get_colour = FLS_SandGetColour,
        .flags = (FLS_CellFlags_exists |
                  FLS_CellFlags_fallVertical |
                  FLS_CellFlags_fallDiagonal |
                  FLS_CellFlags_solid |
                  FLS_CellFlags_destructable),
        .density = 2,
    },
    
    [FLS_CellKind_water] =
    {
        .get_colour = FLS_WaterGetColour,
        .flags = (FLS_CellFlags_exists |
                  FLS_CellFlags_fallVertical |
                  FLS_CellFlags_fallDiagonal |
                  FLS_CellFlags_fallHorizontal),
        .density = 1,
    },
    
    [FLS_CellKind_stone] = 
    {
        .get_colour = FLS_StoneGetColour,
        .flags = FLS_CellFlags_exists | FLS_CellFlags_solid,
        .density = 999,
    },
    
    [FLS_CellKind_dirt] = 
    {
        .get_colour = FLS_DirtGetColour,
        .flags = (FLS_CellFlags_exists |
                  FLS_CellFlags_fallVertical |
                  FLS_CellFlags_fallDiagonal |
                  FLS_CellFlags_solid |
                  FLS_CellFlags_destructable),
        .density = 2,
    },
};

static int
FLS_CellHasFlag(FLS_CellKind cell,
                FLS_CellFlags flags)
{
    return !!(FLS_cellTable[cell].flags & (flags));
}


static int
FLS_CellAtHasFlag(const FLS_State *state,
                  int x, int y,
                  FLS_CellFlags flags)
{
    if (FLS_IsCellInBounds(x, y))
    {
        return FLS_CellHasFlag(FLS_CellAt(state, x, y), flags);
    }
    else
    {
        FLS_CellFlags dummy_flags = FLS_CellFlags_solid;
        return (dummy_flags & flags);
    }
    
}

#include "jam_game_falling_sand_colours.c"

static void
FLS_SandGetColour(const FLS_State *state,
                  Colour *result,
                  int x,
                  int y)
{
    float b =  69.0f;
    float g = 138.0f;
    float r = 238.0f;
    
    FSC_PerlinNoise(state, result, x, y, b, g, r);
}

static void
FLS_WaterGetColour(const FLS_State *state,
                   Colour *result,
                   int x,
                   int y)
{
    Colour base =
    { 
        .b = 245,
        .g = 135,
        .r =  66,
        .a = 120,
    };
    
    Colour light =
    {
        .b = 255,
        .g = 245,
        .r = 245,
        .a = 180,
    };
    
    float depth_gradient_amount = 0.25f;
    
    FSC_Liquid(state, result, x, y, &base, &light, depth_gradient_amount);
}

static void
FLS_StoneGetColour(const FLS_State *state,
                   Colour *result,
                   int x,
                   int y)
{
    float b = 66.0f;
    float g = 65.0f;
    float r = 65.0f;
    
    FSC_PerlinNoise(state, result, x, y, b, g, r);
}

static void
FLS_GasGetColour(const FLS_State *state,
                 Colour *result,
                 int x,
                 int y)
{
    result->b = 135;
    result->g = 245;
    result->r =  66;
    result->a =  60;
}

static void
FLS_DirtGetColour(const FLS_State *state,
                  Colour *result,
                  int x,
                  int y)
{
    enum { colour_count = 10 };
    Colour colours[colour_count] =
    {
        {  64, 133,  79, 255 },
        {  34,  78,  44, 255 },
        {  85, 100, 121, 255 },
        {  62,  73,  88, 255 },
        {  37,  33,  50, 255 },
        {  37,  27,  41, 255 },
        {  29,  19,  27, 255 },
        {  20,  13,  13, 255 },
        {   8,   4,   4, 255 },
        {   0,   0,   0, 255 },
    };
    
    FSC_Stratified(state, result, x, y, colours, colour_count, 1);
}

static void
FLS_GetColour(const FLS_State *state,
              Colour *result,
              int x,
              int y)
{
    if (FLS_IsCellInBounds(x, y) &&
        NULL != FLS_cellTable[FLS_CellAt(state, x, y)].get_colour)
    {
        FLS_cellTable[FLS_CellAt(state, x, y)].get_colour(state, result, x, y);
    }
    else
    {
        result->b = 0;
        result->g = 0;
        result->r = 0;
        result->a = 0;
    }
}

static void
FLS_CellSwap(FLS_State *state,
             int x0, int y0,
             int x1, int y1)
{
    FLS_CellKind temp = FLS_CellAt(state, x0, y0);
    FLS_CellAt(state, x0, y0) = FLS_CellAt(state, x1, y1);
    FLS_CellAt(state, x1, y1) = temp;
}

static int
FLS_CellCanFallTo(FLS_State *state,
                  int x0, int y0,
                  int x1, int y1)
{
    if (FLS_CellAtHasFlag(state, x0, y0, FLS_CellFlags_rise))
    {
        return (FLS_IsCellInBounds(x0, y0) &&
                FLS_IsCellInBounds(x1, y1) &&
                ((!FLS_CellAtHasFlag(state, x1, y1, FLS_CellFlags_solid)) &&
                 FLS_cellTable[FLS_CellAt(state, x1, y1)].density >
                 FLS_cellTable[FLS_CellAt(state, x0, y0)].density));
    }
    else
    {
        return (FLS_IsCellInBounds(x0, y0) &&
                FLS_IsCellInBounds(x1, y1) &&
                (FLS_cellTable[FLS_CellAt(state, x1, y1)].density <
                 FLS_cellTable[FLS_CellAt(state, x0, y0)].density));
    }
}

static void
FLS_SetCells(FLS_State *state,
             int x, int y,
             int radius,
             FLS_CellKind kind)
{
    if (radius)
    {
        for (int y0 = -radius;
             y0 <= radius;
             y0 += 1)
        {
            for (int x0 = -radius;
                 x0 <= radius;
                 x0 += 1)
            {
                if (x0 * x0 + y0 * y0 <= radius * radius &&
                    FLS_IsCellInBounds(x + x0, y + y0))
                {
                    FLS_CellAt(state, x + x0, y + y0) = kind;
                }
            }
        }
    }
    else if (FLS_IsCellInBounds(x, y))
    {
        FLS_CellAt(state, x, y) = kind;
    }
}

static void
FLS_StateMake(FLS_State *state)
{
    state->texture.buffer = state->texture_buffer;
    state->texture.w = PLT_gameFixedW;
    state->texture.h = PLT_gameFixedH;
}

static void
FLS_StateFromTexture(FLS_State *state,
                     const RES_Texture *texture)
{
    FLS_StateMake(state);
    
    int max_x = MTH_MinI(texture->w, PLT_gameFixedW);
    int max_y = MTH_MinI(texture->h, PLT_gameFixedH);
    
    for (int x = 0;
         x < max_x;
         x += 1)
    {
        for (int y = 0;
             y < max_y;
             y += 1)
        {
            unsigned long *pixel = (unsigned long *)(&texture->buffer[x + y * texture->w]);
            
            if (*pixel == 0xFF000000)
            {
                FLS_CellAt(state, x, y) = FLS_CellKind_stone;
            }
            else if (*pixel == 0xFFFFFF00)
            {
                FLS_CellAt(state, x, y) = FLS_CellKind_sand;
            }
            else if (*pixel == 0xFF0000FF)
            {
                FLS_CellAt(state, x, y) = FLS_CellKind_water;
            }
            else if (*pixel == 0xFF00FF00)
            {
                FLS_CellAt(state, x, y) = FLS_CellKind_gas;
            }
            else if (*pixel == 0xFFFF0000)
            {
                FLS_CellAt(state, x, y) = FLS_CellKind_dirt;
            }
            else
            {
                FLS_CellAt(state, x, y) = FLS_CellKind_empty;
            }
        }
    }
}

static void
FLS_Update(const PLT_GameInput *input,
           FLS_State *state)
{
    int has_already_been_updated[PLT_gameFixedW * PLT_gameFixedH] = {0};
    
    Pixel *pixel_buffer = (Pixel *)state->texture.buffer;
    
    for (int y0 = 0;
         y0 < PLT_gameFixedH;
         y0 += 1)
    {
        for (int x0 = 0;
             x0 < PLT_gameFixedW;
             x0 += 1)
        {
            int x1 = x0, y1 = y0;
            
            if (has_already_been_updated[PLT_GamePixelIndex(x0, y0)]) { continue; }
            
            if (FLS_CellAtHasFlag(state, x0, y0, FLS_CellFlags_rise))
            {
                if (FLS_CellCanFallTo(state, x0, y0, x0, y0 - 1))
                {
                    y1 = y0 - 1;
                    goto move_cell;
                }
                else if (FLS_CellCanFallTo(state, x0, y0, x0 - 1, y0 - 1))
                {
                    if (FLS_CellCanFallTo(state, x0, y0, x0 + 1, y0 - 1) &&
                        RNG_RandIntNext(0, 2))
                    {
                        x1 = x0 + 1;
                        y1 = y0 - 1;
                    }
                    else
                    {
                        x1 = x0 - 1;
                        y1 = y0 - 1;
                    }
                    goto move_cell;
                }
                else if (FLS_CellCanFallTo(state, x0, y0, x0 + 1, y0 - 1))
                {
                    x1 = x0 + 1;
                    y1 = y0 - 1;
                    goto move_cell;
                }
            }
            
            if (FLS_CellAtHasFlag(state, x0, y0, FLS_CellFlags_fallVertical))
            {
                if (FLS_CellCanFallTo(state, x0, y0, x0, y0 + 1))
                {
                    y1 = y0 + 1;
                    goto move_cell;
                }
            }
            
            if (FLS_CellAtHasFlag(state, x0, y0, FLS_CellFlags_fallDiagonal))
            {
                if (FLS_CellCanFallTo(state, x0, y0, x0 - 1, y0 + 1))
                {
                    if (FLS_CellCanFallTo(state, x0, y0, x0 + 1, y0 + 1) &&
                        RNG_RandIntNext(0, 2))
                    {
                        x1 = x0 + 1;
                        y1 = y0 + 1;
                    }
                    else
                    {
                        x1 = x0 - 1;
                        y1 = y0 + 1;
                    }
                    goto move_cell;
                }
                else if (FLS_CellCanFallTo(state, x0, y0, x0 + 1, y0 + 1))
                {
                    x1 = x0 + 1;
                    y1 = y0 + 1;
                    goto move_cell;
                }
            }
            
            if (FLS_CellAtHasFlag(state, x0, y0, FLS_CellFlags_fallHorizontal))
            {
                if (FLS_CellCanFallTo(state, x0, y0, x0 + 1, y0))
                {
                    if (FLS_CellCanFallTo(state, x0, y0, x0 - 1, y0) &&
                        RNG_RandIntNext(0, 2))
                    {
                        x1 = x0 - 1;
                    }
                    else
                    {
                        x1 = x0 + 1;
                    }
                    goto move_cell;
                }
                else if (FLS_CellCanFallTo(state, x0, y0, x0 - 1, y0))
                {
                    x1 = x0 - 1;
                    goto move_cell;
                }
            }
            
            has_already_been_updated[PLT_GamePixelIndex(x0, y0)] = 1;
            FLS_GetColour(state, &pixel_buffer[PLT_GamePixelIndex(x0, y0)], x0, y0);
            continue;
            
            move_cell:
            has_already_been_updated[PLT_GamePixelIndex(x0, y0)] = 1;
            has_already_been_updated[PLT_GamePixelIndex(x1, y1)] = 1;
            FLS_CellSwap(state, x0, y0, x1, y1);
            FLS_GetColour(state, &pixel_buffer[PLT_GamePixelIndex(x0, y0)], x0, y0);
            FLS_GetColour(state, &pixel_buffer[PLT_GamePixelIndex(x1, y1)], x1, y1);
        }
    }
}
