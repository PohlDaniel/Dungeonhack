#ifndef PerlinNoise_H
#define PerlinNoise_H

#include <math.h>
#include <stdio.h>
#include <Ogre.h>

using namespace Ogre;

class newPerlinNoise
{
public:
    static float smoothNoise(float * noise, float x, float y, int noiseWidth, int noiseHeight);
    static float turbulence(float * noise, float x, float y, float size, int noiseWidth, int noiseHeight, float scaling, float mixing);
};

class perlinNoise
{
public:
    perlinNoise();
    int p[512];

    double   fade(double t);
    double   lerp(double t, double a, double b);
    double   grad(int hash, double x, double y, double z); 
    void     init();
    double   pnoise(double x, double y, double z);
};

#endif
