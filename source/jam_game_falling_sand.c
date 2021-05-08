typedef enum
{
 FLS_CellKind_empty,
 FLS_CellKind_sand,
 FLS_CellKind_water,
 FLS_CellKind_stone,
 FLS_CellKind_gas,
 FLS_CellKind_dirt,
 
 FLS_CellKind_MAX,
} FLS_CellKind;

typedef unsigned int FLS_CellFlags;
typedef enum
{
 FLS_CellFlags_fall   = 1 << 0,
 FLS_CellFlags_spread = 1 << 1,
 FLS_CellFlags_rise   = 1 << 2,
 FLS_CellFlags_solid  = 1 << 3,
} FLS_CellFlags_ENUM;

typedef struct
{
 double accumulator;
 FLS_CellKind cells[OS_gameFixedW * OS_gameFixedH];
} FLS_State;

#define FLS_CellAt(_state, _x, _y) ((_state)->cells[OS_GamePixelIndex(_x, _y)])

#define FLS_IsCellInBounds(_x, _y) ((_x) >= 0 && (_x) < OS_gameFixedW && (_y) >= 0 && (_y) < OS_gameFixedH)

#define FLS_CellHasFlag(_cell, _flags) (FLS_cellTable[_cell].flags & (_flags))

#define FLS_CellAtHasFlag(_state, _x, _y, _flags) (FLS_CellHasFlag(FLS_CellAt(_state, _x, _y), _flags))

#define FLS_GetColour(_state, _x, _y) (FLS_IsCellInBounds(_x, _y) ? (FLS_cellTable[FLS_CellAt(_state, _x, _y)].get_colour(_state, _x, _y)) : (Colour){0})

typedef Colour ( *FLS_GetColourCallback) (const FLS_State *state, int x, int y);
#include "jam_game_falling_sand_colour_funcs.c"

struct FLS_Cell
{
 FLS_GetColourCallback get_colour;
 FLS_CellFlags flags;
 int density;
} FLS_cellTable[FLS_CellKind_MAX] =
{
 [FLS_CellKind_empty] =
 {
  .get_colour = FLS_EmptyGetColour,
  .flags = 0,
  .density = 0,
 },
 
 [FLS_CellKind_sand] =
 {
  .get_colour = FLS_SandGetColour,
  .flags = FLS_CellFlags_fall,
  .density = 2,
 },
 
 [FLS_CellKind_water] =
 {
  .get_colour = FLS_WaterGetColour,
  .flags = (FLS_CellFlags_fall |
            FLS_CellFlags_spread),
  .density = 1,
 },
 
 [FLS_CellKind_stone] = 
 {
  .get_colour = FLS_StoneGetColour,
  .flags = FLS_CellFlags_solid,
  .density = 999,
 },
 
 [FLS_CellKind_gas] =
 {
  .get_colour = FLS_GasGetColour,
  .flags = (FLS_CellFlags_spread |
            FLS_CellFlags_rise),
  .density = -1,
 },
 
 [FLS_CellKind_dirt] = 
 {
  .get_colour = FLS_DirtGetColour,
  .flags = FLS_CellFlags_solid,
  .density = 999,
 },
};

static void
FLS_CellSwap(FLS_State *state,
             int x0, int y0,
             int x1, int y1)
{
 FLS_CellKind temp = FLS_CellAt(state, x0, y0);
 FLS_CellAt(state, x0, y0) = FLS_CellAt(state, x1, y1);
 FLS_CellAt(state, x1, y1) = temp;
}

static int
FLS_CellCanFallTo(FLS_State *state,
                  int x0, int y0,
                  int x1, int y1)
{
 if (FLS_CellAtHasFlag(state, x0, y0, FLS_CellFlags_rise))
 {
  return (FLS_IsCellInBounds(x0, y0) &&
          FLS_IsCellInBounds(x1, y1) &&
          ((!FLS_CellAtHasFlag(state, x1, y1, FLS_CellFlags_solid)) &&
           FLS_cellTable[FLS_CellAt(state, x1, y1)].density >
           FLS_cellTable[FLS_CellAt(state, x0, y0)].density));
 }
 else
 {
  return (FLS_IsCellInBounds(x0, y0) &&
          FLS_IsCellInBounds(x1, y1) &&
          (FLS_cellTable[FLS_CellAt(state, x1, y1)].density <
           FLS_cellTable[FLS_CellAt(state, x0, y0)].density));
 }
}

static void
FLS_SetCells(FLS_State *state,
             int x, int y,
             int radius,
             FLS_CellKind kind)
{
 if (radius)
 {
  for (int y0 = -radius;
       y0 <= radius;
       y0 += 1)
  {
   for (int x0 = -radius;
        x0 <= radius;
        x0 += 1)
   {
    if (x0 * x0 + y0 * y0 <= radius * radius &&
        FLS_IsCellInBounds(x + x0, y + y0))
    {
     FLS_CellAt(state, x + x0, y + y0) = kind;
    }
   }
  }
 }
 else if (FLS_IsCellInBounds(x, y))
 {
  FLS_CellAt(state, x, y) = kind;
 }
}

