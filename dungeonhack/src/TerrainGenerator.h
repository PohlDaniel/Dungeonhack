#ifndef TerrainGenerator_H
#define TerrainGenerator_H

#include <Ogre.h>

using namespace Ogre;

class TerrainGenerator
{
public:
    static String createTerrainTile(float * mapArray, int zoomLevel,int zoneX, int zoneY, int size, int tileSize, Ogre::SceneManager * mSceneMgr);
    //static tileData createTerrainTile(int zoomLevel,int zoneX, int zoneY, int size, int tileSize, Ogre::SceneManager * mSceneMgr);
    static void createTileMesh(float * mapArray, int width, int height, String ID,int numLODs,float scale, float heightScale);
    static void GeneratePerlinZonedTerrain(float * mapArray, int width, int height, int zoneX, int zoneY, float scale, float min, float max);
    static void GeneratePerlinTerrain(float * mapArray, int width, int height);
    static void GenerateTerrain(float * mapArray, float x, float y, float width, float height, float c1, float c2, float c3, float c4, int terrainWidth, int terrainHeight, int rowPitch, int seedValue, int recursiveLevel = 1);

    static void checkLocation(float * mapArray, int zoneX, int zoneY, int size, int tileSize);
};

#endif
