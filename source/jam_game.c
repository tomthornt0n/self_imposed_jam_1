#include <stdlib.h>
#include <assert.h>


typedef struct
{
 int x0;
 int y0;
 int x1;
 int y1;
} Rect;

#define RectLit(_x, _y, _w, _h) ((Rect[]){ (Rect){ _x, _y, _x + _w, _y + _h } })

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
 
 RES_Texture level_1_texture;
 RES_Level1TextureGet(&level_1_texture);
 FLS_StateFromTexture(&GME_fallingSandState, &level_1_texture);
 
 ETT_PlayerMake(128, 128);
}

void
GME_UpdateAndRender(const PLT_GameInput *input)
{
 FLS_Update(input, &GME_fallingSandState);
 
 RDR_ClearScreen(input);
 RDR_DrawTexture(input, &GME_backgroundTexture, 0, 0);
 ETT_UpdateAndRender(input);
 RDR_DrawFallingSand(input, &GME_fallingSandState);
}
