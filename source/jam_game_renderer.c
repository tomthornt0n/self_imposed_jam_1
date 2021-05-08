//
// NOTE(tbt): writes pixels into a buffer
//

static inline void
RDR_SetPixel(const PLT_GameInput *input,
             size_t x, size_t y,
             const Pixel *pixel)
{
 Pixel current = input->pixels[PLT_GamePixelIndex(x, y)];
 int src_alpha = 255 - pixel->a;
 
 input->pixels[PLT_GamePixelIndex(x, y)].b = ((src_alpha * (current.b - pixel->b)) >> 8) + pixel->b;
 input->pixels[PLT_GamePixelIndex(x, y)].g = ((src_alpha * (current.g - pixel->g)) >> 8) + pixel->g;
 input->pixels[PLT_GamePixelIndex(x, y)].r = ((src_alpha * (current.r - pixel->r)) >> 8) + pixel->r;
 input->pixels[PLT_GamePixelIndex(x, y)].a = 255;
}

static void
RDR_ClearScreen(const PLT_GameInput *input)
{
 memset(input->pixels, 0, input->window_w * input->window_h * sizeof(Pixel));
}

static void
RDR_DrawRectangleFill(const PLT_GameInput *input,
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

typedef Rect RDR_SubTexture;

static void
RDR_DrawSubTexture(const PLT_GameInput *input,
                   const RES_Texture *texture,
                   const RDR_SubTexture *sub_texture,
                   int x,
                   int y)
{
 for (int y0 = sub_texture->y0;
      y0 < sub_texture->y1;
      y0 += 1)
 {
  for (int x0 = sub_texture->x0;
       x0 < sub_texture->x1;
       x0 += 1)
  {
   RDR_SetPixel(input,
                x + x0 - sub_texture->x0,
                y + y0 - sub_texture->y0,
                &texture->buffer[x0 + y0 * texture->w]);
  }
 }
}

static void
RDR_DrawTexture(const PLT_GameInput *input,
                const RES_Texture *texture,
                int x,
                int y)
{
 RDR_DrawSubTexture(input, texture, RectLit(0, 0, texture->w, texture->h), x, y);
}

static void
RDR_DrawFallingSand(const PLT_GameInput *input,
                    const FLS_State *state)
{
 for (int y = 0;
      y < PLT_gameFixedH;
      y += 1)
 {
  for (int x = 0;
       x < PLT_gameFixedW;
       x += 1)
  {
   Colour result = FLS_GetColour(state, x, y);
   RDR_SetPixel(input, x, y, &result);
  }
 }
}