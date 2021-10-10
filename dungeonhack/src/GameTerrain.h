#ifndef GameTerrain_H
#define GameTerrain_H

#include <OgreSceneManager.h>
#include <PagedGeometry.h>
#include <TreeLoader3D.h>

using namespace Ogre;

struct TextureGenerationOptions
{
    int min_value;
    int max_value;
    float max_height;
    float min_height;
    float max_slope;
    float min_slope;

    TextureGenerationOptions()
    {
        min_value   = 0;
        max_value   = 255;
        max_height  = 100000;
        min_height  = -100000;
        max_slope   = 90;
        min_slope   = 0;
    }
};

class GameTerrain;
class TerrainDefType;
class WaterPlane;
class Location;
class btBvhTriangleMeshShape;
class btRigidBody;
class PhysicsBody;

class TerrainChunk
{
public:
    TerrainChunk(int zone_x, int zone_y, float zoom, float scale, Ogre::SceneManager * mSceneMgr, GameTerrain * parent);
    ~TerrainChunk();

    GameTerrain * m_parent_terrain;

    Ogre::SceneNode * m_SceneNode;
    Ogre::SceneNode * m_ImposterNode;
    Ogre::Entity * m_TerrainEntity;

    MaterialPtr terrainMaterial;
    TexturePtr terrainSplatTextures[4];

    float TerrainHeightAt(int heightmap_x, int heightmap_y);
    float ApproximatedHeightAt(int pos_x, int pos_y);
    float ApproximatedHeightAtPercentage(float pos_x, float pos_y);

    TexturePtr GenerateChunkTexture(int pass, int width, int height, TextureGenerationOptions options);
    //TexturePtr GeneratePerlinZonedTerrainTexture(int pass, int width, int height, float min, float max, float min_height = -100000, float max_height = 100000);
    float * GeneratePerlinNoise(int width, int height);
    float * GenerateTextureFromTerrain(int width, int height, TextureGenerationOptions options);
    MaterialPtr GetChunkMaterial();

    //Terrain splatting and foliage generation options
    TexturePtr m_splat1;
    TexturePtr m_splat2;
    TexturePtr m_splat3;

    bool m_splatBaseCreatesGrass;
    bool m_splat1CreatesGrass;
    bool m_splat2CreatesGrass;
    bool m_splat3CreatesGrass;

    bool m_splatBaseCreatesVegetation;
    bool m_splat1CreatesVegetation;
    bool m_splat2CreatesVegetation;
    bool m_splat3CreatesVegetation;

    Vector3 m_terrainPos;

    String m_thisTileString;

    bool m_generatedGrassLastPass;

    void updateGrass(Vector3 position);
    void GenerateVegetation(int startX, int startY, int endX, int endY, String terrainType);
    void GenerateGrass(int startX, int startY, int endX, int endY, String terrainType);

    void ClearGrass();

    void removeGrass();
    void removeTrees();

    void waveGrass(Real timeElapsed, float grass_wave_sin);
    void waveTrees(Real timeElapsed);

    void doIsNowCenterTile();   //Called whenever the player moves onto this tile

    Ogre::BillboardSet* getImposterBillboardSet(std::string imposterMaterial);

    void shadowGround(int x, int y, int radius);

    TerrainDefType* m_terrainDef;

    Location * m_Location;

    MaterialPtr m_terrainMaterial;
    Entity * m_heightmapMesh;

private:
    /// Places a tree or other landscape piece on the terrain
    void placeTree(Vector3 position, std::string meshName, float baseScale);

    int m_zoneX;
    int m_zoneY;
    float m_zoom;
    float m_scale;
    float tileSeperation;

    int m_width;
    int m_height;

    float * m_mapArray;
    bool * m_GenFoliageArray;
    int * m_FoliageShadowArray;

    Ogre::StaticGeometry * grassBatch;
    Ogre::StaticGeometry * mediumDistanceBatch;
    Ogre::StaticGeometry * farDistanceBatch;

    String m_GrassBatchName;
    String m_SwapGrassBatchName;

    //Grass is generated in a static geometry batch over a series of frames, and then swapped with the current grass batch
    Ogre::StaticGeometry * m_swapGrassBatch;

    bool m_doUpdateGrass;
    bool m_doSwapGrass; //Set to true when done generating grass, the batch will then be swapped out next update.
    int m_numFramesToUpdateGrass;
    int m_curGrassUpdateFrame;
    int m_numGrassPerFrame;
    int m_totalGrassToGenerate;

    int m_startGenGrassX;
    int m_startGenGrassY;
    int m_endGenGrassX;
    int m_endGenGrassY;

    bool * haveGeneratedGrassTile;

    Ogre::SceneManager * m_SceneMgr;

    // Physics
    /// Terrain mesh shape
    btBvhTriangleMeshShape* m_shape;
    /// Terrain collision body
    btRigidBody* m_body;
    std::vector<PhysicsBody*> m_terrainObjectBodies;
};

class TerrainTile
{
public:
    TerrainTile(int zone_x, int zone_y, float zoom, float scale, Ogre::SceneManager * mSceneMgr, GameTerrain * parent);
    ~TerrainTile();
    TerrainChunk * m_Chunk;

private:
    int m_zoneX;
    int m_zoneY;
    float m_zoom;
    float m_scale;

    Ogre::SceneManager * m_SceneMgr;
};

class GameTerrain : public Ogre::Singleton<GameTerrain>
{
public:
    GameTerrain(int zone_x, int zone_y, int tileSize, Ogre::SceneManager * mSceneMgr, WaterPlane * water_plane = 0);
    ~GameTerrain();

    static GameTerrain& getSingleton(void);
    static GameTerrain* getSingletonPtr(void);

    TerrainTile * m_NorthTile;
    TerrainTile * m_EastTile;
    TerrainTile * m_WestTile;
    TerrainTile * m_SouthTile;
    TerrainTile * m_CenterTile;

    TerrainTile * m_NorthWestTile;
    TerrainTile * m_NorthEastTile;
    TerrainTile * m_SouthWestTile;
    TerrainTile * m_SouthEastTile;

    Ogre::SceneManager * m_SceneMgr;
    WaterPlane * m_water;

    int currentZoneX;
    int currentZoneY;
    int m_tileSize;

    //Used for updating vegetation around the player when necessary
    float lastGrassX;
    float lastGrassY;
    int curGrassX;
    int curGrassY;

    float m_grass_wave_sin;

    Vector3 m_lastGrassUpdatePlayerPos;

    void Update(float X, float Y, Real timeElapsed);
    void Move(int dir);

    void MoveEast();
    void MoveWest();
    void MoveNorth();
    void MoveSouth();

    void unloadTerrain();
    void loadTerrain(int zone_x, int zone_y, int tileSize, Ogre::SceneManager * mSceneMgr);
    void updateGrass(Vector3 position);

    void exitInterior();
    void enterInterior();

    bool isIndoor;
    bool m_doGrass; /// Whether or not to generate and render grass
    bool m_doWindWave; /// Whether or not to wave foliage

    Forests::TreeLoader3D* m_treeLoader;
    std::map<std::string, Entity*> m_precachedFoliageList;

private:
    // Paged Geometry
    Forests::PagedGeometry* m_trees;
};

#endif
