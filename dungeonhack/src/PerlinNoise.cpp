#include "DungeonHack.h"
#include "PerlinNoise.h"

float newPerlinNoise::smoothNoise(float * noise, float x, float y, int noiseWidth, int noiseHeight)
{  
   //get fractional part of x and y
   float fractX = x - int(x);
   float fractY = y - int(y);
   
   //wrap around
   int x1 = (int(x) + noiseWidth) % noiseWidth;
   int y1 = (int(y) + noiseHeight) % noiseHeight;
   
   //neighbor values
   int x2 = (x1 + noiseWidth - 1) % noiseWidth;
   int y2 = (y1 + noiseHeight - 1) % noiseHeight;

   /*int x1 = (int(x)+1);
   int y1 = (int(y)+1);
   
   //neighbor values
   int x2 = (x1-1);
   int y2 = (y1-1);

   if(x1 > noiseWidth)
       x1 = noiseWidth;
   if(y1 > noiseHeight)
       y1 = noiseHeight;

   if(x2 < 0)
       x2 = 0;

   if(y2 < 0)
       y2 = 0;*/

   //smooth the noise with bilinear interpolation
   float value = 0.0;
   value += fractX       * fractY       * noise[y1 * (noiseHeight) + x1]; //noise[x1][y1];
   value += fractX       * (1 - fractY) * noise[y2 * (noiseHeight) + x1]; //noise[x1][y2];
   value += (1 - fractX) * fractY       * noise[y1 * (noiseHeight) + x2]; //noise[x2][y1];
   value += (1 - fractX) * (1 - fractY) * noise[y2 * (noiseHeight) + x2]; //noise[x2][y2];

   return value;
}

/*float newPerlinNoise::smoothNoise(float * noise, float x, float y, int noiseWidth, int noiseHeight)
{  
   //get fractional part of x and y
   float fractX = x - int(x);
   float fractY = y - int(y);
   
   //wrap around
   int x1 = (int(x) + noiseWidth) % noiseWidth;
   int y1 = (int(y) + noiseHeight) % noiseHeight;
   
   //neighbor values
   int x2 = (x1 + noiseWidth - 1) % noiseWidth;
   int y2 = (y1 + noiseHeight - 1) % noiseHeight;

   //smooth the noise with bilinear interpolation
   float value = 0.0;
   value += fractX       * fractY       * noise[y1 * (noiseHeight) + x1];   //;noise[x1][y1];
   value += fractX       * (1 - fractY) * noise[y2 * (noiseHeight) + x1];   //;noise[x1][y2];
   value += (1 - fractX) * fractY       * noise[y1 * (noiseHeight) + x2];   //;noise[x2][y1];
   value += (1 - fractX) * (1 - fractY) * noise[y2 * (noiseHeight) + x2];   //;noise[x2][y2];

   return value;
}*/

float newPerlinNoise::turbulence(float * noise, float x, float y, float size, int noiseWidth, int noiseHeight,float scaling, float mixing)
{
    float value = 0.0, initialSize = size;
    perlinNoise newNoise;
    
    bool isDone = false;

    while(size >= 1 && !isDone)
    {
        float newVal;
        //newVal = smoothNoise(noise, x / size, y / size, noiseWidth, noiseHeight);
        //float newVal = newNoise.pnoise(x*(size * 2),y*(size*2),3);

        newVal = newNoise.pnoise(x/size,y/size,3);
        //newVal = Math::Cos((x/size) * newNoise.pnoise(x/size,y/size,3));
        newVal /= 2.0;
        newVal += 0.5;

        if(size < 1)
            isDone = true;

        value += newVal * (size * mixing);
        size /= scaling;
    }
   
    return(128.0 * (value / (initialSize)));
    //return(128 * (value));
}

static int permutation[] = { 151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,
    21,10,23,190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
    35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168, 68,175,
    74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,
    230,220,105,92,41,55,46,245,40,244,102,143,54, 65,25,63,161, 1,216,
    80,73,209,76,132,187,208, 89,18,169,200,196,135,130,116,188,159,86,
    164,100,109,198,173,186, 3,64,52,217,226,250,124,123,5,202,38,147,
    118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,
    183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,
    172,9,129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,
    218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241, 81,51,
    145,235,249,14,239,107,49,192,214,31,181,199,106,157,184, 84,204,176,
    115,121,50,45,127, 4,150,254,138,236,205,93,222,114,67,29,24,72,243,
    141,128,195,78,66,215,61,156,180
    };

perlinNoise::perlinNoise()
{
    init();
}

void perlinNoise::init()
{
    int i;
    for(i = 0; i < 256 ; i++) 
      p[256+i] = p[i] = permutation[i]; 
}

double perlinNoise::pnoise(double x, double y, double z) 
{
    const int   X = (int)floor(x) & 255,             // FIND UNIT CUBE THAT 
        Y = (int)floor(y) & 255,             // CONTAINS POINT.     
        Z = (int)floor(z) & 255;

    x -= floor(x);                             // FIND RELATIVE X,Y,Z 
    y -= floor(y);                             // OF POINT IN CUBE.   
    z -= floor(z);
    double  u = fade(x),                       // COMPUTE FADE CURVES 
            v = fade(y),                       // FOR EACH OF X,Y,Z.  
            w = fade(z);
    int  A = p[X]+Y, 
        AA = p[A]+Z, 
        AB = p[A+1]+Z, // HASH COORDINATES OF 
        B = p[X+1]+Y, 
        BA = p[B]+Z, 
        BB = p[B+1]+Z; // THE 8 CUBE CORNERS, 

    return lerp(w,lerp(v,lerp(u, grad(p[AA  ], x, y, z),   // AND ADD 
                        grad(p[BA  ], x-1, y, z)),        // BLENDED 
                lerp(u, grad(p[AB  ], x, y-1, z),         // RESULTS 
                        grad(p[BB  ], x-1, y-1, z))),     // FROM  8
                lerp(v, lerp(u, grad(p[AA+1], x, y, z-1 ), // CORNERS 
                        grad(p[BA+1], x-1, y, z-1)),      //OF CUBE
                lerp(u, grad(p[AB+1], x, y-1, z-1),
                      grad(p[BB+1], x-1, y-1, z-1))));
}

double perlinNoise::fade(double t){ return t * t * t * (t * (t * 6 - 15) + 10); }
double perlinNoise::lerp(double t, double a, double b){ return a + t * (b - a); }
double perlinNoise::grad(int hash, double x, double y, double z) 
{
    int     h = hash & 15;       // CONVERT LO 4 BITS OF HASH CODE
    double  u = h < 8 ? x : y,   // INTO 12 GRADIENT DIRECTIONS. 
            v = h < 4 ? y : h==12||h==14 ? x : z;
    return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}
