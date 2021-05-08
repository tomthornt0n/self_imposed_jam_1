enum ETT_Constants
{
 ETT_animationFramesMax = 16,
};

typedef size_t ETT_Flags;
typedef enum
{
 ETT_Flags_playerMovement = 1 << 0,
 ETT_Flags_drawSubTexture    = 1 << 1,
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
 
 RES_Texture texture;
 RDR_SubTexture sub_texture[ETT_animationFramesMax];
 size_t animation_frames_count;
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
 player->x = x;
 player->y = y;
 RES_PlayerTextureGet(&player->texture); // NOTE(tbt): player art from https://opengameart.org/content/a-platformer-in-the-forest
 player->sub_texture[0] = (RDR_SubTexture){ 0, 0, 16, 32 };
 player->sub_texture[1] = (RDR_SubTexture){ 0, 0, 32, 48 };
 player->sub_texture[2] = (RDR_SubTexture){ 0, 0, 48, 64 };
 player->sub_texture[3] = (RDR_SubTexture){ 0, 0, 64, 80 };
 player->animation_frames_count = 4;
 
 return player;
}

static void
ETT_TreeDescend(const PLT_GameInput *input,
                ETT_e *e,
                int x, int y)
{
 if (e->flags & ETT_Flags_playerMovement)
 {
  const int player_speed = 1;
  
  int max_x = PLT_gameFixedW - (e->sub_texture->x1 - e->sub_texture->x0);
  
  if (input->is_key_down[PLT_Key_a] ||
      input->is_key_down[PLT_Key_left])
  {
   e->x = MAT_ClampI(e->x - player_speed, 0, max_x);
  }
  else if (input->is_key_down[PLT_Key_d] ||
           input->is_key_down[PLT_Key_right])
  {
   e->x = MAT_ClampI(e->x + player_speed, 0, max_x);
  }
 }
 
 x += e->x;
 y += e->y;
 
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
  ETT_TreeDescend(input, child, x, y);
 }
}

static void
ETT_UpdateAndRender(const PLT_GameInput *input)
{
 for (ETT_e *e = ETT_topLevel;
      NULL != e;
      e = e->sibling_next)
 {
  ETT_TreeDescend(input, e, 0, 0);
 }
}