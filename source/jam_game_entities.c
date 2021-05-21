enum ETT_Constants
{
 ETT_maxEntities = 512,
 ETT_maxAnimationFrames = 16,
 
 ETT_gravityStrength = 1,
 
 ETT_playerJumpPower = 2,
 ETT_playerJumpMax = 7,
 
 ETT_healthBarWidth = 36,
};

typedef size_t ETT_Flags;
typedef enum
{
 ETT_Flags_drawSubTexture           = 1 <<  0,
 ETT_Flags_drawHealthBar            = 1 <<  1,
 
 ETT_Flags_isPlayer                 = 1 <<  2,
 ETT_Flags_isMonster                = 1 <<  3,
 
 ETT_Flags_humanoidAI               = 1 <<  4,
 
 ETT_Flags_simpleMovement           = 1 <<  5,
 ETT_Flags_mobMovement              = 1 <<  6,
 ETT_Flags_gravity                  = 1 <<  7,
 
 ETT_Flags_destructive              = 1 <<  8,
 
 ETT_Flags_removeOnContact          = 1 <<  9,
 ETT_Flags_removeAfterTimer         = 1 << 10,
 ETT_Flags_removeWhenHealthDepleted = 1 << 11,
 
 ETT_Flags_playAnimation            = 1 << 12,
 ETT_Flags_walkAnimation            = 1 << 13,
 
 ETT_Flags_dealDamage               = 1 << 14,
 ETT_Flags_takeDamage               = 1 << 15,
 ETT_Flags_knockBack                = 1 << 16,
 
 ETT_Flags_fireProjectile           = 1 << 17,
} ETT_Flags_ENUM;

typedef enum
{
 ETT_ProjectileKind_player,
 ETT_ProjectileKind_golem,
} ETT_ProjectileKind;

typedef enum
{
 ETT_State_default,
 ETT_State_chasing,
 ETT_State_jumping,
 ETT_State_removed,
} ETT_State;

typedef unsigned char ETT_CollisionMask;
typedef enum
{
 ETT_CollisionMask_player   = 1 << 0,
 ETT_CollisionMask_monsters = 1 << 1,
} ETT_CollisionMask_ENUM;

typedef struct ETT_Entity ETT_Entity;
struct ETT_Entity
{
 ETT_Entity *next;
 ETT_Entity *next_free;
 
 ETT_Flags flags;
 ETT_State state;
 ETT_CollisionMask collision_mask;
 
 float timer;
 
 int x;
 int y;
 int prev_x;
 int prev_y;
 int collision_w;
 int collision_h;
 int vel_x;
 int vel_y;
 
 RES_Texture texture;
 int is_v_flip;
 RDR_SubTexture sub_texture[ETT_maxAnimationFrames];
 size_t animation_loop_begin;
 size_t animation_loop_end;
 size_t animation_frame;
 int animation_frame_time;
 
 int speed;
 int jump_charge;
 int max_walkable_incline;
 
 ETT_ProjectileKind projectile_kind;
 float fire_rate;
 
 int chase_range;
 
 int health;
 int max_health;
};

static struct
{
 // NOTE(tbt): hybrid pool/arena allocator
 ETT_Entity pool[ETT_maxEntities];
 size_t arena_index;
 ETT_Entity *free_list;
 ETT_Entity *active_list;
} ETT_entities = {0};


static ETT_Entity *
ETT_Push(void)
{
 ETT_Entity *result = NULL;
 
 if (ETT_entities.free_list)
 {
  result = ETT_entities.free_list;
  ETT_entities.free_list = ETT_entities.free_list->next_free;
 }
 else if (ETT_entities.arena_index < ETT_maxEntities - 1)
 {
  result = &ETT_entities.pool[ETT_entities.arena_index];
  ETT_entities.arena_index += 1;
 }
 else
 {
  return NULL;
 }
 
 memset(result, 0, sizeof(*result));
 result->next = ETT_entities.active_list;
 ETT_entities.active_list = result;
 
 return result;
}

