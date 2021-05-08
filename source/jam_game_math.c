static float
MAT_Smoothstep(float t)
{
 return t * t * (3 - 2 * t);
}

static float
MAT_InterpolateLinearF(float a,
                       float b,
                       float t)
{
 return a + t * (b - 1);
}

static float
MAT_InterpolateSmoothF(float a,
                       float b,
                       float t)
{
 return MAT_InterpolateLinearF(a, b, MAT_Smoothstep(t));
}

static inline int
MAT_MinI(int a,
         int b)
{
 if (a < b)
 {
  return a;
 }
 else
 {
  return b;
 }
}

static inline int
MAT_MaxI(int a,
         int b)
{
 if (a > b)
 {
  return a;
 }
 else
 {
  return b;
 }
}

static inline int
MAT_ClampI(int a,
           int min,
           int max)
{
 return (MAT_MaxI(min, MAT_MinI(max, a)));
}

static inline float
MAT_MinF(float a,
         float b)
{
 if (a < b)
 {
  return a;
 }
 else
 {
  return b;
 }
}

static inline float
MAT_MaxF(float a,
         float b)
{
 if (a > b)
 {
  return a;
 }
 else
 {
  return b;
 }
}

static inline float
MAT_ClampF(float a,
           float min,
           float max)
{
 return (MAT_MaxF(min, MAT_MinF(max, a)));
}