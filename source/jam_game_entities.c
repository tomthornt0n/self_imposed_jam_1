enum ETT_Constants
{
    ETT_maxEntities = 4096,
    ETT_maxAnimationFrames = 16,
    
    ETT_gravityStrength = 1,
    
    ETT_playerJumpPower = 2,
    ETT_playerJumpMax = 7,
    
    ETT_progressBarWidth = 36,
    ETT_progressBarHeight = 4,
};

typedef size_t ETT_Flags;
typedef enum
{
    ETT_Flags_drawSubTexture           = 1 <<  0,
    ETT_Flags_drawHealthBar            = 1 <<  1,
    ETT_Flags_drawTimerBar             = 1 <<  2,
    ETT_Flags_drawBobAnimation         = 1 <<  3,
    
    ETT_Flags_isPlayer                 = 1 <<  4,
    ETT_Flags_isMonster                = 1 <<  5,
    
    ETT_Flags_humanoidAI               = 1 <<  6,
    ETT_Flags_flyingAI                 = 1 <<  7,
    ETT_Flags_fireAtPlayer             = 1 <<  8,
    
    ETT_Flags_simpleMovement           = 1 <<  9,
    ETT_Flags_mobMovement              = 1 << 10,
    ETT_Flags_gravity                  = 1 << 11,
    ETT_Flags_bounceOnContact          = 1 << 12,
    
    ETT_Flags_destructive              = 1 << 13,
    
    ETT_Flags_removeOnContact          = 1 << 14,
    ETT_Flags_removeAfterTimer         = 1 << 15,
    ETT_Flags_removeWhenHealthDepleted = 1 << 16,
    ETT_Flags_removeWhenSubmerged      = 1 << 17,
    
    ETT_Flags_playAnimation            = 1 << 18,
    ETT_Flags_walkAnimation            = 1 << 19,
    
    ETT_Flags_dealDamage               = 1 << 20,
    ETT_Flags_takeDamage               = 1 << 21,
    ETT_Flags_dropHealth               = 1 << 22,
    
    ETT_Flags_knockBack                = 1 << 23,
    ETT_Flags_knockedBack              = 1 << 24,
    
    ETT_Flags_fireProjectile           = 1 << 25,
    
    ETT_Flags_particlesWhenRemoved     = 1 << 26,
    ETT_Flags_turnToSandWhenRemoved    = 1 << 27,
    
    ETT_Flags_attractedToPlayer        = 1 << 28,
    
    ETT_Flags_isCloud                  = 1 << 29,
    ETT_Flags_rain                     = 1 << 30,
    
    ETT_Flags_drown                    = 1 << 31,
} ETT_Flags_ENUM;

typedef enum
{
    ETT_ProjectileKind_player,
    ETT_ProjectileKind_sand,
    ETT_ProjectileKind_dirt,
    ETT_ProjectileKind_stone,
    ETT_ProjectileKind_slime,
    ETT_ProjectileKind_lightning,
} ETT_ProjectileKind;

typedef enum
{
    ETT_State_default,
    ETT_State_jumping,
    ETT_State_swimming,
    ETT_State_removed,
    ETT_State_hidden,
} ETT_State;

typedef unsigned char ETT_CollisionMask;
typedef enum
{
    ETT_CollisionLayer_fallingSand,
    ETT_CollisionLayer_player,
    ETT_CollisionLayer_monsters,
    ETT_CollisionLayer_projectiles,
    ETT_CollisionLayer_pickups,
} ETT_CollisionLayer;

typedef struct ETT_Entity ETT_Entity;
struct ETT_Entity
{
    ETT_Entity *next;
    ETT_Entity *next_free;
    
    ETT_Flags flags;
    ETT_State state;
    ETT_CollisionLayer collision_layer;
    ETT_CollisionMask collide_with;
    
    float timer;
    float timer_max;
    
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
    float fire_timer;
    
    int range_max;
    int range_min;
    
    int health;
    int max_health;
    int health_to_drop;
    
