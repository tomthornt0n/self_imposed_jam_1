//
// NOTE(tbt): procedural texture helpers for the falling sand simulation
//

enum
{
    FSC_lightPenetrationDepth = 10,
};

static float FSC_precomputedPerlin[PLT_gameFixedW * PLT_gameFixedH];
static void
FSC_PerlinPrecompute(void)
{
    for (int y = 0;
         y < PLT_gameFixedH;
         y += 1)
    {
        for (int x = 0;
             x < PLT_gameFixedW;
             x += 1)
        {
            FSC_precomputedPerlin[PLT_GamePixelIndex(x, y)] = RNG_Perlin2D(x, y, 1.0f, 4);
        }
    }
}

static void
FSC_Darkness(Colour *result,
             int x,
             int y)
{
    int noise = (MTH_AbsI(FSC_precomputedPerlin[PLT_GamePixelIndex(x, y)]) % 2);
    
    result->b = noise * 8;
    result->g = noise * 4;
    result->r = noise * 4;
    result->a = 255;
}

static void
FSC_PerlinNoise(const FLS_State *state,
                Colour *result,
                int x,
                int y,
                float base_b,
                float base_g,
                float base_r)
{
    float b = base_b / 255.0f;
    float g = base_g / 255.0f;
    float r = base_r / 255.0f;
    
    float perlin = FSC_precomputedPerlin[PLT_GamePixelIndex(x, y)];
    
    float noise_intensity = 0.00000002f;
    b = MTH_MinF(1.0f, b + (perlin * noise_intensity));
    g = MTH_MinF(1.0f, g + (perlin * noise_intensity));
    r = MTH_MinF(1.0f, r + (perlin * noise_intensity));
    
    result->b = 255.0f * b;
    result->g = 255.0f * g;
    result->r = 255.0f * r;
    result->a = 255;
    
    int depth = 0;
    for (int i = 0;
         i < FSC_lightPenetrationDepth;
         i += 1)
    {
        if (!FLS_CellAtHasFlag(state, x - i, y + 0, FLS_CellFlags_solid) ||
            !FLS_CellAtHasFlag(state, x - i, y - i, FLS_CellFlags_solid) ||
            !FLS_CellAtHasFlag(state, x + 0, y - i, FLS_CellFlags_solid) ||
            !FLS_CellAtHasFlag(state, x + i, y + 0, FLS_CellFlags_solid) ||
            !FLS_CellAtHasFlag(state, x + i, y + i, FLS_CellFlags_solid) ||
            !FLS_CellAtHasFlag(state, x + 0, y + i, FLS_CellFlags_solid) ||
            !FLS_CellAtHasFlag(state, x - i, y + i, FLS_CellFlags_solid) ||
            !FLS_CellAtHasFlag(state, x - i, y + 0, FLS_CellFlags_solid))
        {
            depth += 1;
        }
    }
    
    Colour darkness;
    FSC_Darkness(&darkness, x, y);
    
    unsigned char light_intensity = depth * (255 / FSC_lightPenetrationDepth);
    
    result->b = MTH_InterpolateLinearI(result->b, darkness.b, 255 - light_intensity);
    result->g = MTH_InterpolateLinearI(result->g, darkness.g, 255 - light_intensity);
    result->r = MTH_InterpolateLinearI(result->r, darkness.r, 255 - light_intensity);
}

static void
FSC_Stratified(const FLS_State *state,
               Colour *result,
               int x,
               int y,
               Colour colours[],
               size_t colour_count,
               int randomness)
{
    int depth = colour_count;
    for (int i = 0;
         i < colour_count;
         i += 1)
    {
        if (!FLS_CellAtHasFlag(state, x - i, y + 0, FLS_CellFlags_solid) ||
            !FLS_CellAtHasFlag(state, x - i, y - i, FLS_CellFlags_solid) ||
            !FLS_CellAtHasFlag(state, x + 0, y - i, FLS_CellFlags_solid) ||
            !FLS_CellAtHasFlag(state, x + i, y + 0, FLS_CellFlags_solid) ||
            !FLS_CellAtHasFlag(state, x + i, y + i, FLS_CellFlags_solid) ||
            !FLS_CellAtHasFlag(state, x + 0, y + i, FLS_CellFlags_solid) ||
            !FLS_CellAtHasFlag(state, x - i, y + i, FLS_CellFlags_solid) ||
            !FLS_CellAtHasFlag(state, x - i, y + 0, FLS_CellFlags_solid))
        {
            depth -= 1;
        }
    }
    
    int noise = RNG_RandInt2D(x, y, -randomness, randomness);
    int i = MTH_MaxI(depth + noise, 0);
    
    if (i >= colour_count)
    {
        FSC_Darkness(result, x, y);
    }
    else
    {
        result->b = colours[i].b;
        result->g = colours[i].g;
        result->r = colours[i].r;
        result->a = colours[i].a;
    }
}

static void
FSC_Liquid(const FLS_State *state,
           Colour *result,
           int x,
           int y,
           const Colour *base_colour,
           const Colour *light_colour,
           float depth_gradient_amount)

{
    result->b = base_colour->b;
    result->g = base_colour->g;
    result->r = base_colour->r;
    result->a = base_colour->a;
    
    int depth;
    for (depth = 0; FLS_CellKind_empty != FLS_CellAt(state, x, y - depth); depth += 1);
    
    int light_intensity = MTH_MaxI(255 - (depth * (255 / FSC_lightPenetrationDepth)), 0);
    result->b = MTH_InterpolateLinearI(result->b, light_colour->b, light_intensity);
    result->g = MTH_InterpolateLinearI(result->g, light_colour->g, light_intensity);
    result->r = MTH_InterpolateLinearI(result->r, light_colour->r, light_intensity);
    result->a = MTH_MinI(result->a + depth / (int)(1.0f / depth_gradient_amount), 255);
}