//
// NOTE(tbt): archetypal procedural texture gen helpers for the falling sand simulation
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

static Colour
FSC_Darkness(int x,
             int y)
{
 int noise = (MTH_AbsI(FSC_precomputedPerlin[PLT_GamePixelIndex(x, y)]) % 2);
 return (Colour){ noise * 8, noise * 4, noise * 4, 255 };
}

static Colour
FSC_PerlinNoise(const FLS_State *state,
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
 
 b = MTH_MinF(1.0f, b + (perlin * 0.00000002f));
 g = MTH_MinF(1.0f, g + (perlin * 0.00000002f));
 r = MTH_MinF(1.0f, r + (perlin * 0.00000002f));
 
 Colour result = (Colour){ 255 * b, 255 * g, 255 * r, 255 };
 
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
 
 Colour darkness = FSC_Darkness(x, y);
 unsigned char light_intensity = depth * (255 / FSC_lightPenetrationDepth);
 result.b = ((light_intensity * (result.b - darkness.b)) >> 8) + darkness.b;
 result.g = ((light_intensity * (result.g - darkness.g)) >> 8) + darkness.g;
 result.r = ((light_intensity * (result.r - darkness.r)) >> 8) + darkness.r;
 
 return result;
}

static Colour
FSC_Stratified(const FLS_State *state,
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
  return FSC_Darkness(x, y);
 }
 else
 {
  return colours[i];
 }
}

static Colour FSC_Liquid(const FLS_State *state,
                         int x,
                         int y,
                         const Colour *base_colour,
                         const Colour *light_colour,
                         float depth_gradient_amount)

{
 Colour result = *base_colour;
 
 int depth;
 for (depth = 0; FLS_CellKind_empty != FLS_CellAt(state, x, y - depth); depth += 1);
 
 int light_intensity = MTH_MinI(depth * (255 / FSC_lightPenetrationDepth), 255);
 result.b = ((light_intensity * (result.b - light_colour->b)) >> 8) + light_colour->b;
 result.g = ((light_intensity * (result.g - light_colour->g)) >> 8) + light_colour->g;
 result.r = ((light_intensity * (result.r - light_colour->r)) >> 8) + light_colour->r;
 result.a = ((light_intensity * (result.a - light_colour->a)) >> 8) + light_colour->a;
 
 result.a = MTH_MinI(result.a + depth / (int)(1.0f / depth_gradient_amount), 255);
 
 return result;
}