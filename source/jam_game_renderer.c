//
// NOTE(tbt): writes pixels into a buffer
//

static inline void
RDR_SetPixel(const PLT_GameInput *input,
             size_t x, size_t y,
             const Colour *colour,
             int is_bg)
{
 if (colour->a > 0 && PLT_GameHasPixel(x, y))
 {
  Pixel *pixel= &input->pixels[PLT_GamePixelIndex(x, y)];
  
  if (colour->a == 255)
  {
   memcpy(pixel, colour, sizeof(*pixel));
  }
  else
  {
   
   pixel->b = MTH_InterpolateLinearI(pixel->b, colour->b, colour->a);
   pixel->g = MTH_InterpolateLinearI(pixel->g, colour->g, colour->a);
   pixel->r = MTH_InterpolateLinearI(pixel->r, colour->r, colour->a);
  }
  
  pixel->a = !!is_bg;
 }
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
   RDR_SetPixel(input, x, y, colour, 0);
  }
 }
}

typedef Rect RDR_SubTexture;

typedef enum
{
 RDR_DrawSubTextureFlags_hFlip = 1 << 0,
 RDR_DrawSubTextureFlags_vFlip = 1 << 1,
 RDR_DrawSubTextureFlags_isBg  = 1 << 2,
} RDR_DrawSubTextureFlags;

static void
RDR_DrawSubTexture(const PLT_GameInput *input,
                   const RES_Texture *texture,
                   const RDR_SubTexture *sub_texture,
                   int x,
                   int y,
                   RDR_DrawSubTextureFlags flags)
{
 int sub_texture_w = sub_texture->x1 - sub_texture->x0;
 int sub_texture_h = sub_texture->y1 - sub_texture->y0;
 
 for (int y0 = 0;
      y0 < sub_texture_h;
      y0 += 1)
 {
  for (int x0 = 0;
       x0 < sub_texture_w;
       x0 += 1)
  {
   int sample_x;
   int sample_y;
   if (flags & RDR_DrawSubTextureFlags_vFlip)
   {
    sample_x = sub_texture->x1 - x0 - 1;
   }
   else
   {
    sample_x = sub_texture->x0 + x0;
   }
   
   if (flags & RDR_DrawSubTextureFlags_hFlip)
   {
    sample_y = sub_texture->y1 - y0 - 1;
   }
   else
   {
    sample_y = sub_texture->y0 + y0;
   }
   
   RDR_SetPixel(input,
                x + x0,
                y + y0,
                &texture->buffer[sample_x + sample_y * texture->w],
                flags & RDR_DrawSubTextureFlags_isBg);
  }
 }
}

static void
RDR_DrawTexture(const PLT_GameInput *input,
                const RES_Texture *texture,
                int x,
                int y)
{
 RDR_DrawSubTexture(input, texture, RectLit(0, 0, texture->w, texture->h), x, y, 0);
}

