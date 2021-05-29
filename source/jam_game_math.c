//
// NOTE(tbt): basic math helpers
//


//~NOTE(tbt): floats

static float MTH_pi = 3.141593f;

static float
MTH_SmoothstepF(float t)
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
    return MTH_InterpolateLinearF(a, b, MTH_SmoothstepF(t));
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
MTH_SqrtF(float a)
{
    __m128 _a;
    _a = _mm_load_ss(&a);
    _a = _mm_sqrt_ss(_a);
    _mm_store_ss(&a, _a);
    return a;
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


// NOTE(tbt): not very fast or accurate
static float
MTH_SinF(float a)
{
    // NOTE(tbt): range reduction
    int k = a * (1.0f / (2 * MTH_pi));
    a = a - k * (2 * MTH_pi);
    a = MTH_MinF(a, MTH_pi - a);
    a = MTH_MaxF(a, -MTH_pi - a);
    a = MTH_MinF(a, MTH_pi - a);
    
    float result = 0.0f;
    
    float a1 = a;
    float a2 = a1 * a1;
    float a4 = a2 * a2;
    float a5 = a1 * a4;
    float a9 = a4 * a5;
    float a13 = a9 * a4;
    
    float term_1 = a1  * (1.0f - a2 /   6.0f);
    float term_2 = a5  * (1.0f - a2 /  42.0f) / 120.0f;
    float term_3 = a9  * (1.0f - a2 / 110.0f) / 362880.0f;
    float term_4 = a13 * (1.0f - a2 / 225.0f) / 6227020800.0f;
    
    result += term_4;
    result += term_3;
    result += term_2;
    result += term_1;
    
    return result;
}

//~NOTE(tbt): integers

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
