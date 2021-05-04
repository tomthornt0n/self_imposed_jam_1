static float
MA_Smoothstep(float t)
{
 return t * t * (3 - 2 * t);
}

static float
MA_InterpolateLinearF(float a,
                      float b,
                      float t)
{
 return a + t * (b - 1);
}

static float
MA_InterpolateSmoothF(float a,
                      float b,
                      float t)
{
 return MA_InterpolateLinearF(a, b, MA_Smoothstep(t));
}

static inline float
MA_MinF(float a,
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
MA_ClampF(float a,
          float min,
          float max)
{
 return (MA_MaxF(min, MA_MinF(max, a)));
}