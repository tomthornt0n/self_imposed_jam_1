static inline void
R_SetPixel(const OS_GameInput *input,
           size_t x, size_t y,
           const Pixel *pixel)
{
 Pixel current;
 memcpy(&current, &input->pixels[OS_GamePixelIndex(x, y)], sizeof(current));
 int src_alpha = 255 - pixel->a;
 
 input->pixels[OS_GamePixelIndex(x, y)].b = ((src_alpha * (current.b - pixel->b)) >> 8) + pixel->b;
 input->pixels[OS_GamePixelIndex(x, y)].g = ((src_alpha * (current.g - pixel->g)) >> 8) + pixel->g;
 input->pixels[OS_GamePixelIndex(x, y)].r = ((src_alpha * (current.r - pixel->r)) >> 8) + pixel->r;
 input->pixels[OS_GamePixelIndex(x, y)].a = 255;
}

static void
R_ClearScreen(const OS_GameInput *input)
{
 memset(input->pixels, 0, input->window_w * input->window_h * sizeof(Pixel));
}

static void
R_DrawRectangleFill(const OS_GameInput *input,
                    const Rect *rect,
                    const Colour *colour)
{
 for (size_t y = rect->y0;
      y < rect->y1 && y < input->window_h;
      y += 1)
 {
  for (size_t x = rect->x0;
       x < rect->x1 && x < input->window_w;
       x += 1)
  {
   R_SetPixel(input, x, y, colour);
  }
 }
}

static void
R_DrawFallingSand(const OS_GameInput *input,
                  const FS_State *state)
{
 for (int y = 0;
      y < OS_gameFixedH;
      y += 1)
 {
  for (int x = 0;
       x < OS_gameFixedW;
       x += 1)
  {
   Colour result;
   
   Colour centre = FS_GetColour(state, x, y);
   Colour nw = FS_GetColour(state, x - 1, y - 1);
   Colour nn = FS_GetColour(state, x + 0, y - 1);
   Colour ne = FS_GetColour(state, x + 1, y - 1);
   Colour ee = FS_GetColour(state, x + 1, y + 0);
   Colour se = FS_GetColour(state, x + 1, y + 1);
   Colour ss = FS_GetColour(state, x + 0, y + 1);
   Colour sw = FS_GetColour(state, x - 1, y + 1);
   Colour ww = FS_GetColour(state, x - 1, y + 0);
   
   result.r = (centre.r / 3 +
               nw.r / 12 +
               nn.r / 12 +
               ne.r / 12 +
               ee.r / 12 +
               se.r / 12 +
               ss.r / 12 +
               sw.r / 12 +
               sw.r / 12);
   
   result.g = (centre.g / 3 +
               nw.g / 12 +
               nn.g / 12 +
               ne.g / 12 +
               ee.g / 12 +
               se.g / 12 +
               ss.g / 12 +
               sw.g / 12 +
               sw.g / 12);
   
   result.b = (centre.b / 3 +
               nw.b / 12 +
               nn.b / 12 +
               ne.b / 12 +
               ee.b / 12 +
               se.b / 12 +
               ss.b / 12 +
               sw.b / 12 +
               sw.b / 12);
   
   result.a = (centre.a / 3 +
               nw.a / 12 +
               nn.a / 12 +
               ne.a / 12 +
               ee.a / 12 +
               se.a / 12 +
               ss.a / 12 +
               sw.a / 12 +
               sw.a / 12);
   
   //result.a = 255;
   
   R_SetPixel(input, x, y, &result);
  }
 }
 
}