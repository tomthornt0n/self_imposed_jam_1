enum ETT_Constants
{
 ETT_animationFramesMax = 16,
 ETT_gravityStrength = 2,
 ETT_maxIncline = 2,
 ETT_playerSpeed = 2,
 ETT_playerJumpPower = 6,
};

typedef size_t ETT_Flags;
typedef enum
{
 ETT_Flags_playerMovement    = 1 << 0,
 ETT_Flags_drawSubTexture    = 1 << 1,
 ETT_Flags_movement          = 1 << 2,
 ETT_Flags_gravity           = 1 << 2,
} ETT_Flags_ENUM;

typedef struct ETT_e ETT_e;
struct ETT_e
{
 ETT_e *parent;
 ETT_e *children_first;
 ETT_e *children_last;
 ETT_e *sibling_next;
 ETT_e *sibling_prev;
 
 ETT_Flags flags;
 
 int x;
 int y;
 int prev_x;
 int prev_y;
 int collision_w;
 int collision_h;
 
 RES_Texture texture;
 RDR_SubTexture sub_texture[ETT_animationFramesMax];
 size_t animation_frames_count;
 
 int vel_x;
 int vel_y;
};

enum { ETT_maxEntities = 128 };
static struct
{
 ETT_e buffer[ETT_maxEntities];
 size_t i;
} ETT_arena;

static ETT_e *ETT_topLevel = NULL;

static ETT_e *
ETT_Push(void)
{
 ETT_e *result = NULL;
 
 if (ETT_arena.i < ETT_maxEntities - 1)
 {
  result = &ETT_arena.buffer[ETT_arena.i];
  memset(result, 0, sizeof(*result));
  ETT_arena.i += 1;
 }
 
 return result;
}

static void
ETT_FreeAll(void)
{
 memset(&ETT_arena, 0, sizeof(ETT_arena));
 ETT_topLevel = NULL;
}

static ETT_e *
ETT_ChildMake(ETT_e *parent)
{
 ETT_e *child = ETT_Push();
 
 if (NULL != parent)
 {
  if (parent->children_last)
  {
   parent->children_last->sibling_next = child;
   child->sibling_prev = parent->children_last;
  }
  else
  {
   parent->children_first = child;
  }
  parent->children_last = child;
  child->parent = parent;
 }
 else
 {
  if (ETT_topLevel)
  {
   ETT_topLevel->sibling_prev = child;
  }
  child->sibling_next = ETT_topLevel;
  ETT_topLevel = child;
 }
 
 return child;
}

static ETT_e *
ETT_PlayerMake(int x, int y)
{
 ETT_e *player = ETT_ChildMake(NULL);
 
 player->flags |= ETT_Flags_playerMovement;
 player->flags |= ETT_Flags_drawSubTexture;
 player->flags |= ETT_Flags_movement;
 RES_PlayerTextureGet(&player->texture);
 player->x = x;
 player->y = y;
 player->collision_w = 16;
 player->collision_h = 33;
 player->sub_texture[0] = (RDR_SubTexture){ 0, 0, 16, 32 };
 player->sub_texture[1] = (RDR_SubTexture){ 0, 0, 32, 48 };
 player->sub_texture[2] = (RDR_SubTexture){ 0, 0, 48, 64 };
 player->sub_texture[3] = (RDR_SubTexture){ 0, 0, 64, 80 };
 player->animation_frames_count = 4;
 
 return player;
}