    FLS_CellKind turn_to_sand_kind;
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
        player->flags |= ETT_Flags_knockedBack;
        player->flags |= ETT_Flags_particlesWhenRemoved;
        player->flags |= ETT_Flags_removeWhenSubmerged;
        //player->flags |= ETT_Flags_removeWhenHealthDepleted;
        player->flags |= ETT_Flags_drown;
        player->x = x;
        player->y = y;
        player->collision_w = 16;
        player->collision_h = 24;
        player->collision_layer = ETT_CollisionLayer_player;
        player->collide_with = (Bit(ETT_CollisionLayer_fallingSand) |
                                Bit(ETT_CollisionLayer_projectiles) |
                                Bit(ETT_CollisionLayer_pickups));
        player->max_walkable_incline = 2;
        RES_SpritesheetTextureGet(&player->texture);
        player->animation_loop_begin = 0;
        player->animation_loop_end = 3;
        player->animation_frame_time = 15;
        player->sub_texture[0] = (RDR_SubTexture){  0,  0, 16, 23 };
        player->sub_texture[1] = (RDR_SubTexture){ 16,  0, 32, 23 };
        player->sub_texture[2] = (RDR_SubTexture){  0,  0, 16, 23 };
        player->sub_texture[3] = (RDR_SubTexture){ 32,  0, 48, 23 };
        player->fire_rate = 0.125f;
        player->projectile_kind = ETT_ProjectileKind_player;
        player->health = 256;
        player->max_health = 256;
        player->speed = 2;
        player->timer_max = 0.75; // NOTE(tbt): time in seconds to start drowning
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
            particle->sub_texture[0] = (RDR_SubTexture){ 48, 16, 64, 32 };
            particle->vel_x = RNG_RandIntNext(-speed, speed);
            particle->vel_y = RNG_RandIntNext(-speed, speed);
            particle->timer = life * (float)RNG_RandIntNext(0, 1000) / 1000;
        }
    }
}

static ETT_Entity *
ETT_SandGolemMake(int x, int y)
{
    ETT_Entity *golem = ETT_Push();
    
    if (golem)
    {
        ETT_SmokeParticlesMake(64, x, y);
        GME_state.monster_count += 1;
        
        golem->flags |= ETT_Flags_isMonster;
        golem->flags |= ETT_Flags_humanoidAI;
        golem->flags |= ETT_Flags_fireAtPlayer;
        golem->flags |= ETT_Flags_drawSubTexture;
        golem->flags |= ETT_Flags_mobMovement;
        golem->flags |= ETT_Flags_gravity;
        golem->flags |= ETT_Flags_walkAnimation;
        golem->flags |= ETT_Flags_removeWhenHealthDepleted;
        golem->flags |= ETT_Flags_removeWhenSubmerged;
        golem->flags |= ETT_Flags_drawHealthBar;
        golem->flags |= ETT_Flags_takeDamage;
        golem->flags |= ETT_Flags_dropHealth;
        golem->flags |= ETT_Flags_knockedBack;
        golem->flags |= ETT_Flags_turnToSandWhenRemoved;
        golem->flags |= ETT_Flags_drown;
        golem->x = x;
        golem->y = y;
        golem->collision_w = 16;
        golem->collision_h = 22;
        golem->collision_layer = ETT_CollisionLayer_monsters;
        golem->collide_with = (Bit(ETT_CollisionLayer_fallingSand) |
                               Bit(ETT_CollisionLayer_projectiles));
        golem->max_walkable_incline = 6;
        RES_SpritesheetTextureGet(&golem->texture);
        golem->animation_loop_begin = 0;
        golem->animation_loop_end = 3;
        golem->animation_frame_time = 15;
        golem->sub_texture[0] = (RDR_SubTexture){  0, 23, 16, 44 };
        golem->sub_texture[1] = (RDR_SubTexture){ 16, 23, 32, 44 };
        golem->sub_texture[2] = (RDR_SubTexture){  0, 23, 16, 44 };
        golem->sub_texture[3] = (RDR_SubTexture){ 32, 23, 48, 44 };
        golem->turn_to_sand_kind = FLS_CellKind_sand;
        golem->fire_rate = 0.75f;
        golem->projectile_kind = ETT_ProjectileKind_sand;
        golem->max_health = 255;
        golem->health = 255;
        golem->health_to_drop = 4 * GME_state.wave;
        golem->speed = 1;
        golem->range_max = 256;
        golem->timer_max = 0.5; // NOTE(tbt): time in seconds to start drowning
    }
    
    return golem;
}