static unsigned char RDR_bitmapFont[128][8] =
{
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0000 (nul)
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0001
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0002
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0003
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0004
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0005
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0006
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0007
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0008
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0009
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000A
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000B
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000C
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000D
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000E
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000F
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0010
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0011
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0012
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0013
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0014
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0015
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0016
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0017
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0018
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0019
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001A
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001B
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001C
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001D
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001E
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001F
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0020 (space)
 { 0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00},   // U+0021 (!)
 { 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0022 (")
 { 0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00},   // U+0023 (#)
 { 0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00},   // U+0024 ($)
 { 0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00},   // U+0025 (%)
 { 0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00},   // U+0026 (&)
 { 0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0027 (')
 { 0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00},   // U+0028 (()
 { 0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00},   // U+0029 ())
 { 0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00},   // U+002A (*)
 { 0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00},   // U+002B (+)
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x06},   // U+002C (,)
 { 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00},   // U+002D (-)
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00},   // U+002E (.)
 { 0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00},   // U+002F (/)
 { 0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00},   // U+0030 (0)
 { 0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00},   // U+0031 (1)
 { 0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00},   // U+0032 (2)
 { 0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00},   // U+0033 (3)
 { 0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00},   // U+0034 (4)
 { 0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00},   // U+0035 (5)
 { 0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00},   // U+0036 (6)
 { 0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00},   // U+0037 (7)
 { 0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00},   // U+0038 (8)
 { 0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00},   // U+0039 (9)
 { 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00},   // U+003A (:)
 { 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x06},   // U+003B (;)
 { 0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00},   // U+003C (<)
 { 0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00},   // U+003D (=)
 { 0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00},   // U+003E (>)
 { 0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00},   // U+003F (?)
 { 0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00},   // U+0040 (@)
 { 0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00},   // U+0041 (A)
 { 0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00},   // U+0042 (B)
 { 0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00},   // U+0043 (C)
 { 0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00},   // U+0044 (D)
 { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00},   // U+0045 (E)
 { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00},   // U+0046 (F)
 { 0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00},   // U+0047 (G)
 { 0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00},   // U+0048 (H)
 { 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0049 (I)
 { 0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00},   // U+004A (J)
 { 0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00},   // U+004B (K)
 { 0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00},   // U+004C (L)
 { 0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00},   // U+004D (M)
 { 0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00},   // U+004E (N)
 { 0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00},   // U+004F (O)
 { 0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00},   // U+0050 (P)
 { 0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00},   // U+0051 (Q)
 { 0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00},   // U+0052 (R)
 { 0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00},   // U+0053 (S)
 { 0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0054 (T)
 { 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00},   // U+0055 (U)
 { 0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},   // U+0056 (V)
 { 0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00},   // U+0057 (W)
 { 0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00},   // U+0058 (X)
 { 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00},   // U+0059 (Y)
 { 0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00},   // U+005A (Z)
 { 0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1E, 0x00},   // U+005B ([)
 { 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00},   // U+005C (\)
 { 0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00},   // U+005D (])
 { 0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00},   // U+005E (^)
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF},   // U+005F (_)
 { 0x0C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0060 (`)
 { 0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00},   // U+0061 (a)
 { 0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00},   // U+0062 (b)
 { 0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00},   // U+0063 (c)
 { 0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6E, 0x00},   // U+0064 (d)
 { 0x00, 0x00, 0x1E, 0x33, 0x3f, 0x03, 0x1E, 0x00},   // U+0065 (e)
 { 0x1C, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0F, 0x00},   // U+0066 (f)
 { 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F},   // U+0067 (g)
 { 0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00},   // U+0068 (h)
 { 0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0069 (i)
 { 0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E},   // U+006A (j)
 { 0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00},   // U+006B (k)
 { 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+006C (l)
 { 0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00},   // U+006D (m)
 { 0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00},   // U+006E (n)
 { 0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00},   // U+006F (o)
 { 0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x0F},   // U+0070 (p)
 { 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78},   // U+0071 (q)
 { 0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x0F, 0x00},   // U+0072 (r)
 { 0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00},   // U+0073 (s)
 { 0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00},   // U+0074 (t)
 { 0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00},   // U+0075 (u)
 { 0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},   // U+0076 (v)
 { 0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00},   // U+0077 (w)
 { 0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00},   // U+0078 (x)
 { 0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F},   // U+0079 (y)
 { 0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00},   // U+007A (z)
 { 0x38, 0x0C, 0x0C, 0x07, 0x0C, 0x0C, 0x38, 0x00},   // U+007B ({)
 { 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00},   // U+007C (|)
 { 0x07, 0x0C, 0x0C, 0x38, 0x0C, 0x0C, 0x07, 0x00},   // U+007D (})
 { 0x6E, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+007E (~)
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}    // U+007F
};

static void
RDR_DrawString(const PLT_GameInput *input,
               const char *string,
               int x,
               int y)
{
 int row_start = x;
 
 for (char c = *string;
      c != '\0';
      string += 1, c = *string)
 {
  if (c == '\n')
  {
   y += 8;
   x = row_start;
  }
  else if (c < 128)
  {
   for (int y0 = 0;
        y0 < 8;
        y0 += 1)
   {
    unsigned char row = RDR_bitmapFont[c][y0];
    for (int x0 = 0;
         x0 < 8;
         x0 += 1)
    {
     if (row & (1 << x0))
     {
      input->pixels[PLT_GamePixelIndex(x + x0, y + y0)] = (Pixel){ 255, 255, 255, 255 };
     }
    }
   }
   x += 8;
  }
 }
}

static void
RDR_DrawShadows(const PLT_GameInput *input,
                int amount)
{
 for (int i = 0;
      i < PLT_gameFixedW * PLT_gameFixedH;
      i += 1)
 {
  if (input->pixels[i].a) { continue; } // NOTE(tbt): don't draw shadows against the background
  
  int sample_cap = amount * 2;
  int sample_count = 0;
  
  
  int offset;
  int sector = (i % PLT_gameFixedW) / (PLT_gameFixedW / 3);
  if (sector == 0)
  {
   offset = -PLT_gameFixedW + 1;
  }
  else if (sector == 1)
  {
   offset = -PLT_gameFixedW;
  }
  else if (sector == 2)
  {
   offset = -PLT_gameFixedW - 1;
  }
  
  int light = amount;
  for (int j = i;
       j > 0 && light > 0;
       j += offset)
  {
   if (sample_count > sample_cap) { break; }
   light -= !input->pixels[j].a;
   sample_count += 1;
  }
  
  input->pixels[i].b = MTH_ClampI(input->pixels[i].b + light, 0, 255);
  input->pixels[i].g = MTH_ClampI(input->pixels[i].g + light, 0, 255);
  input->pixels[i].r = MTH_ClampI(input->pixels[i].r + light, 0, 255);
 }
}

static void
RDR_DrawGodRays_EXPERIMENT(const PLT_GameInput *input,
                           int amount)
{
 int buffer[PLT_gameFixedW * PLT_gameFixedH] = {0};
 
 for (int i = 0;
      i < PLT_gameFixedW * PLT_gameFixedH;
      i += 1)
 {
  buffer[i] = input->pixels[i].a;
 }
 
 for (int y = 1;
      y < PLT_gameFixedH;
      y += 1)
 {
  for (int x = 1;
       x < PLT_gameFixedW / 2;
       x += 1)
  {
   buffer[PLT_GamePixelIndex(x, y)] += buffer[PLT_GamePixelIndex(x + 1, y - 1)];
  }
  
  for (int x = PLT_gameFixedW / 2;
       x < PLT_gameFixedW;
       x += 1)
  {
   buffer[PLT_GamePixelIndex(x, y)] += buffer[PLT_GamePixelIndex(x - 1, y - 1)];
  }
 }
 
 for (int y = 0;
      y < PLT_gameFixedH;
      y += 1)
 {
  for (int x = 0;
       x < PLT_gameFixedW;
       x += 1)
  {
   int i = PLT_GamePixelIndex(x, y);
   
   buffer[i] -= y;
   buffer[i] = MTH_MinI(buffer[i], amount);
   input->pixels[i].b = MTH_ClampI(input->pixels[i].b + buffer[i], 0, 255);
   input->pixels[i].g = MTH_ClampI(input->pixels[i].g + buffer[i], 0, 255);
   input->pixels[i].r = MTH_ClampI(input->pixels[i].r + buffer[i], 0, 255);
  }
 }
}