static void
ETT_FreeAll(void)
{
 memset(&ETT_entities, 0, sizeof(ETT_entities));
}

static ETT_Entity *
ETT_PlayerMake(int x, int y)
{
 ETT_Entity *player = ETT_Push();
 
 if (player)
 {
  player->flags |= ETT_Flags_isPlayer;
  player->flags |= ETT_Flags_drawSubTexture;
  player->flags |= ETT_Flags_mobMovement;
  player->flags |= ETT_Flags_gravity;
  player->flags |= ETT_Flags_walkAnimation;
  player->flags |= ETT_Flags_takeDamage;
  player->flags |= ETT_Flags_drawHealthBar;
  player->flags |= ETT_Flags_knockBack;
  player->x = x;
  player->y = y;
  player->collision_w = 16;
  player->collision_h = 33;
  player->collision_mask = ETT_CollisionMask_player;
  player->max_walkable_incline = 2;
  RES_SpritesheetTextureGet(&player->texture);
  player->animation_loop_begin = 0;
  player->animation_loop_end = 3;
  player->animation_frame_time = 15;
  player->sub_texture[0] = (RDR_SubTexture){  0,  0, 16, 32 };
  player->sub_texture[1] = (RDR_SubTexture){ 16,  0, 32, 32 };
  player->sub_texture[2] = (RDR_SubTexture){ 32,  0, 48, 32 };
  player->sub_texture[3] = (RDR_SubTexture){ 48,  0, 64, 32 };
  player->fire_rate = 0.125f;
  player->projectile_kind = ETT_ProjectileKind_player;
  player->health = 512;
  player->max_health = 512;
  player->speed = 2;
 }
 
 return player;
}

static void
ETT_SmokeParticlesMake(int count,
                       int x, int y)
{
 for (int i = 0;
      i < count;
      i += 1)
 {
  ETT_Entity *particle = ETT_Push();
  
  const int speed = 1;
  const float life = 0.8f;
  
  if (particle)
  {
   particle->flags |= ETT_Flags_drawSubTexture;
   particle->flags |= ETT_Flags_simpleMovement;
   particle->flags |= ETT_Flags_removeAfterTimer;
   particle->x = x;
   particle->y = y;
   particle->collision_w = 16;
   particle->collision_h = 16;
   RES_SpritesheetTextureGet(&particle->texture);
   particle->sub_texture[0] = (RDR_SubTexture){ 64, 16, 80, 32 };
   particle->vel_x = RNG_RandIntNext(-speed, speed);
   particle->vel_y = RNG_RandIntNext(-speed, speed);
   particle->timer = life * (float)RNG_RandIntNext(0, 1000) / 1000;
  }
 }
}

static ETT_Entity *
ETT_GolemMake(int x, int y)
{
 ETT_Entity *golem = ETT_Push();
 
 if (golem)
 {
  ETT_SmokeParticlesMake(64, x, y);
  GME_monsterCount += 1;
  
  golem->flags |= ETT_Flags_isMonster;
  golem->flags |= ETT_Flags_humanoidAI;
  golem->flags |= ETT_Flags_drawSubTexture;
  golem->flags |= ETT_Flags_mobMovement;
  golem->flags |= ETT_Flags_gravity;
  golem->flags |= ETT_Flags_walkAnimation;
  golem->flags |= ETT_Flags_removeWhenHealthDepleted;
  golem->flags |= ETT_Flags_drawHealthBar;
  golem->flags |= ETT_Flags_takeDamage;
  golem->flags |= ETT_Flags_knockBack;
  golem->x = x;
  golem->y = y;
  golem->collision_w = 16;
  golem->collision_h = 33;
  golem->collision_mask = ETT_CollisionMask_monsters;
  golem->max_walkable_incline = 6;
  RES_SpritesheetTextureGet(&golem->texture);
  golem->animation_loop_begin = 0;
  golem->animation_loop_end = 3;
  golem->animation_frame_time = 15;
  golem->sub_texture[0] = (RDR_SubTexture){  0, 32, 16, 64 };
  golem->sub_texture[1] = (RDR_SubTexture){ 16, 32, 32, 64 };
  golem->sub_texture[2] = (RDR_SubTexture){ 32, 32, 48, 64 };
  golem->sub_texture[3] = (RDR_SubTexture){ 48, 32, 64, 64 };
  golem->fire_rate = 0.75f;
  golem->projectile_kind = ETT_ProjectileKind_golem;
  golem->max_health = 255;
  golem->health = 255;
  golem->speed = 1;
  golem->chase_range = 256;
 }
 
 return golem;
}

