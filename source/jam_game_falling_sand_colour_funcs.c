static Colour
FLS_EmptyGetColour(const FLS_State *state,
                   int x,
                   int y)
{
 return (Colour){ 0, 0, 0, 0 };
}

static Colour
FLS_SandGetColour(const FLS_State *state,
                  int x,
                  int y)
{
 float b = 66.0f  / 255.0f;
 float g = 135.0f / 255.0f;
 float r = 245.0f / 255.0f;
 
 b = MAT_MinF(1.0f, b + (RNG_Perlin2D(x, y, 0.9f, 4) * 0.00000002f));
 g = MAT_MinF(1.0f, g + (RNG_Perlin2D(x, y, 0.9f, 4) * 0.00000002f));
 r = MAT_MinF(1.0f, r + (RNG_Perlin2D(x, y, 0.9f, 4) * 0.00000002f));
 
 return (Colour){ 255 * b, 255 * g, 255 * r, 255 };
}

static Colour
FLS_WaterGetColour(const FLS_State *state,
                   int x,
                   int y)
{
 Colour result = (Colour){ 245, 135, 66, 120 };
 Colour light = (Colour){ 255, 245, 245, 180 };
 const int light_penetration_depth = 4;
 
 int depth;
 for (depth = 0;
      FLS_CellKind_empty != FLS_CellAt(state, x, y - depth);
      depth += 1);
 
 
 int light_intensity = MAT_MinI(depth, light_penetration_depth) * (255 / light_penetration_depth);
 result.b = ((light_intensity * (result.b - light.b)) >> 8) + light.b;
 result.g = ((light_intensity * (result.g - light.g)) >> 8) + light.g;
 result.r = ((light_intensity * (result.r - light.r)) >> 8) + light.r;
 result.a = ((light_intensity * (result.a - light.a)) >> 8) + light.a;
 
 result.a = MAT_MinI(result.a + depth / 4, 255);
 
 return result;
}

static Colour
FLS_StoneGetColour(const FLS_State *state,
                   int x,
                   int y)
{
 float b = 56.0f  / 255.0f;
 float g = 55.0f / 255.0f;
 float r = 55.0f / 255.0f;
 
 b = MAT_MinF(1.0f, b + (RNG_Perlin2D(x, y, 1.0f, 4) * 0.00000002f));
 g = MAT_MinF(1.0f, g + (RNG_Perlin2D(x, y, 1.0f, 4) * 0.00000002f));
 r = MAT_MinF(1.0f, r + (RNG_Perlin2D(x, y, 1.0f, 4) * 0.00000002f));
 
 return (Colour){ 255 * b, 255 * g, 255 * r, 255 };
}

static Colour
FLS_GasGetColour(const FLS_State *state,
                 int x,
                 int y)
{
 Colour result = (Colour){ 135, 245, 66, 60 };
 
 for (int i = 0;
      i < 8;
      i += 1)
 {
  if (FLS_CellKind_gas == state->cells[OS_GamePixelIndex(x - i, y + 0)] &&
      FLS_CellKind_gas == state->cells[OS_GamePixelIndex(x - i, y - i)] &&
      FLS_CellKind_gas == state->cells[OS_GamePixelIndex(x + 0, y - i)] &&
      FLS_CellKind_gas == state->cells[OS_GamePixelIndex(x + i, y + 0)] &&
      FLS_CellKind_gas == state->cells[OS_GamePixelIndex(x + i, y + i)] &&
      FLS_CellKind_gas == state->cells[OS_GamePixelIndex(x + 0, y + i)] &&
      FLS_CellKind_gas == state->cells[OS_GamePixelIndex(x - i, y + i)] &&
      FLS_CellKind_gas == state->cells[OS_GamePixelIndex(x - i, y + 0)])
  {
   result.a += 8;
  }
 }
 
 return result;
}

static Colour
FLS_DirtGetColour(const FLS_State *state,
                  int x,
                  int y)
{
 
 
 return (Colour){ 255, 255, 255, 255 };
}