static ETT_Entity *
ETT_StoneGolemMake(int x, int y)
{
    ETT_Entity *golem = ETT_Push();
    
    if (golem)
    {
        ETT_SmokeParticlesMake(64, x, y);
        GME_state.monster_count += 1;
        
        golem->flags |= ETT_Flags_isMonster;
        golem->flags |= ETT_Flags_humanoidAI;
        golem->flags |= ETT_Flags_fireAtPlayer;
        golem->flags |= ETT_Flags_drawSubTexture;
        golem->flags |= ETT_Flags_mobMovement;
        golem->flags |= ETT_Flags_gravity;
        golem->flags |= ETT_Flags_walkAnimation;
        golem->flags |= ETT_Flags_removeWhenHealthDepleted;
        golem->flags |= ETT_Flags_removeWhenSubmerged;
        golem->flags |= ETT_Flags_drawHealthBar;
        golem->flags |= ETT_Flags_takeDamage;
        golem->flags |= ETT_Flags_dropHealth;
        golem->flags |= ETT_Flags_knockedBack;
        golem->flags |= ETT_Flags_turnToSandWhenRemoved;
        golem->x = x;
        golem->y = y;
        golem->collision_w = 16;
        golem->collision_h = 21;
        golem->collision_layer = ETT_CollisionLayer_monsters;
        golem->collide_with = (Bit(ETT_CollisionLayer_fallingSand) |
                               Bit(ETT_CollisionLayer_projectiles));
        golem->max_walkable_incline = 6;
        RES_SpritesheetTextureGet(&golem->texture);
        golem->animation_loop_begin = 0;
        golem->animation_loop_end = 3;
        golem->animation_frame_time = 15;
        golem->sub_texture[0] = (RDR_SubTexture){  0, 77, 16, 98 };
        golem->sub_texture[1] = (RDR_SubTexture){ 16, 77, 32, 98 };
        golem->sub_texture[2] = (RDR_SubTexture){  0, 77, 16, 98 };
        golem->sub_texture[3] = (RDR_SubTexture){ 32, 77, 48, 98 };
        golem->turn_to_sand_kind = FLS_CellKind_stone;
        golem->fire_rate = 0.75f;
        golem->projectile_kind = ETT_ProjectileKind_stone;
        golem->max_health = 512;
        golem->health = 512;
        golem->health_to_drop = 16 * GME_state.wave;
        golem->speed = 1;
        golem->range_max = 256;
    }
    
    return golem;
}

static ETT_Entity *
ETT_DirtGolemMake(int x, int y)
{
    ETT_Entity *golem = ETT_Push();
    
    if (golem)
    {
        ETT_SmokeParticlesMake(64, x, y);
        GME_state.monster_count += 1;
        
        golem->flags |= ETT_Flags_isMonster;
        golem->flags |= ETT_Flags_humanoidAI;
        golem->flags |= ETT_Flags_fireAtPlayer;
        golem->flags |= ETT_Flags_drawSubTexture;
        golem->flags |= ETT_Flags_mobMovement;
        golem->flags |= ETT_Flags_gravity;
        golem->flags |= ETT_Flags_walkAnimation;
        golem->flags |= ETT_Flags_removeWhenHealthDepleted;
        golem->flags |= ETT_Flags_removeWhenSubmerged;
        golem->flags |= ETT_Flags_drawHealthBar;
        golem->flags |= ETT_Flags_takeDamage;
        golem->flags |= ETT_Flags_dropHealth;
        golem->flags |= ETT_Flags_knockedBack;
        golem->flags |= ETT_Flags_turnToSandWhenRemoved;
        golem->x = x;
        golem->y = y;
        golem->collision_w = 16;
        golem->collision_h = 22;
        golem->collision_layer = ETT_CollisionLayer_monsters;
        golem->collide_with = (Bit(ETT_CollisionLayer_fallingSand) |
                               Bit(ETT_CollisionLayer_projectiles));
        golem->max_walkable_incline = 6;
        RES_SpritesheetTextureGet(&golem->texture);
        golem->animation_loop_begin = 0;
        golem->animation_loop_end = 3;
        golem->animation_frame_time = 15;
        golem->sub_texture[0] = (RDR_SubTexture){  0, 44, 16, 65 };
        golem->sub_texture[1] = (RDR_SubTexture){ 16, 44, 32, 65 };
        golem->sub_texture[2] = (RDR_SubTexture){  0, 44, 16, 65 };
        golem->sub_texture[3] = (RDR_SubTexture){ 32, 44, 48, 65 };
        golem->turn_to_sand_kind = FLS_CellKind_dirt;
        golem->fire_rate = 0.75f;
        golem->projectile_kind = ETT_ProjectileKind_dirt;
        golem->max_health = 255;
        golem->health = 255;
        golem->health_to_drop = 5 * GME_state.wave;
        golem->speed = 1;
        golem->range_max = 256;
        golem->range_min = 24;
    }
    
    return golem;
}