static ETT_Entity *
ETT_ProjectileMake(ETT_ProjectileKind kind,
                   ETT_Entity *parent,
                   int source_x,
                   int source_y,
                   int target_x,
                   int target_y)
{
 ETT_Entity *projectile = ETT_Push();
 
 if (projectile)
 {
  projectile->flags |= ETT_Flags_drawSubTexture;
  projectile->flags |= ETT_Flags_simpleMovement;
  projectile->flags |= ETT_Flags_destructive;
  projectile->flags |= ETT_Flags_removeOnContact;
  projectile->flags |= ETT_Flags_removeAfterTimer;
  projectile->flags |= ETT_Flags_dealDamage;
  projectile->x = source_x;
  projectile->y = source_y;
  projectile->prev_x = projectile->x;
  projectile->prev_y = projectile->y;
  
  float speed;
  
  switch(kind)
  {
   case ETT_ProjectileKind_player:
   {
    projectile->collision_mask = ETT_CollisionMask_monsters;
    projectile->timer = 0.5f;
    projectile->collision_w = 8;
    projectile->collision_h = 8;
    RES_SpritesheetTextureGet(&projectile->texture);
    projectile->sub_texture[0] = (RDR_SubTexture){ 64, 0, 72, 8 };
    projectile->health = 8;
    speed = 7.0f;
   } break;
   
   case ETT_ProjectileKind_golem:
   {
    projectile->collision_mask = ETT_CollisionMask_player;
    projectile->timer = 1.0f;
    projectile->collision_w = 8;
    projectile->collision_h = 8;
    RES_SpritesheetTextureGet(&projectile->texture);
    projectile->sub_texture[0] = (RDR_SubTexture){ 72, 0, 80, 8 };
    projectile->health = 4;
    speed = 5.0f;
   } break;
  }
  
  {
   float a = target_x - source_x;
   float b = target_y - source_y;
   float normalise = MTH_ReciprocalSqrtF(a * a + b * b);
   projectile->vel_x = a * normalise * speed;
   projectile->vel_y = b * normalise * speed;
  }
 }
 
 return projectile;
}