static void
FLS_StateFromTexture(FLS_State *state,
                     const RES_Texture *texture)
{
 int max_x = MAT_MinI(texture->w, OS_gameFixedW);
 int max_y = MAT_MinI(texture->h, OS_gameFixedH);
 
 for (int x = 0;
      x < max_x;
      x += 1)
 {
  for (int y = 0;
       y < max_y;
       y += 1)
  {
   unsigned long *pixel = (unsigned long *)(&texture->buffer[x + y * texture->w]);
   
   if (*pixel == 0xFF000000)
   {
    FLS_CellAt(state, x, y) = FLS_CellKind_stone;
   }
   else if (*pixel == 0xFFFF0000)
   {
    FLS_CellAt(state, x, y) = FLS_CellKind_sand;
   }
   else if (*pixel == 0xFF0000FF)
   {
    FLS_CellAt(state, x, y) = FLS_CellKind_water;
   }
   else if (*pixel == 0xFF00FF00)
   {
    FLS_CellAt(state, x, y) = FLS_CellKind_gas;
   }
   else
   {
    FLS_CellAt(state, x, y) = FLS_CellKind_empty;
   }
  }
 }
}

static void
FLS_Update(const OS_GameInput *input,
           FLS_State *state)
{
 double timestep = 0.01f;
 
 state->accumulator += input->dt;
 
 while (state->accumulator > timestep)
 {
  state->accumulator -= timestep;
  
  int has_already_been_updated[OS_gameFixedW * OS_gameFixedH] = {0};
  
  for (int y0 = 0;
       y0 < OS_gameFixedH;
       y0 += 1)
  {
   for (int x0 = 0;
        x0 < OS_gameFixedW;
        x0 += 1)
   {
    int x1 = x0, y1 = y0;
    
    if (has_already_been_updated[OS_GamePixelIndex(x0, y0)]) { continue; }
    
    if (FLS_CellAtHasFlag(state, x0, y0, FLS_CellFlags_rise))
    {
     if (FLS_CellCanFallTo(state, x0, y0, x0, y0 - 1))
     {
      y1 = y0 - 1;
      goto move_cell;
     }
     else if (FLS_CellCanFallTo(state, x0, y0, x0 - 1, y0 - 1))
     {
      if (FLS_CellCanFallTo(state, x0, y0, x0 + 1, y0 - 1) &&
          RNG_RandIntNext(0, 2))
      {
       x1 = x0 + 1;
       y1 = y0 - 1;
      }
      else
      {
       x1 = x0 - 1;
       y1 = y0 - 1;
      }
      goto move_cell;
     }
     else if (FLS_CellCanFallTo(state, x0, y0, x0 + 1, y0 - 1))
     {
      x1 = x0 + 1;
      y1 = y0 - 1;
      goto move_cell;
     }
    }
    
    if (FLS_CellAtHasFlag(state, x0, y0, FLS_CellFlags_fall))
    {
     if (FLS_CellCanFallTo(state, x0, y0, x0, y0 + 1))
     {
      y1 = y0 + 1;
      goto move_cell;
     }
     else if (FLS_CellCanFallTo(state, x0, y0, x0 - 1, y0 + 1))
     {
      if (FLS_CellCanFallTo(state, x0, y0, x0 + 1, y0 + 1) &&
          RNG_RandIntNext(0, 2))
      {
       x1 = x0 + 1;
       y1 = y0 + 1;
      }
      else
      {
       x1 = x0 - 1;
       y1 = y0 + 1;
      }
      goto move_cell;
     }
     else if (FLS_CellCanFallTo(state, x0, y0, x0 + 1, y0 + 1))
     {
      x1 = x0 + 1;
      y1 = y0 + 1;
      goto move_cell;
     }
    }
    
    if (FLS_CellAtHasFlag(state, x0, y0, FLS_CellFlags_spread))
    {
     if (FLS_CellCanFallTo(state, x0, y0, x0 + 1, y0))
     {
      if (FLS_CellCanFallTo(state, x0, y0, x0 - 1, y0) &&
          RNG_RandIntNext(0, 2))
      {
       x1 = x0 - 1;
      }
      else
      {
       x1 = x0 + 1;
      }
      goto move_cell;
     }
     else if (FLS_CellCanFallTo(state, x0, y0, x0 - 1, y0))
     {
      x1 = x0 - 1;
      goto move_cell;
     }
    }
    
    has_already_been_updated[OS_GamePixelIndex(x0, y0)] = 1;
    
    move_cell:
    has_already_been_updated[OS_GamePixelIndex(x0, y0)] = 1;
    has_already_been_updated[OS_GamePixelIndex(x1, y1)] = 1;
    FLS_CellSwap(state, x0, y0, x1, y1);
   }
  }
 }
}