static ETT_Entity *
ETT_SlimeMake(int x, int y)
{
    ETT_Entity *slime = ETT_Push();
    
    if (slime)
    {
        ETT_SmokeParticlesMake(64, x, y);
        GME_state.monster_count += 1;
        
        slime->flags |= ETT_Flags_isMonster;
        slime->flags |= ETT_Flags_humanoidAI;
        slime->flags |= ETT_Flags_fireAtPlayer;
        slime->flags |= ETT_Flags_drawSubTexture;
        slime->flags |= ETT_Flags_mobMovement;
        slime->flags |= ETT_Flags_gravity;
        slime->flags |= ETT_Flags_walkAnimation;
        slime->flags |= ETT_Flags_removeWhenHealthDepleted;
        slime->flags |= ETT_Flags_removeWhenSubmerged;
        slime->flags |= ETT_Flags_drawHealthBar;
        slime->flags |= ETT_Flags_takeDamage;
        slime->flags |= ETT_Flags_dropHealth;
        slime->flags |= ETT_Flags_knockedBack;
        slime->flags |= ETT_Flags_turnToSandWhenRemoved;
        slime->x = x;
        slime->y = y;
        slime->collision_w = 16;
        slime->collision_h = 12;
        slime->collision_layer = ETT_CollisionLayer_monsters;
        slime->collide_with = (Bit(ETT_CollisionLayer_fallingSand) |
                               Bit(ETT_CollisionLayer_projectiles));
        slime->max_walkable_incline = 6;
        RES_SpritesheetTextureGet(&slime->texture);
        slime->animation_loop_begin = 0;
        slime->animation_loop_end = 3;
        slime->animation_frame_time = 15;
        slime->sub_texture[0] = (RDR_SubTexture){  0, 65, 16, 77 };
        slime->sub_texture[1] = (RDR_SubTexture){ 16, 65, 32, 77 };
        slime->sub_texture[2] = (RDR_SubTexture){  0, 65, 16, 77 };
        slime->sub_texture[3] = (RDR_SubTexture){ 32, 65, 48, 77 };
        slime->turn_to_sand_kind = FLS_CellKind_water;
        slime->fire_rate = 0.4f;
        slime->projectile_kind = ETT_ProjectileKind_slime;
        slime->max_health = 32;
        slime->health = 32;
        slime->health_to_drop = GME_state.wave;
        slime->speed = 1;
        slime->range_max = 256;
    }
    
    return slime;
}

static ETT_Entity *
ETT_CloudMake(int x, int y)
{
    ETT_Entity *cloud = ETT_Push();
    
    if (cloud)
    {
        ETT_SmokeParticlesMake(64, x, y);
        GME_state.monster_count += 1;
        
        cloud->flags |= ETT_Flags_isMonster;
        cloud->flags |= ETT_Flags_flyingAI;
        cloud->flags |= ETT_Flags_drawSubTexture;
        cloud->flags |= ETT_Flags_mobMovement;
        cloud->flags |= ETT_Flags_removeWhenHealthDepleted;
        cloud->flags |= ETT_Flags_drawHealthBar;
        cloud->flags |= ETT_Flags_takeDamage;
        cloud->flags |= ETT_Flags_dropHealth;
        cloud->flags |= ETT_Flags_turnToSandWhenRemoved;
        cloud->flags |= ETT_Flags_fireProjectile;
        cloud->flags |= ETT_Flags_isCloud;
        cloud->x = x;
        cloud->y = y;
        cloud->collision_w = 35;
        cloud->collision_h = 16;
        cloud->collision_layer = ETT_CollisionLayer_monsters;
        cloud->collide_with = (Bit(ETT_CollisionLayer_fallingSand) |
                               Bit(ETT_CollisionLayer_projectiles));
        RES_SpritesheetTextureGet(&cloud->texture);
        cloud->sub_texture[0] = (RDR_SubTexture){ 0, 98, 35, 114 };
        cloud->turn_to_sand_kind = FLS_CellKind_water;
        cloud->fire_rate = 2.0f;
        cloud->projectile_kind = ETT_ProjectileKind_lightning;
        cloud->max_health = 32;
        cloud->health = 32;
        cloud->health_to_drop = (GME_state.wave / 2) * 4;
        cloud->speed = 1;
        cloud->range_max = 128;
    }
    
    return cloud;
}

static void
ETT_HealthPickupMake(int count,
                     int randomness,
                     int x,
                     int y)
{
    randomness /= 2;
    
    while (count)
    {
        ETT_Entity *particle = ETT_Push();
        if (particle)
        {
            particle->flags |= ETT_Flags_drawSubTexture;
            particle->flags |= ETT_Flags_drawBobAnimation;
            particle->flags |= ETT_Flags_removeOnContact;
            particle->flags |= ETT_Flags_removeAfterTimer;
            particle->flags |= ETT_Flags_dealDamage;
            particle->flags |= ETT_Flags_attractedToPlayer;
            particle->collision_layer = ETT_CollisionLayer_pickups;
            particle->collide_with |= Bit(ETT_CollisionLayer_player);
            particle->x = x + RNG_RandIntNext(-randomness, randomness);
            particle->y = y + RNG_RandIntNext(-randomness, randomness);
            particle->prev_x = particle->x;
            particle->prev_y = particle->y;
            particle->collision_w = 7;
            particle->collision_h = 6;
            particle->timer = 8.0f;
            RES_SpritesheetTextureGet(&particle->texture);
            particle->sub_texture[0] = (RDR_SubTexture){ 56, 9, 63, 15 };
            particle->range_max = 96;
            particle->health = -1;
            particle->speed = 4;
        }
        else
        {
            return;
        }
        
        count -= 1;
    }
}