// NOTE(tbt): move mobs on falling sand sim terrain terrain
static void
ETT_HelpMePleaseWhatAmIDoing(const PLT_GameInput *input,
                             const FLS_State *falling_sand_state,
                             ETT_Entity *e)
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
                         e->y + y - incline,
                         FLS_CellFlags_solid))
   {
    if (incline < e->max_walkable_incline)
    {
     int can_move_up = 1;
     for (int x = 0;
          x < e->collision_w;
          x += 1)
     {
      if (FLS_CellAtHasFlag(falling_sand_state,
                            e->x + x + x_move_amount + (e->vel_x > 0) * e->collision_w,
                            e->y - incline,
                            FLS_CellFlags_solid))
      {
       can_move_up = 0;
       break;
      }
     }
     if (can_move_up)
     {
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
 e->y -= incline;
}

static void
ETT_Update(const PLT_GameInput *input,
           FLS_State *falling_sand_state)
{
 static size_t tick_count = 0;
 
 static ETT_Entity *player = NULL;
 
 ETT_Entity *prev = NULL;
 for (ETT_Entity *e = ETT_entities.active_list;
      NULL != e;
      e = e->next)
 {
  int projectile_target_x;
  int projectile_target_y;
  
  e->prev_x = e->x;
  e->prev_y = e->y;
  
  e->timer -= GME_timestep;
  
  if (e->flags & ETT_Flags_gravity)
  {
   e->vel_y += ETT_gravityStrength;
  }
  
  if (e->flags & ETT_Flags_playAnimation &&
      0 == (tick_count % e->animation_frame_time))
  {
   e->animation_frame += 1;
   if (e->animation_frame > e->animation_loop_end)
   {
    e->animation_frame = e->animation_loop_begin;
   }
  }
  
  if (e->flags & ETT_Flags_isPlayer)
  {
   player = e;
   
   int move_left = (input->is_key_down[PLT_Key_a] || input->is_key_down[PLT_Key_left]);
   int move_right = (input->is_key_down[PLT_Key_d] || input->is_key_down[PLT_Key_right]);
   int jump = (input->is_key_down[PLT_Key_w] || input->is_key_down[PLT_Key_up] || input->is_key_down[PLT_Key_space]);
   
   if (move_left)
   {
    e->vel_x = MTH_MaxI(e->vel_x - 1, -e->speed);
   }
   else if (move_right)
   {
    e->vel_x = MTH_MinI(e->vel_x + 1, e->speed);
   }
   else
   {
    e->vel_x /= 2;
   }
   
   e->state = ETT_State_jumping;
   for (int x = 0;
        x < e->collision_w;
        x += 1)
   {
    if (FLS_CellAtHasFlag(falling_sand_state, e->x + x, e->y + e->collision_h, FLS_CellFlags_solid))
    {
     e->state = ETT_State_default;
     break;
    }
   }
   
   if (jump)
   {
    if (ETT_State_jumping != e->state)
    {
     e->vel_y -= ETT_playerJumpPower;
    }
    else if (e->jump_charge < ETT_playerJumpMax)
    {
     e->vel_y -= ETT_playerJumpPower;
     e->jump_charge += 1;
    }
   }
   else if (ETT_State_jumping != e->state)
   {
    e->jump_charge = 0;
   }
   
   e->is_v_flip = input->mouse_x < e->x;
   
   if (input->is_key_down[PLT_Key_mouseLeft])
   {
    e->flags |= ETT_Flags_fireProjectile;
    projectile_target_x = input->mouse_x;
    projectile_target_y = input->mouse_y;
   }
   else
   {
    e->flags &= ~ETT_Flags_fireProjectile;
   }
  }
  
  if (e->flags & ETT_Flags_humanoidAI)
  {
   if (player)
   {
    int x_dist = MTH_AbsI(e->x - player->x);
    
    if (ETT_State_chasing == e->state)
    {
     e->flags |= ETT_Flags_fireProjectile;
     projectile_target_x = player->x + player->collision_w / 2;
     projectile_target_y = player->y + player->collision_h / 2;
     
     if (player->x < e->x && x_dist < e->chase_range)
     {
      if (e->vel_x > -e->speed)
      {
       e->vel_x -= 1;
      }
      e->is_v_flip = 1;
     }
     else if (player->x > e->x && x_dist < e->chase_range)
     {
      if (e->vel_x < e->speed)
      {
       e->vel_x += 1;
      }
      e->is_v_flip = 0;
     }
     else
     {
      e->state = ETT_State_default;
     }
    }
    else
    {
     e->flags &= ~ETT_Flags_fireProjectile;
     
     if (x_dist > player->collision_w && x_dist < e->chase_range)
     {
      e->state = ETT_State_chasing;
     }
    }
   }
  }
  
  if (e->flags & ETT_Flags_fireProjectile)
  {
   if (e->timer < 0.0f)
   {
    ETT_ProjectileMake(e->projectile_kind,
                       e,
                       e->x,
                       e->y + e->collision_h / 2,
                       projectile_target_x,
                       projectile_target_y);
    e->timer = e->fire_rate;
   }
  }
  
  if (e->flags & ETT_Flags_walkAnimation)
  {
   if (e->state == ETT_State_jumping)
   {
    e->animation_frame = 1;
   }
   else
   {
    if (e->vel_x != 0)
    {
     e->flags |= ETT_Flags_playAnimation;
    }
    else
    {
     e->flags &= ~ETT_Flags_playAnimation;
     e->animation_frame = 0;
    }
   }
  }
  
  // NOTE(tbt): collisions with falling sand sim stuff
  for (int y = 0;
       y < e->collision_h;
       y += 1)
  {
   for (int x = 0;
        x < e->collision_w;
        x += 1)
   {
    if (FLS_CellAtHasFlag(falling_sand_state, e->x + x, e->y + y, FLS_CellFlags_solid) &&
        (e->flags & ETT_Flags_removeOnContact))
    {
     e->state = ETT_State_removed;
    }
    
    int radius = e->collision_w / 2;
    if ((x - radius) * (x - radius) + (y - radius) * (y - radius) < radius * radius &&
        FLS_CellAtHasFlag(falling_sand_state, e->x + x, e->y + y, FLS_CellFlags_destructable) &&
        (e->flags & ETT_Flags_destructive))
    {
     FLS_CellAt(falling_sand_state, e->x + x, e->y + y) = FLS_CellKind_empty;
    }
   }
  }
  
  // NOTE(tbt): collisions with other entities
  for (ETT_Entity *f = ETT_entities.active_list;
       NULL != f;
       f = f->next)
  {
   if (0 == (e->collision_mask & f->collision_mask)) { continue; }
   if (e->x > f->x + f->collision_w || e->x + e->collision_w < f->x) { continue; }
   if (e->y > f->y + f->collision_h || e->y + e->collision_h < f->y) { continue; }
   
   // NOTE(tbt): collision
   {
    if ((e->flags & ETT_Flags_dealDamage) &&
        (f->flags & ETT_Flags_takeDamage))
    {
     f->health -= e->health;
     if (f->flags & ETT_Flags_knockBack)
     {
      f->vel_x += e->vel_x;
      f->vel_y += e->vel_y;
     }
     
     if (e->flags & ETT_Flags_removeOnContact)
     {
      e->state = ETT_State_removed;
     }
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
  
  if (((e->flags & ETT_Flags_removeWhenHealthDepleted) && e->health <= 0) ||
      ((e->flags & ETT_Flags_removeAfterTimer) && e->timer <= 0.0f))
  {
   e->state = ETT_State_removed;
  }
  
  if (ETT_State_removed == e->state)
  {
   if (e->flags & ETT_Flags_isMonster)
   {
    GME_monsterCount -= 1;
   }
   
   if (prev)
   {
    prev->next = e->next;
   }
   else
   {
    ETT_entities.active_list = e->next;
   }
   e->next_free = ETT_entities.free_list;
   ETT_entities.free_list = e;
  }
  else
  {
   prev = e;
  }
 }
 
 tick_count += 1;
}

static void
ETT_Render(const PLT_GameInput *input,
           double accumulator)
{
 for (ETT_Entity *e = ETT_entities.active_list;
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
                      &e->sub_texture[e->animation_frame],
                      interpolated_x,
                      interpolated_y,
                      e->is_v_flip * RDR_DrawSubTextureFlags_vFlip);
  }
  
  if (e->flags & ETT_Flags_drawHealthBar)
  {
   int x = e->x + (e->collision_w - ETT_healthBarWidth) / 2;
   int y = e->y;
   int w = ((float)e->health / (float)e->max_health) * ETT_healthBarWidth;
   int h = 4;
   
   RDR_DrawRectangleFill(input,
                         RectLit(x, y, ETT_healthBarWidth, h),
                         ColLit(8, 4, 4, 255));
   RDR_DrawRectangleFill(input,
                         RectLit(x, y, w, h),
                         ColLit(24, 12, 200, 255));
  }
 }
}