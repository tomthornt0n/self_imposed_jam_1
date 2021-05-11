// TODO(tbt): actually decent particles

typedef enum
{
 PRT_Shape_square,
 PRT_Shape_circle,
} PRT_Shape;

typedef struct
{
 int x;
 int y;
 int vel_x;
 int vel_y;
 double life;
} PRT_Particle;

static void
PRT_SystemUpdate(PRT_Particle particles[],
                 size_t count,
                 int x,
                 int y,
                 int speed,
                 double life)
{
 while (--count)
 {
  particles[count].life += GME_timestep;
  
  if (particles[count].life > life)
  {
   particles[count].vel_x = 0;
   particles[count].vel_y = 0;
  }
  
  if (particles[count].vel_x == 0 &&
      particles[count].vel_y == 0)
  {
   particles[count].x = x;
   particles[count].y = y;
   particles[count].vel_x = RNG_RandIntNext(-speed, speed + 1);
   particles[count].vel_y = RNG_RandIntNext(-speed, speed + 1);
   particles[count].life = 0.0;
  }
  
  particles[count].x += particles[count].vel_x;
  particles[count].y += particles[count].vel_y;
 }
}

static void
PRT_SystemRender(const PLT_GameInput *input,
                 PRT_Particle particles[],
                 size_t count,
                 PRT_Shape shape,
                 int size,
                 const Colour *colour)
{
 while (--count)
 {
  switch (shape)
  {
   case PRT_Shape_square:
   {
    RDR_DrawRectangleFill(input,
                          RectLit(particles[count].x, particles[count].y, size, size),
                          colour);
   } break;
   
   case PRT_Shape_circle:
   {
   } break;
  }
 }
}