static ETT_Entity *
ETT_LightningMake(int x, int y)
{
    ETT_Entity *lightning = ETT_Push();
    
    if (lightning)
    {
        
    }
    
    return lightning;
}

static ETT_Entity *
ETT_ProjectileMake(ETT_ProjectileKind kind,
                   ETT_Entity *parent,
                   int source_x,
                   int source_y,
                   int target_x,
                   int target_y)
{
    ETT_Entity *projectile;
    
    projectile = ETT_Push();
    
    if (projectile)
    {
        if (ETT_ProjectileKind_lightning == kind)
        {
            projectile->flags |= ETT_Flags_drawSubTexture;
            projectile->flags |= ETT_Flags_simpleMovement;
            projectile->flags |= ETT_Flags_dealDamage;
            projectile->flags |= ETT_Flags_removeOnContact;
            projectile->flags |= ETT_Flags_destructive;
            projectile->flags |= ETT_Flags_knockBack;
            projectile->collision_w = 8;
            projectile->collision_h = 11;
            projectile->x = source_x + RNG_RandIntNext(0, parent->collision_w);
            projectile->y = source_y;
            projectile->prev_x = projectile->x;
            projectile->prev_y = projectile->y;
            projectile->collision_layer = ETT_CollisionLayer_projectiles;
            projectile->collide_with |= (Bit(ETT_CollisionLayer_fallingSand) |
                                         Bit(ETT_CollisionLayer_player));
            RES_SpritesheetTextureGet(&projectile->texture);
            projectile->sub_texture[0] = (RDR_SubTexture){ 48, 40, 56, 51 };
            projectile->health = 60;
            projectile->vel_y = 13;
        }
        else
        {
            projectile->flags |= ETT_Flags_drawSubTexture;
            projectile->flags |= ETT_Flags_simpleMovement;
            projectile->flags |= ETT_Flags_dealDamage;
            projectile->collision_layer = ETT_CollisionLayer_projectiles;
            projectile->x = source_x;
            projectile->y = source_y;
            projectile->prev_x = projectile->x;
            projectile->prev_y = projectile->y;
            
            float speed;
            
            switch(kind)
            {
                case ETT_ProjectileKind_player:
                {
                    projectile->flags |= ETT_Flags_removeOnContact;
                    projectile->flags |= ETT_Flags_removeAfterTimer;
                    projectile->flags |= ETT_Flags_knockBack;
                    projectile->flags |= ETT_Flags_destructive;
                    projectile->collide_with |= (Bit(ETT_CollisionLayer_fallingSand) |
                                                 Bit(ETT_CollisionLayer_monsters));
                    projectile->timer = 0.5f;
                    projectile->collision_w = 8;
                    projectile->collision_h = 8;
                    RES_SpritesheetTextureGet(&projectile->texture);
                    projectile->sub_texture[0] = (RDR_SubTexture){ 48, 0, 56, 8 };
                    projectile->health = 8;
                    speed = 7.0f;
                } break;
                
                case ETT_ProjectileKind_sand:
                {
                    projectile->flags |= ETT_Flags_removeOnContact;
                    projectile->flags |= ETT_Flags_removeAfterTimer;
                    projectile->flags |= ETT_Flags_knockBack;
                    projectile->flags |= ETT_Flags_destructive;
                    projectile->collide_with |= (Bit(ETT_CollisionLayer_fallingSand) |
                                                 Bit(ETT_CollisionLayer_player));
                    projectile->timer = 1.0f;
                    projectile->collision_w = 8;
                    projectile->collision_h = 8;
                    RES_SpritesheetTextureGet(&projectile->texture);
                    projectile->sub_texture[0] = (RDR_SubTexture){ 56, 0, 64, 8 };
                    projectile->health = 7 + GME_state.wave / 4;
                    speed = 4.0f;
                } break;
                
                case ETT_ProjectileKind_dirt:
                {
                    projectile->flags |= ETT_Flags_removeOnContact;
                    projectile->flags |= ETT_Flags_knockBack;
                    projectile->flags |= ETT_Flags_turnToSandWhenRemoved;
                    projectile->collide_with |= (Bit(ETT_CollisionLayer_fallingSand) |
                                                 Bit(ETT_CollisionLayer_player));
                    projectile->collision_w = 8;
                    projectile->collision_h = 8;
                    RES_SpritesheetTextureGet(&projectile->texture);
                    projectile->sub_texture[0] = (RDR_SubTexture){ 48, 32, 56, 40 };
                    projectile->health = 8;
                    projectile->turn_to_sand_kind = FLS_CellKind_dirt;
                    speed = 5.0f;
                } break;
                
                case ETT_ProjectileKind_stone:
                {
                    projectile->flags |= ETT_Flags_removeAfterTimer;
                    projectile->flags |= ETT_Flags_removeOnContact;
                    projectile->collide_with |= (Bit(ETT_CollisionLayer_fallingSand) |
                                                 Bit(ETT_CollisionLayer_player));
                    projectile->timer = 1.0f;
                    projectile->collision_w = 8;
                    projectile->collision_h = 8;
                    RES_SpritesheetTextureGet(&projectile->texture);
                    projectile->sub_texture[0] = (RDR_SubTexture){ 56, 32, 64, 40 };
                    projectile->health = 1 + GME_state.wave / 8;
                    speed = 7.0f;
                } break;
                
                case ETT_ProjectileKind_slime:
                {
                    projectile->flags |= ETT_Flags_removeOnContact;
                    projectile->flags |= ETT_Flags_removeAfterTimer;
                    projectile->collide_with |= (Bit(ETT_CollisionLayer_fallingSand) |
                                                 Bit(ETT_CollisionLayer_player));
                    projectile->timer = 0.3f;
                    projectile->collision_w = 6;
                    projectile->collision_h = 6;
                    RES_SpritesheetTextureGet(&projectile->texture);
                    projectile->sub_texture[0] = (RDR_SubTexture){ 49, 9, 55, 15 };
                    projectile->health = 2;
                    speed = 8.0f;
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
             x <= e->collision_w;
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
             y <= e->collision_h;
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
        
        e->timer -= GME_timestep * GME_state.time_dilation;
        e->fire_timer -= GME_timestep * GME_state.time_dilation;
        
        if (ETT_State_hidden != e->state)
        {
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
                
                if (ETT_State_swimming == e->state)
                {
                    e->vel_y -= jump * ETT_playerJumpPower;
                    e->vel_y = MTH_MaxF(e->vel_y, - ETT_playerJumpPower);
                }
                else
                {
                    e->state = ETT_State_jumping;
                    for (int x = 0;
                         x < e->collision_w;
                         x += 1)
                    {
                        if (FLS_CellAtHasFlag(falling_sand_state, e->x + x, e->y + e->collision_h + 1, FLS_CellFlags_solid))
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
                
                if (e->health <= 0)
                {
                    ETT_SmokeParticlesMake(128, e->x, e->y);
                    
                    e->state = ETT_State_hidden;
                    e->health = 1;
                    e->timer = 3.0f;
                    e->flags &= ~ETT_Flags_takeDamage;
                    e->flags &= ~ETT_Flags_drawSubTexture;
                    e->flags &= ~ETT_Flags_drawHealthBar;
                    e->flags &= ~ETT_Flags_drawTimerBar;
                    e->flags |= ETT_Flags_removeAfterTimer;
                    GME_state.time_dilation = 4.0;
                    return;
                }
            }
            
            if ((ETT_Flags_drown & e->flags) &&
                ETT_State_swimming == e->state &&
                e->timer < 0.0)
            {
                e->health -= 1;
            }
            
            if (player)
            {
                int x_dist = MTH_AbsI(e->x - player->x);
                int y_dist = MTH_AbsI(e->y - player->y);
                int distance_from_player = MTH_SqrtF(x_dist * x_dist + y_dist * y_dist);
                
                if (e->flags & ETT_Flags_flyingAI)
                {
                    if (player->x < e->x - e->collision_w)
                    {
                        e->vel_x = -e->speed;
                    }
                    else if (player->x > e->x + e->collision_w)
                    {
                        e->vel_x = +e->speed;
                    }
                }
                
                if (distance_from_player >= e->range_min &&
                    distance_from_player < e->range_max)
                {
                    if (e->flags & ETT_Flags_isCloud)
                    {
                        e->flags |= ETT_Flags_rain;
                    }
                    
                    if (e->flags & ETT_Flags_humanoidAI)
                    {
                        if (player->x < e->x)
                        {
                            if (e->vel_x > -e->speed)
                            {
                                e->vel_x -= 1;
                            }
                            e->is_v_flip = 1;
                        }
                        else if (player->x > e->x)
                        {
                            if (e->vel_x < e->speed)
                            {
                                e->vel_x += 1;
                            }
                            e->is_v_flip = 0;
                        }
                    }
                    
                    if (e->flags & ETT_Flags_fireAtPlayer)
                    {
                        e->flags |= ETT_Flags_fireProjectile;
                        projectile_target_x = player->x + player->collision_w / 2;
                        projectile_target_y = player->y + player->collision_h / 2;
                    }
                }
                else
                {
                    if (e->flags & ETT_Flags_fireAtPlayer)
                    {
                        e->flags &= ~ETT_Flags_fireProjectile;
                    }
                    
                    if (e->flags & ETT_Flags_isCloud)
                    {
                        e->flags &= ~ETT_Flags_rain;
                    }
                }
                
                if (e->flags & ETT_Flags_attractedToPlayer &&
                    e->range_max >= distance_from_player )
                {
                    int damping = 8;
                    int from_x = e->x + e->collision_w / 2;
                    int from_y = e->y + e->collision_h / 2;
                    int to_x = player->x + player->collision_w / 2;
                    int to_y = player->y + player->collision_h / 2;
                    
                    e->x += (to_x - from_x) / damping;
                    e->y += (to_y - from_y) / damping;
                }
            }
            
            if (e->flags & ETT_Flags_rain)
            {
                for (int x = 0;
                     x < e->collision_w;
                     x += 1)
                {
                    if (!RNG_RandIntNext(0, 4))
                    {
                        FLS_CellAt(falling_sand_state, e->x + x, e->y + e->collision_h) = FLS_CellKind_water;
                    }
                }
            }
            
            if (e->flags & ETT_Flags_fireProjectile)
            {
                if (e->fire_timer < 0.0f)
                {
                    ETT_ProjectileMake(e->projectile_kind,
                                       e,
                                       e->x,
                                       e->y + e->collision_h / 2,
                                       projectile_target_x,
                                       projectile_target_y);
                    e->fire_timer = e->fire_rate;
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
            
            if (e->flags & ETT_Flags_mobMovement)
            {
                int edge_collision_w = 16;
                e->x = MTH_ClampI(e->x, edge_collision_w, PLT_gameFixedW - e->collision_w - edge_collision_w);
                e->x = MTH_ClampI(e->x, 0, PLT_gameFixedW - e->vel_x);
                
                ETT_HelpMePleaseWhatAmIDoing(input, falling_sand_state, e);
            }
            
            if (e->flags & ETT_Flags_simpleMovement)
            {
                e->x += e->vel_x;
                e->y += e->vel_y;
            }
            
            //-NOTE(tbt): collisions with falling sand sim stuff
            if (e->collide_with & Bit(ETT_CollisionLayer_fallingSand))
            {
                int solid_count = 0;
                int water_count = 0;
                
                for (int y = 0;
                     y < e->collision_h;
                     y += 1)
                {
                    for (int x = 0;
                         x < e->collision_w;
                         x += 1)
                    {
                        solid_count += FLS_CellAtHasFlag(falling_sand_state, e->x + x, e->y + y, FLS_CellFlags_solid);
                        water_count += (FLS_CellKind_water == FLS_CellAt(falling_sand_state, e->x + x, e->y + y));
                        
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
                
                if (solid_count > (e->collision_w * e->collision_h) / 2 &&
                    (e->flags & ETT_Flags_removeWhenSubmerged))
                {
                    e->state = ETT_State_removed;
                }
                
                if (water_count > (e->collision_w * e->collision_h) / 2)
                {
                    if (ETT_State_swimming != e->state &&
                        (e->flags & ETT_Flags_drown))
                    {
                        e->timer = e->timer_max;
                        e->flags |= ETT_Flags_drawTimerBar;
                    }
                    
                    e->state = ETT_State_swimming;
                }
                else
                {
                    if (e->flags & ETT_Flags_drown)
                    {
                        e->state = ETT_State_default;
                        e->flags &= ~ETT_Flags_drawTimerBar;
                    }
                }
            }
            
            //-NOTE(tbt): collisions with other entities
            for (ETT_Entity *f = ETT_entities.active_list;
                 NULL != f;
                 f = f->next)
            {
                if (e != f &&
                    (Bit(e->collision_layer) & f->collide_with) &&
                    (Bit(f->collision_layer) & e->collide_with) &&
                    !(e->x > f->x + f->collision_w || e->x + e->collision_w < f->x ||
                      e->y > f->y + f->collision_h || e->y + e->collision_h < f->y))
                {
                    if ((e->flags & ETT_Flags_isMonster) &&
                        (f->flags & ETT_Flags_isMonster))
                    {
                        // TODO(tbt): move monsters out of each other
                    }
                    
                    if ((e->flags & ETT_Flags_dealDamage) &&
                        (f->flags & ETT_Flags_takeDamage))
                    {
                        f->health = MTH_ClampI(f->health - e->health, 0, f->max_health);
                        
                        if ((e->flags & ETT_Flags_knockBack) &&
                            (f->flags & ETT_Flags_knockedBack))
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
        }
        
        if (((e->flags & ETT_Flags_removeWhenHealthDepleted) && e->health <= 0) ||
            ((e->flags & ETT_Flags_removeAfterTimer) && e->timer <= 0.0f))
        {
            e->state = ETT_State_removed;
        }
        
        if (ETT_State_removed == e->state)
        {
            if (e->flags & ETT_Flags_dropHealth)
            {
                ETT_HealthPickupMake(e->health_to_drop, 16, e->x, e->y);
            }
            
            if (e->flags & ETT_Flags_particlesWhenRemoved)
            {
                ETT_SmokeParticlesMake(96, e->x, e->y);
            }
            
            if (e->flags & ETT_Flags_turnToSandWhenRemoved)
            {
                RDR_SubTexture *sub_texture = &e->sub_texture[e->animation_frame];
                int sub_texture_w = sub_texture->x1 - sub_texture->x0;
                int sub_texture_h = sub_texture->y1 - sub_texture->y0;
                
                for (int y = 0;
                     y < sub_texture_h;
                     y += 1)
                {
                    for (int x = 0;
                         x < sub_texture_w;
                         x += 1)
                    {
                        int world_x = e->x + x;
                        int world_y = e->y + y;
                        int texel_x = sub_texture->x0 + x;
                        int texel_y = sub_texture->y0 + y;
                        
                        if (FLS_IsCellInBounds(world_x, world_y) && e->texture.buffer[texel_x + texel_y * e->texture.w].a)
                        {
                            FLS_CellAt(falling_sand_state, world_x, world_y) = e->turn_to_sand_kind;
                        }
                    }
                }
            }
            
            if (e->flags & ETT_Flags_isPlayer)
            {
                GME_state.state = GME_State_gameOver;
            }
            
            if (e->flags & ETT_Flags_isMonster)
            {
                GME_state.monster_count -= 1;
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
    static double time = 0.0;
    time += input->dt;
    
    for (ETT_Entity *e = ETT_entities.active_list;
         NULL != e;
         e = e->next)
    {
        int t = 255.0 * (accumulator / (GME_timestep * GME_state.time_dilation));
        int interpolated_x = MTH_InterpolateLinearI(e->prev_x, e->x, t);
        int interpolated_y = MTH_InterpolateLinearI(e->prev_y, e->y, t);
        
        if (e->flags & ETT_Flags_drawBobAnimation)
        {
            interpolated_y += MTH_SinF(time * e->speed) * e->collision_h;
        }
        
        if (e->flags & ETT_Flags_drawSubTexture)
        {
            RDR_DrawSubTexture(input,
                               &e->texture,
                               &e->sub_texture[e->animation_frame],
                               interpolated_x,
                               interpolated_y,
                               e->is_v_flip * RDR_DrawSubTextureFlags_vFlip);
        }
        
        int progress_bar_y = e->y;
        
        if (e->flags & ETT_Flags_drawHealthBar)
        {
            progress_bar_y -= ETT_progressBarHeight * 2;
            
            int w = ((float)e->health / (float)e->max_health) * ETT_progressBarWidth;
            int h = ETT_progressBarHeight;
            int x = e->x + (e->collision_w - ETT_progressBarWidth) / 2;
            int y = progress_bar_y;
            
            RDR_DrawRectangleFill(input,
                                  RectLit(x, y, ETT_progressBarWidth, h),
                                  ColLit(24, 6, 0, 255));
            RDR_DrawRectangleFill(input,
                                  RectLit(x, y, w, h),
                                  ColLit(24, 12, 200, 255));
        }
        
        if (e->flags & ETT_Flags_drawTimerBar)
        {
            progress_bar_y -= ETT_progressBarHeight * 2;
            
            int w = ((float)e->timer / (float)e->timer_max) * ETT_progressBarWidth;
            int h = ETT_progressBarHeight;
            int x = e->x + (e->collision_w - ETT_progressBarWidth) / 2;
            int y = progress_bar_y;
            
            RDR_DrawRectangleFill(input,
                                  RectLit(x, y, ETT_progressBarWidth, h),
                                  ColLit(24, 6, 0, 255));
            RDR_DrawRectangleFill(input,
                                  RectLit(x, y, w, h),
                                  ColLit(200, 24, 12, 255));
        }
    }
}