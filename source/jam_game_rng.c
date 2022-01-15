//
// NOTE(tbt): simple RNG helpers
//

static unsigned int
RNG_HashFromInt(unsigned int in)
{
 in ^= 2747636419;
 in *= 2654435769;
 in ^= in >> 16;
 in *= 2654435769;
 in ^= in >> 16;
 in *= 2654435769;
 return in;
}

static int
RNG_RandIntNext(int min,
                int max)
{
 static unsigned int seed = 0;
 seed += 1;
 
 int result;
 
 if (max == min)
 {
  result = min;
 }
 else
 {
  result = MTH_AbsI(RNG_HashFromInt(seed));
  result %= max - min;
  result += min;
 }
 
 return result;
}

static int
RNG_Noise2DI_INTERNAL(int x,
                      int y)
{
 int result;
 result = RNG_HashFromInt(y);
 result = RNG_HashFromInt(result + x);
 return result;
}

static int
RNG_RandInt2D(int x,
              int y,
              int min,
              int max)
{
 return ((RNG_Noise2DI_INTERNAL(x, y) % (max - min)) + min);
}

static float
RNG_Noise2DF_INTERNAL(float x,
                      float y)
{
 int x_int = x;
 int y_int = y;
 float x_frac = x - x_int;
 float y_frac = y - y_int;
 int s = RNG_Noise2DI_INTERNAL(x_int, y_int);
 int t = RNG_Noise2DI_INTERNAL(x_int + 1, y_int);
 int u = RNG_Noise2DI_INTERNAL(x_int, y_int + 1);
 int v = RNG_Noise2DI_INTERNAL(x_int + 1, y_int + 1);
 float low = MTH_InterpolateSmoothF(s, t, x_frac);
 float high = MTH_InterpolateSmoothF(u, v, x_frac);
 return MTH_InterpolateSmoothF(low, high, y_frac);
}

static float
RNG_Perlin2D(float x,
             float y,
             float freq,
             int depth)
{
 float xa = x * freq;
 float ya = y * freq;
 float amp = 1.0f;
 float fin = 0.0f;
 float div = 0.0f;
 
 for (int i = 0;
      i < depth;
      i += 1)
 {
  div += 256 * amp;
  fin += RNG_Noise2DF_INTERNAL(xa, ya) * amp;
  amp /= 2.0f;
  xa *= 2;
  ya *= 2;
 }
 
 return fin / div;
}
