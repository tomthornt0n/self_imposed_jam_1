enum ETT_Constants
{
 ETT_animationFramesMax = 16,
 ETT_gravityStrength = 2,
 ETT_maxIncline = 2,
 ETT_playerSpeed = 2,
 ETT_playerJumpPower = 3,
};

typedef size_t ETT_Flags;
typedef enum
{
 ETT_Flags_playerMovement    = 1 << 0,
 ETT_Flags_drawSubTexture    = 1 << 1,
 ETT_Flags_mobMovement       = 1 << 2,
 ETT_Flags_gravity           = 1 << 3,
 ETT_Flags_simpleMovement    = 1 << 4,
 ETT_Flags_destructive       = 1 << 5,
 ETT_Flags_destroyOnContact  = 1 << 6,
 ETT_Flags_markedForRemoval  = 1 << 7,
} ETT_Flags_ENUM;

typedef struct ETT_e ETT_e;
struct ETT_e
{
 ETT_e *next;
 ETT_e *next_free;
 
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

enum { ETT_maxEntities = 512 };
static struct
{
 ETT_e buffer[ETT_maxEntities];
 size_t i;
 ETT_e *free_list;
} ETT_arena = {0};

static ETT_e *ETT_list = NULL;

static ETT_e *
ETT_Push(void)
{
 ETT_e *result = NULL;
 
 if (ETT_arena.free_list)
 {
  result = ETT_arena.free_list;
  ETT_arena.free_list = ETT_arena.free_list->next_free;
 }
 else if (ETT_arena.i < ETT_maxEntities - 1)
 {
  result = &ETT_arena.buffer[ETT_arena.i];
  ETT_arena.i += 1;
 }
 else
 {
  return NULL;
 }
 
 memset(result, 0, sizeof(*result));
 result->next = ETT_list;
 ETT_list = result;
 
 return result;
}

static void
ETT_FreeAll(void)
{
 memset(&ETT_arena, 0, sizeof(ETT_arena));
 ETT_list = NULL;
}

static ETT_e *
ETT_PlayerMake(int x, int y)
{
 ETT_e *player = ETT_Push();
 
 if (player)
 {
  player->flags |= ETT_Flags_playerMovement;
  player->flags |= ETT_Flags_drawSubTexture;
  player->flags |= ETT_Flags_mobMovement;
  player->flags |= ETT_Flags_gravity;
  player->x = x;
  player->y = y;
  player->collision_w = 16;
  player->collision_h = 33;
  RES_PlayerTextureGet(&player->texture);
  player->sub_texture[0] = (RDR_SubTexture){  0,  0, 16, 32 };
  player->sub_texture[1] = (RDR_SubTexture){ 16,  0, 32, 32 };
  player->sub_texture[2] = (RDR_SubTexture){ 32,  0, 48, 32 };
  player->sub_texture[3] = (RDR_SubTexture){ 48,  0, 64, 32 };
  player->animation_frames_count = 4;
 }
 
 return player;
}

static ETT_e *
ETT_PlayerProjectileMake(const PLT_GameInput *input,
                         ETT_e *player)
{
 ETT_e *projectile = ETT_Push();
 
 if (projectile)
 {
  projectile->flags |= ETT_Flags_drawSubTexture;
  projectile->flags |= ETT_Flags_simpleMovement;
  projectile->flags |= ETT_Flags_destructive;
  projectile->flags |= ETT_Flags_destroyOnContact;
  projectile->x = player->x + player->collision_w / 2;
  projectile->y = player->y + player->collision_h / 2;
  projectile->prev_x = projectile->x;
  projectile->prev_y = projectile->y;
  projectile->collision_w = 16;
  projectile->collision_h = 16;
  RES_PlayerTextureGet(&projectile->texture);
  *projectile->sub_texture = (RDR_SubTexture){ 64, 0, 80, 16 };
  
  {
   int projectile_speed = 4;
   float a = input->mouse_x - player->x;
   float b = input->mouse_y - player->y;
   float normalise = MTH_ReciprocalSqrtF(a * a + b * b);
   projectile->vel_x = a * normalise * projectile_speed;
   projectile->vel_y = b * normalise * projectile_speed;
  }
 }
 
 return projectile;
}

static void
ETT_HelpMePleaseWhatAmIDoing(const PLT_GameInput *input,
                             const FLS_State *falling_sand_state,
                             ETT_e *e)
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
 
 int is_y_collision = 0;
 int is_x_collision = 0;
 
 for (int i = 0;
      i < vel_y;
      i += 1)
 {
  for (int x = 1;
       x < e->collision_w;
       x += 1)
  {
   if (FLS_CellAtHasFlag(falling_sand_state,
                         e->x + x,
                         e->y + (e->vel_y > 0) * e->collision_h,
                         FLS_CellFlags_solid))
   {
    is_y_collision = 1;
    e->vel_y = 0;
    goto break_y_collision_loop;
   }
  }
  e->y += dir_y;
 } break_y_collision_loop:;
 
