static int
RNG_HashFromInt(int in)
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
RNG_NextRandInt(int min,
                int max)
{
 static int seed = 0;
 int result = RNG_HashFromInt(seed);
 result %= max - min;
 result += min;
 return result;
}

static float
RNG_Perlin_Internal_Noise2D_I(int x,
                              int y)
{
 int result;
 result = RNG_HashFromInt(y);
 result = RNG_HashFromInt(result + x);
 return result;
}

static float
RNG_Perlin_Internal_Noise2D_F(float x,
                              float y)
{
 int x_int = x;
 int y_int = y;
 float x_frac = x - x_int;
 float y_frac = y - y_int;
 int s = RNG_Perlin_Internal_Noise2D_I(x_int, y_int);
 int t = RNG_Perlin_Internal_Noise2D_I(x_int + 1, y_int);
 int u = RNG_Perlin_Internal_Noise2D_I(x_int, y_int + 1);
 int v = RNG_Perlin_Internal_Noise2D_I(x_int + 1, y_int + 1);
 float low = MA_InterpolateSmoothF(s, t, x_frac);
 float high = MA_InterpolateSmoothF(u, v, x_frac);
 return MA_InterpolateSmoothF(low, high, y_frac);
}

static float
RNG_Perlin_2D(float x,
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
  fin += RNG_Perlin_Internal_Noise2D_F(xa, ya) * amp;
  amp /= 2.0f;
  xa *= 2;
  ya *= 2;
 }
 
 return fin / div;
}