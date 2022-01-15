#include <math.h>
#include <immintrin.h>
#include <stdio.h>
#include <string.h>

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

typedef enum
{
    GME_State_menu,
    GME_State_playing,
    GME_State_gameOver,
} GME_State;

typedef struct
{
    double time_dilation;
    int monster_count;
    int wave;
    GME_State state;
} GME_Data;
static GME_Data GME_state = {0};

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
GME_Initialise(const PLT_GameInput *input)
{
    RES_BackgroundTextureGet(&GME_backgroundTexture);
    
    FSC_PerlinPrecompute();
    
    RES_Texture level_1_texture;
    RES_Level1TextureGet(&level_1_texture);
    FLS_StateFromTexture(&GME_fallingSandState, &level_1_texture);
    ETT_PlayerMake(300, 47);
    
    GME_state.time_dilation = 1.0;
}

static int
GME_CalculateScore(void)
{
    return (GME_state.wave * GME_state.wave * GME_state.wave - GME_state.monster_count);
}

void
GME_UpdateAndRender(const PLT_GameInput *input)
{
    static double accumulator = 0.0;
    
    accumulator += input->dt;
    
#if 0
    accumulator = MTH_MinF(accumulator, 0.12);
    while
#else
    if
#endif
    (accumulator > GME_timestep * GME_state.time_dilation)
    {
        accumulator -= GME_timestep * GME_state.time_dilation;
        
        if (GME_State_playing == GME_state.state || GME_State_menu == GME_state.state)
        {
            FLS_Update(input, &GME_fallingSandState);
            ETT_Update(input, &GME_fallingSandState);
            
            if(GME_State_playing == GME_state.state)
            {
                if (GME_state.monster_count <= 0)
                {
                    GME_state.wave += 1;
                    
                    for (int i = 0;
                         i < GME_state.wave;
                         i += 1)
                    {
                        int x = RNG_RandIntNext(0, PLT_gameFixedW - 48);
                        int y = 47;
                        
                        enum
                        {
                            MonsterKind_sandGolem,
                            MonsterKind_dirtGolem,
                            MonsterKind_stoneGolem,
                            MonsterKind_cloud,
                            MonsterKind_slime,
                            MonsterKind_MAX,
                        } monster_kind = RNG_RandIntNext(0, MonsterKind_MAX);
                        
                        if (monster_kind == MonsterKind_sandGolem)
                        {
                            ETT_SandGolemMake(x, y);
                        }
                        else if (monster_kind == MonsterKind_dirtGolem)
                        {
                            ETT_DirtGolemMake(x, y);
                        }
                        else if (monster_kind == MonsterKind_slime)
                        {
                            ETT_SlimeMake(x, y);
                        }
                        else if (monster_kind == MonsterKind_stoneGolem)
                        {
                            ETT_StoneGolemMake(x, y);
                        }
                        else if (monster_kind == MonsterKind_cloud)
                        {
                            ETT_CloudMake(x, y + RNG_RandIntNext(0, 16));
                        }
                    }
                }
            }
            else
            {
                if(input->is_key_down[PLT_Key_enter])
                {
                    GME_state.state = GME_State_playing;
                }
            }
        }
        else if (GME_State_gameOver == GME_state.state)
        {
        }
    }
    
    if (GME_State_playing == GME_state.state || GME_State_menu == GME_state.state)
    {
        RDR_DrawSubTexture(input, &GME_backgroundTexture, RectLit(0, 0, GME_backgroundTexture.w, GME_backgroundTexture.h), 0, 0, RDR_DrawSubTextureFlags_isBg);
        ETT_Render(input, accumulator);
        RDR_DrawTexture(input, &GME_fallingSandState.texture, 0, 0);
        RDR_DrawShadows(input, 12);
        
        // NOTE(tbt): draw score text
        char score_str[64];
        if(GME_State_playing == GME_state.state)
        {
            stbsp_snprintf(score_str, sizeof(score_str), "score: %d", GME_CalculateScore());
        }
        else
        {
            stbsp_snprintf(score_str, sizeof(score_str), "press enter to begin", GME_CalculateScore());
        }
        RDR_DrawString(input, score_str, 4, 4);
    }
    else if (GME_State_gameOver == GME_state.state)
    {
        RDR_ClearScreen(input);
        char msg_str[80];
        stbsp_snprintf(msg_str, sizeof(msg_str), "Game Over\nYou scored: %d\nPress enter to retry...", GME_CalculateScore());
        RDR_DrawString(input, msg_str, 100, 100);
        
        if (input->is_key_down[PLT_Key_enter])
        {
            ETT_FreeAll();
            memset(&GME_state, 0, sizeof(GME_state));
            GME_state.time_dilation = 1.0;
            
            RES_Texture level_1_texture;
            RES_Level1TextureGet(&level_1_texture);
            FLS_StateFromTexture(&GME_fallingSandState, &level_1_texture);
            
            ETT_PlayerMake(300, 47);
        }
    }
    
#if 0
    char fps_str[64];
    stbsp_snprintf(fps_str, sizeof(fps_str), "%fms (%f fps)", input->dt * 1000, 1.0 / input->dt);
    RDR_DrawString(input, fps_str, 4, 4);
#endif
}
