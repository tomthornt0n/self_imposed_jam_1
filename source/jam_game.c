#include <math.h>
#include <immintrin.h>

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

static double GME_timestep = 0.0167;

static int GME_monsterCount = 0;
static int GME_wave = 0;

typedef enum
{
    GME_State_playing,
    GME_State_gameOver,
} GME_State;

GME_State GME_state = GME_State_playing;

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
    ETT_PlayerMake(300, 47);
}

void
GME_UpdateAndRender(const PLT_GameInput *input)
{
    static double accumulator = 0.0;
    
    accumulator += input->dt;
    accumulator = MTH_MinF(accumulator, 0.12);
    
    while (accumulator > GME_timestep)
    {
        accumulator -= GME_timestep;
        
        if (GME_State_playing == GME_state)
        {
            FLS_Update(input, &GME_fallingSandState);
            ETT_Update(input, &GME_fallingSandState);
            
            if (GME_monsterCount <= 0)
            {
                GME_wave += 1;
                
                for (int i = 0;
                     i < GME_wave;
                     i += 1)
                {
                    int x = RNG_RandIntNext(0, PLT_gameFixedW - 1);
                    int y = 47;
                    
                    int monster_kind = RNG_RandIntNext(0, 3);
                    if (monster_kind == 0)
                    {
                        ETT_SandGolemMake(x, y);
                    }
                    else if (monster_kind == 2)
                    {
                        ETT_DirtGolemMake(x, y);
                    }
                    else if (monster_kind == 1)
                    {
                        ETT_SlimeMake(x, y);
                    }
                }
            }
        }
        else if (GME_State_gameOver == GME_state)
        {
        }
    }
    
    if (GME_State_playing == GME_state)
    {
        RDR_DrawSubTexture(input, &GME_backgroundTexture, RectLit(0, 0, GME_backgroundTexture.w, GME_backgroundTexture.h), 0, 0, RDR_DrawSubTextureFlags_isBg);
        ETT_Render(input, accumulator);
        RDR_DrawTexture(input, &GME_fallingSandState.texture, 0, 0);
        RDR_DrawShadows(input, 12);
    }
    else if (GME_State_gameOver == GME_state)
    {
        RDR_ClearScreen(input);
        char msg_str[64];
        stbsp_snprintf(msg_str, sizeof(msg_str), "    :(\n\n Game Over\n\nYou scored: %d", GME_wave * GME_wave * GME_wave - GME_monsterCount);
        RDR_DrawString(input, msg_str, 200, 100);
    }
    
    char fps_str[64];
    stbsp_snprintf(fps_str, sizeof(fps_str), "%fms (%f fps)", input->dt * 1000, 1.0 / input->dt);
    RDR_DrawString(input, fps_str, 4, 4);
}
