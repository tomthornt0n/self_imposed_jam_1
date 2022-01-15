#include <stddef.h>

typedef struct
{
 size_t w;
 size_t h;
 const Pixel *buffer;
} RES_Texture;

typedef struct
{
 size_t size;
 unsigned char *wav;
} RES_Audio;

void RES_BackgroundTextureGet(RES_Texture *result);
void RES_Level1TextureGet(RES_Texture *result);
void RES_SpritesheetTextureGet(RES_Texture *result);
