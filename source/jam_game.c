#include <stdlib.h>
#include <assert.h>


typedef struct
{
 size_t x0;
 size_t y0;
 size_t x1;
 size_t y1;
} Rect;

#define RectLit(_x, _y, _w, _h) ((Rect[]){ (Rect){ _x, _y, _x + _w, _y + _h } })

#include "jam_game_os.c"
#include "jam_game_math.c"
#include "jam_game_rng.c"
#include "jam_game_falling_sand.c"
#include "jam_game_renderer.c"

void
Game_Initialise(void)
{
 return;
}

void
Game_UpdateAndRender(const OS_GameInput *input)
{
 static FS_State falling_sand_state = {0};
 
 static int brush_radius = 10;
 
 R_ClearScreen(input);
 
 if (input->is_key_down[OS_Key_mouseLeft])
 {
  FS_SetCells(&falling_sand_state, input->mouse_x, input->mouse_y, brush_radius, FS_CellKind_sand);
 }
 else if (input->is_key_down[OS_Key_mouseRight])
 {
  FS_SetCells(&falling_sand_state, input->mouse_x, input->mouse_y, brush_radius, FS_CellKind_water);
 }
 else if (input->is_key_down[OS_Key_mouseMiddle])
 {
  
  FS_SetCells(&falling_sand_state, input->mouse_x, input->mouse_y, brush_radius, FS_CellKind_stone);
 }
 
 FS_Update(input, &falling_sand_state);
 
 R_DrawFallingSand(input, &falling_sand_state);
}