 int x_move_amount = 0;
 int incline = 0;
 incline_loop:
 for (int i = 0;
      i < vel_x;
      i += 1)
 {
  for (int y = 1;
       y < e->collision_h;
       y += 1)
  {
   if (FLS_CellAtHasFlag(falling_sand_state,
                         e->x + x_move_amount + (e->vel_x > 0) * e->collision_w,
                         e->y + y,
                         FLS_CellFlags_solid))
   {
    if (incline < ETT_maxIncline)
    {
     int can_move_up = 1;
     for (int x = 0;
          x < e->collision_w;
          x += 1)
     {
      if (FLS_CellAtHasFlag(falling_sand_state,
                            e->x + x + x_move_amount + (e->vel_x > 0) * e->collision_w,
                            e->y,
                            FLS_CellFlags_solid))
      {
       can_move_up = 0;
       break;
      }
     }
     if (can_move_up)
     {
      e->y -= 1;
      incline += 1;
      goto incline_loop;
     }
    }
    is_x_collision = 1;
    e->vel_x = 0;
    goto break_x_collision_loop;
   }
  }
  x_move_amount += dir_x;
 } break_x_collision_loop:;
 
 e->x += x_move_amount;
}

static void
ETT_Update(const PLT_GameInput *input,
           FLS_State *falling_sand_state)
{
 ETT_e *prev = NULL;
 for (ETT_e *e = ETT_list;
      NULL != e;
      prev = e, e = e->next)
 {
  e->prev_x = e->x;
  e->prev_y = e->y;
  
  if (e->flags & ETT_Flags_gravity)
  {
   e->vel_y += ETT_gravityStrength;
  }
  
  if (e->flags & ETT_Flags_playerMovement)
  {
   int move_left = (input->is_key_down[PLT_Key_a] || input->is_key_down[PLT_Key_left]);
   int move_right = (input->is_key_down[PLT_Key_d] || input->is_key_down[PLT_Key_right]);
   int jump = (input->is_key_down[PLT_Key_w] || input->is_key_down[PLT_Key_up] || input->is_key_down[PLT_Key_space]);
   
   if (move_left)
   {
    e->vel_x = MTH_MaxI(e->vel_x - 1, -ETT_playerSpeed);
   }
   else if (move_right)
   {
    e->vel_x = MTH_MinI(e->vel_x + 1, ETT_playerSpeed);
   }
   else
   {
    e->vel_x /= 2;
   }
   
   if (jump)
   {
    e->vel_y -= ETT_playerJumpPower;
   }
   
   if (input->is_key_down[PLT_Key_mouseLeft])
   {
    ETT_PlayerProjectileMake(input, e);
   }
  }
  
  for (int y = 0;
       y < e->collision_h;
       y += 1)
  {
   for (int x = 0;
        x < e->collision_w;
        x += 1)
   {
    if (FLS_CellAtHasFlag(falling_sand_state, e->x + x, e->y + y, FLS_CellFlags_destructable) &&
        (e->flags & ETT_Flags_destructive))
    {
     FLS_CellAt(falling_sand_state, e->x + x, e->y + y) = FLS_CellKind_empty;
    }
    
    if (FLS_CellAtHasFlag(falling_sand_state, e->x + x, e->y + y, FLS_CellFlags_solid) &&
        (e->flags & ETT_Flags_destroyOnContact))
    {
     e->flags |= ETT_Flags_markedForRemoval;
    }
   }
  }
  
  if (e->flags & ETT_Flags_mobMovement)
  {
   ETT_HelpMePleaseWhatAmIDoing(input, falling_sand_state, e);
  }
  
  if (e->flags & ETT_Flags_simpleMovement)
  {
   e->x += e->vel_x;
   e->y += e->vel_y;
  }
  
  if (e->flags & ETT_Flags_markedForRemoval)
  {
   if (prev)
   {
    prev->next = e->next;
   }
   else
   {
    ETT_list = e->next;
   }
   e->next_free = ETT_arena.free_list;
   ETT_arena.free_list = e;
   
   // NOTE(tbt): somehow end up with a circle in the entity list if more than one entity is removed per frame!?!?! exit entity processing for now
   // TODO(tbt): fix me!
   return;
  }
 }
}

static void
ETT_Render(const PLT_GameInput *input,
           double accumulator)
{
 for (ETT_e *e = ETT_list;
      NULL != e;
      e = e->next)
 {
  int t = 255.0 * (accumulator / GME_timestep);
  int interpolated_x = ((t * (e->x - e->prev_x)) >> 8) + e->prev_x;
  int interpolated_y = ((t * (e->y - e->prev_y)) >> 8) + e->prev_y;
  
  if (e->flags & ETT_Flags_drawSubTexture)
  {
   RDR_DrawSubTexture(input,
                      &e->texture,
                      e->sub_texture,
                      interpolated_x,
                      interpolated_y);
  }
 }
}