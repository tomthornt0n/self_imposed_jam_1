static inline void
RDR_SetPixel(const OS_GameInput *input,
             size_t x, size_t y,
             const Pixel *pixel)
{
 Pixel current = input->pixels[OS_GamePixelIndex(x, y)];
 int src_alpha = 255 - pixel->a;
 
 input->pixels[OS_GamePixelIndex(x, y)].b = ((src_alpha * (current.b - pixel->b)) >> 8) + pixel->b;
 input->pixels[OS_GamePixelIndex(x, y)].g = ((src_alpha * (current.g - pixel->g)) >> 8) + pixel->g;
 input->pixels[OS_GamePixelIndex(x, y)].r = ((src_alpha * (current.r - pixel->r)) >> 8) + pixel->r;
 input->pixels[OS_GamePixelIndex(x, y)].a = 255;
}

static void
RDR_ClearScreen(const OS_GameInput *input)
{
 memset(input->pixels, 0, input->window_w * input->window_h * sizeof(Pixel));
}

static void
RDR_DrawRectangleFill(const OS_GameInput *input,
                      const Rect *rect,
                      const Colour *colour)
{
 for (int y = rect->y0;
      y < rect->y1 && y < input->window_h;
      y += 1)
 {
  for (int x = rect->x0;
       x < rect->x1 && x < input->window_w;
       x += 1)
  {
   RDR_SetPixel(input, x, y, colour);
  }
 }
}

static void
RDR_DrawTexture(const OS_GameInput *input,
                const RES_Texture *texture,
                int x,
                int y)
{
 for (int y0 = 0;
      y0 < texture->h;
      y0 += 1)
 {
  for (int x0 = 0;
       x0 < texture->w;
       x0 += 1)
  {
   RDR_SetPixel(input, x + x0, y + y0,
                &texture->buffer[x0 + y0 * texture->w]);
  }
 }
}

static void
RDR_DrawFallingSand(const OS_GameInput *input,
                    const FLS_State *state)
{
 for (int y = 0;
      y < OS_gameFixedH;
      y += 1)
 {
  for (int x = 0;
       x < OS_gameFixedW;
       x += 1)
  {
   Colour result = FLS_GetColour(state, x, y);
   RDR_SetPixel(input, x, y, &result);
  }
 }
}