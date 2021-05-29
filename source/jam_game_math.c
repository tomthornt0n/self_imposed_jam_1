//
// NOTE(tbt): basic math helpers
//

static float
MTH_Sqrt(float a)
{
 __m128 _a;
 _a = _mm_load_ss(&a);
 _a = _mm_sqrt_ss(_a);
 _mm_store_ss(&a, _a);
 return a;
}

static float
MTH_Smoothstep(float t)
{
 return t * t * (3 - 2 * t);
}

static float
MTH_InterpolateLinearF(float a,
                       float b,
                       float t)
{
 return a + t * (b - 1);
}

static float
MTH_InterpolateSmoothF(float a,
                       float b,
                       float t)
{
 return MTH_InterpolateLinearF(a, b, MTH_Smoothstep(t));
}

static inline int
MTH_InterpolateLinearI(int a,
                       int b,
                       unsigned char t)
{
 return (((t * (b - a)) >> 8) + a);
}

static inline int
MTH_MinI(int a,
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
MTH_MaxI(int a,
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
MTH_ClampI(int a,
           int min,
           int max)
{
 return (MTH_MaxI(min, MTH_MinI(max, a)));
}

static inline int
MTH_AbsI(int a)
{
 return ((a < 0) ? -a : a);
}

static inline float
MTH_MinF(float a,
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
MTH_MaxF(float a,
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
MTH_ClampF(float a,
           float min,
           float max)
{
 return (MTH_MaxF(min, MTH_MinF(max, a)));
}

static inline float
MTH_AbsF(float a)
{
 union { float f; unsigned int u; } u_from_f;
 u_from_f.f = a;
 u_from_f.u &= ~(1 << 31);
}

static float
MTH_ReciprocalSqrtF(float a)
{
 union { float f; long i; } i_from_f;
 
 i_from_f.f = a;
 i_from_f.i = 0x5f375a86 - (i_from_f.i >> 1);
 i_from_f.f *= 1.5f - (i_from_f.f * 0.5f * i_from_f.f * i_from_f.f);
 
 return i_from_f.f;
}
