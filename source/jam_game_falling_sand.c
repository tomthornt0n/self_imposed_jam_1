typedef enum
{
 FS_CellKind_empty,
 FS_CellKind_sand,
 FS_CellKind_water,
 
 FS_CellKind_MAX,
} FS_CellKind;

typedef unsigned int FS_CellFlags;
typedef enum
{
 FS_CellFlags_fallDown        = 1 << 0,
 FS_CellFlags_fallHorizontal  = 1 << 1,
} FS_CellFlags_ENUM;

static Colour
FS_EmptyGetColour(int x,
                  int y)
{
 return (Colour){ 0, 0, 0, 0 };
}

static Colour
FS_SandGetColour(int x,
                 int y)
{
 Colour sand_colour =
 {
  .b = 66,
  .g = 135,
  .r = 245,
  .a = 255,
 };
 
 float b = 66.0f  / 255.0f;
 float g = 135.0f / 255.0f;
 float r = 245.0f / 255.0f;
 
 b = MA_MinF(1.0f, b + (RNG_Perlin_2D(x, y, 0.9f, 4) * 0.00000002f));
 g = MA_MinF(1.0f, g + (RNG_Perlin_2D(x, y, 0.9f, 4) * 0.00000002f));
 r = MA_MinF(1.0f, r + (RNG_Perlin_2D(x, y, 0.9f, 4) * 0.00000002f));
 
 return (Colour){ 255 * b, 255 * g, 255 * r, 255 };
}

static Colour
FS_WaterGetColour(int x,
                  int y)
{
 return (Colour){ 245, 135, 66, 255 };
}

typedef Colour ( *FS_GetColourCallback) (int x, int y);

struct FS_Cell
{
 FS_GetColourCallback get_colour;
 FS_CellFlags flags;
 int density;
} FS_cellTable[FS_CellKind_MAX] =
{
 [FS_CellKind_empty] =
 {
  .get_colour = FS_EmptyGetColour,
  .flags = 0,
  .density = 0,
 },
 
 [FS_CellKind_sand] =
 {
  .get_colour = FS_SandGetColour,
  .flags = FS_CellFlags_fallDown,
  .density = 2,
 },
 
 [FS_CellKind_water] =
 {
  .get_colour = FS_WaterGetColour,
  .flags = (FS_CellFlags_fallDown |
            FS_CellFlags_fallHorizontal),
  .density = 1,
 },
};

typedef struct
{
 FS_CellKind cells[OS_gameFixedW * OS_gameFixedH];
} FS_State;

#define FS_CellAt(_state, _x, _y) ((_state)->cells[OS_GamePixelIndex(_x, _y)])

#define FS_IsCellInBounds(_x, _y) ((_x) >= 0 && (_x) < OS_gameFixedW && (_y) >= 0 && (_y) < OS_gameFixedH)

#define FS_GetColour(_state, _x, _y) (FS_IsCellInBounds(_x, _y) ? (FS_cellTable[FS_CellAt(_state, _x, _y)].get_colour(_x, _y)) : (Colour){0})

#define FS_CellHasFlag(_cell, _flags) (FS_cellTable[_cell].flags & (_flags))

#define FS_CellAtHasFlag(_state, _x, _y, _flags) (FS_CellHasFlag(FS_CellAt(_state, _x, _y), _flags))

static void
FS_CellSwap(FS_State *state,
            int x0, int y0,
            int x1, int y1)
{
 FS_CellKind temp = FS_CellAt(state, x0, y0);
 FS_CellAt(state, x0, y0) = FS_CellAt(state, x1, y1);
 FS_CellAt(state, x1, y1) = temp;
}

static int
FS_CellCanFallTo(FS_State *state,
                 int x0, int y0,
                 int x1, int y1)
{
 return ((FS_cellTable[FS_CellAt(state, x1, y1)].density <
          FS_cellTable[FS_CellAt(state, x0, y0)].density) &&
         FS_IsCellInBounds(x1, y1));
}

static void
FS_PlaceCell(FS_State *state,
             int x, int y,
             FS_CellKind kind)
{
 FS_CellAt(state, x, y) = kind;
}

static void
FS_Update(const OS_GameInput *input,
          FS_State *state)
{
 int is_not_dirty[OS_gameFixedW * OS_gameFixedH] = {0};
 
 for (int y = 0;
      y < OS_gameFixedH;
      y += 1)
 {
  for (int x = 0;
       x < OS_gameFixedW;
       x += 1)
  {
   if (!is_not_dirty[x + y * OS_gameFixedW])
   {
    is_not_dirty[x + y * OS_gameFixedW] = 1;
    
    if (FS_CellAtHasFlag(state, x, y, FS_CellFlags_fallDown))
    {
     if (FS_CellCanFallTo(state, x, y, x, y + 1))
     {
      FS_CellSwap(state, x, y, x, y + 1);
      is_not_dirty[OS_GamePixelIndex(x, y + 1)] = 1;
      continue;
     }
     else if (FS_CellCanFallTo(state, x, y, x - 1, y + 1))
     {
      FS_CellSwap(state, x, y, x - 1, y + 1);
      is_not_dirty[OS_GamePixelIndex(x - 1, y + 1)] = 1;
      continue;
     }
     else if (FS_CellCanFallTo(state, x, y, x + 1, y + 1))
     {
      FS_CellSwap(state, x, y, x + 1, y + 1);
      is_not_dirty[OS_GamePixelIndex(x + 1, y + 1)] = 1;
      continue;
     }
    }
    
    if (FS_CellAtHasFlag(state, x, y, FS_CellFlags_fallHorizontal))
    {
     if (FS_CellCanFallTo(state, x, y, x + 1, y))
     {
      FS_CellSwap(state, x, y, x + 1, y);
      is_not_dirty[OS_GamePixelIndex(x + 1, y)] = 1;
      continue;
     }
     else if (FS_CellCanFallTo(state, x, y, x - 1, y))
     {
      FS_CellSwap(state, x, y, x - 1, y);
      is_not_dirty[OS_GamePixelIndex(x - 1, y)] = 1;
      continue;
     }
    }
   }
  }
 }
}