static void
ETT_TreeDescendUpdate(const PLT_GameInput *input,
                      const FLS_State *falling_sand_state,
                      ETT_e *e,
                      int sum_parents_x,
                      int sum_parents_y)
{
 e->prev_x = e->x;
 e->prev_y = e->y;
 
 e->vel_x = 0;
 e->vel_y = 0;
 
 if (e->flags & ETT_Flags_gravity)
 {
  e->vel_y += ETT_gravityStrength;
 }
 
 if (e->flags & ETT_Flags_playerMovement)
 {
  int max_x = PLT_gameFixedW - (e->sub_texture->x1 - e->sub_texture->x0);
  int max_y = PLT_gameFixedW - (e->sub_texture->y1 - e->sub_texture->y0);
  
  e->vel_x += ETT_playerSpeed * (input->is_key_down[PLT_Key_d] || input->is_key_down[PLT_Key_right]);
  e->vel_x -= ETT_playerSpeed * (input->is_key_down[PLT_Key_a] || input->is_key_down[PLT_Key_left]);
  
  e->vel_y += -ETT_playerJumpPower * (input->is_key_down[PLT_Key_w] ||
                                      input->is_key_down[PLT_Key_up] ||
                                      input->is_key_down[PLT_Key_space]);
  
  e->x = MTH_ClampI(e->x, 0, max_x);
  e->y = MTH_ClampI(e->y, 0, max_y);
 }
 
 if (e->flags & ETT_Flags_movement)
 {
  int vel_x;
  int dir_x;
  if (e->vel_x < 0)
  {
   vel_x = -1 * e->vel_x;
   dir_x = -1;
  }
  else
  {
   vel_x = e->vel_x;
   dir_x = 1;
  }
  
  int vel_y;
  int dir_y;
  if (e->vel_y < 0)
  {
   vel_y = -1 * e->vel_y;
   dir_y = -1;
  }
  else
  {
   vel_y = e->vel_y;
   dir_y = 1;
  }
  
  for (int i = 0;
       i < vel_y;
       i += 1)
  {
   for (int x = 0;
        x < e->collision_w;
        x += 1)
   {
    if (FLS_CellAtHasFlag(falling_sand_state,
                          sum_parents_x + e->x + x,
                          sum_parents_y + e->y + (e->vel_y > 0) * e->collision_h,
                          FLS_CellFlags_solid))
    {
     goto collision_y;
    }
   }
   e->y += dir_y;
  } collision_y:;
  
  int incline = 1;
  incline_loop:
  for (int i = 0;
       i < vel_x;
       i += 1)
  {
   e->x += dir_x;
   
   for (int y = 0;
        y < e->collision_h;
        y += 1)
   {
    if (FLS_CellAtHasFlag(falling_sand_state,
                          sum_parents_x + e->x + (e->vel_x > 0) * e->collision_w,
                          sum_parents_y + e->y + y - incline,
                          FLS_CellFlags_solid))
    {
     e->x -= dir_x;
     incline += 1;
     if (incline <= ETT_maxIncline)
     {
      e->y -= 1;
      goto incline_loop;
     }
     else
     {
      goto collision_x;
     }
    }
   }
  } collision_x:;
 }
 
 for (ETT_e *child = e->children_first;
      NULL != child;
      child = child->sibling_next)
 {
  ETT_TreeDescendUpdate(input,
                        falling_sand_state,
                        child,
                        e->x + sum_parents_x,
                        e->y + sum_parents_y);
 }
}

static void
ETT_TreeDescendRender(const PLT_GameInput *input,
                      double accumulator,
                      ETT_e *e,
                      int x, int y)
{
 int t = 255.0 * (accumulator / GME_timestep);
 int interpolated_x = ((t * (e->prev_x - e->x)) >> 8) + e->x;
 int interpolated_y = ((t * (e->prev_y - e->y)) >> 8) + e->y;
 
 x += interpolated_x;
 y += interpolated_y;
 
 if (e->flags & ETT_Flags_drawSubTexture)
 {
  RDR_DrawSubTexture(input,
                     &e->texture,
                     e->sub_texture,
                     x, y);
 }
 
 for (ETT_e *child = e->children_first;
      NULL != child;
      child = child->sibling_next)
 {
  ETT_TreeDescendRender(input, accumulator, child, x, y);
 }
}

static void
ETT_Update(const PLT_GameInput *input,
           const FLS_State *falling_sand_state)
{
 for (ETT_e *e = ETT_topLevel;
      NULL != e;
      e = e->sibling_next)
 {
  ETT_TreeDescendUpdate(input, falling_sand_state, e, 0, 0);
 }
}

static void
ETT_Render(const PLT_GameInput *input,
           double accumulator)
{
 for (ETT_e *e = ETT_topLevel;
      NULL != e;
      e = e->sibling_next)
 {
  ETT_TreeDescendRender(input, accumulator, e, 0, 0);
 }
}