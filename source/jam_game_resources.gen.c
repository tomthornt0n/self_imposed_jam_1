typedef struct
{
 size_t w;
 size_t h;
 const Pixel *buffer;
} RES_Texture;

void RES_BackgroundTextureGet(RES_Texture *result);
void RES_Level1TextureGet(RES_Texture *result);
void RES_SpritesheetTextureGet(RES_Texture *result);
