typedef struct
{
 int x0;
 int y0;
 int x1;
 int y1;
} Rect;

#define RectLit(_x, _y, _w, _h) ((Rect[]){ (Rect){ _x, _y, _x + _w, _y + _h } })

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

static const double GME_timestep = 0.0167;

#include "jam_game_platform.c"
#include "jam_game_resources.gen.c"
#include "jam_game_math.c"
#include "jam_game_rng.c"
#include "jam_game_falling_sand.c"
#include "jam_game_renderer.c"
#include "jam_game_entities.c"

FLS_State GME_fallingSandState;
RES_Texture GME_backgroundTexture;

void
GME_Initialise(void)
{
 RES_BackgroundTextureGet(&GME_backgroundTexture);
 
 FSC_PerlinPrecompute();
 
 RES_Texture level_1_texture;
 RES_Level1TextureGet(&level_1_texture);
 FLS_StateFromTexture(&GME_fallingSandState, &level_1_texture);
 ETT_PlayerMake(344, 175 - 128);
}

void
GME_UpdateAndRender(const PLT_GameInput *input)
{
 static double accumulator = 0.0;
 
 accumulator += input->dt;
 accumulator = MTH_MinF(accumulator, 0.2);
 while (accumulator > GME_timestep)
 {
  accumulator -= GME_timestep;
  
  FLS_Update(input, &GME_fallingSandState);
  ETT_Update(input, &GME_fallingSandState);
 }
 
 RDR_DrawTexture(input, &GME_backgroundTexture, 0, 0);
 ETT_Render(input, accumulator);
 RDR_DrawTexture(input, &GME_fallingSandState.texture, 0, 0);
 
 char fps_str[32];
 stbsp_snprintf(fps_str, sizeof(fps_str), "%fms (%f fps)", input->dt * 1000, 1.0 / input->dt);
 RDR_DrawString(input, fps_str, 4, 4);
}
