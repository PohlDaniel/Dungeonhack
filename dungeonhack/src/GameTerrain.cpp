#include "DungeonHack.h"
#include "GameTerrain.h"

#include <BatchPage.h>
#include <ImpostorPage.h>

#include "TerrainType.h"
#include "TerrainGenerator.h"
#include "GameWorld.h"
#include "PhysicsManager.h"
#include "Raycast.h"
#include "GameManager.h"
#include "GameState.h"
#include "EntityManager.h"
#include "GameFoliage.h"
#include "WaterPlane.h"
#include <OgreHardwarePixelBuffer.h>
#include "PerlinNoise.h"
#include "GameConfig.h"
#include "SoundManager.h"
#include "PlayState.h"

#include "BtOgreGP.h"
#include "BtOgrePG.h"

template<> GameTerrain* Singleton<GameTerrain>::ms_Singleton = 0;

GameTerrain* GameTerrain::getSingletonPtr(void)
{
    return ms_Singleton;
}

GameTerrain& GameTerrain::getSingleton(void)
{  
    assert(ms_Singleton);
    return *ms_Singleton;
}

GameTerrain::GameTerrain(int zone_x, int zone_y, int tileSize, Ogre::SceneManager * mSceneMgr, WaterPlane * water_plane)
{
    isIndoor = false;
    this->m_water = water_plane;

    this->m_EastTile = 0;
    this->m_WestTile = 0;
    this->m_SouthTile = 0;
    this->m_NorthTile = 0;
    this->m_CenterTile = 0;

    this->m_NorthWestTile = 0;
    this->m_NorthEastTile = 0;
    this->m_SouthWestTile = 0;
    this->m_SouthEastTile = 0;

    this->currentZoneX = zone_x;
    this->currentZoneY = zone_y;

    this->m_tileSize = tileSize;

    m_SceneMgr = mSceneMgr;

    float zoom = 3;
    m_grass_wave_sin = 0;

    GameConfiguration* config = GameManager::getSingletonPtr()->getConfig();
    if(config->getInteger("grassWave") == 1)
        m_doWindWave = true;
    else
        m_doWindWave = false;

    if(config->getInteger("doGrass") == 1)
        m_doGrass = true;
    else
        m_doGrass = false;

    // Get foliage config values
    int pageSize = config->getInteger("treePageSize");
    int modelDist = config->getInteger("treeModelDistance");
    int imposterDist = config->getInteger("treeImposterDistance");
    int modelFade = config->getInteger("treeModelFadeDistance") - modelDist;
    int imposterFade = config->getInteger("treeImposterFadeDistance") - imposterDist;

    // Initialize Paged Geometry before tiles
    m_trees = new Forests::PagedGeometry(
            GameManager::getSingletonPtr()->getCurrentState()->getCamera(), pageSize);
    m_trees->addDetailLevel<Forests::BatchPage>(modelDist, modelFade);
    m_trees->addDetailLevel<Forests::ImpostorPage>(imposterDist, imposterFade);
    m_treeLoader = new Forests::TreeLoader3D(m_trees, Forests::TBounds(400000, 400000, 600000, 600000));
    m_treeLoader->setMaximumScale(100.0);
    m_trees->setPageLoader(m_treeLoader);

    // Preload all the foliage that will be used
    Ogre::SceneManager* sceneMgr = GameManager::getSingletonPtr()->getSceneManager();
    std::string terrainType = GameWorld::getSingletonPtr()->m_gameDataPath
        + config->getString("testTerrainDefinition");
    TerrainDefType terrainDef(terrainType);

    std::vector<TerrainFoliageDef*>::iterator it;
    // Main foliage
    for(it = terrainDef.m_foliageDefList.begin(); it != terrainDef.m_foliageDefList.end(); ++it)
    {
        TerrainFoliageDef* thisDef = *it;
        // Make sure this isn't already in the list
        if(m_precachedFoliageList.find(thisDef->m_Mesh) == m_precachedFoliageList.end())
            m_precachedFoliageList[thisDef->m_Mesh] = sceneMgr->createEntity(
                thisDef->m_Mesh, thisDef->m_Mesh);
        else
            continue; // Skip to the next main entry

        // Grow around foliage
        std::vector<TerrainFoliageDef*>::iterator it2;
        for(it2 = thisDef->m_foliageGrowAroundList.begin();
                it2 != thisDef->m_foliageGrowAroundList.end(); ++it2)
        {
            TerrainFoliageDef* thisDef2 = *it2;
            if(m_precachedFoliageList.find(thisDef2->m_Mesh) == m_precachedFoliageList.end())
                m_precachedFoliageList[thisDef2->m_Mesh] = sceneMgr->createEntity(
                    thisDef2->m_Mesh, thisDef2->m_Mesh);
        }
    }

    this->m_CenterTile = new TerrainTile(currentZoneX, currentZoneY,zoom,tileSize,mSceneMgr,this);

    this->m_EastTile = new TerrainTile(currentZoneX + 1, currentZoneY,zoom,tileSize,mSceneMgr,this);
    this->m_WestTile = new TerrainTile(currentZoneX - 1, currentZoneY,zoom,tileSize,mSceneMgr,this);

    this->m_SouthWestTile = new TerrainTile(currentZoneX - 1, currentZoneY - 1,zoom,tileSize,mSceneMgr,this);
    this->m_SouthEastTile = new TerrainTile(currentZoneX + 1, currentZoneY - 1,zoom,tileSize,mSceneMgr,this);

    this->m_NorthTile = new TerrainTile(currentZoneX, currentZoneY + 1,zoom,tileSize,mSceneMgr,this);
    this->m_SouthTile = new TerrainTile(currentZoneX, currentZoneY - 1,zoom,tileSize,mSceneMgr,this);

    this->m_NorthEastTile = new TerrainTile(currentZoneX + 1, currentZoneY + 1,zoom,tileSize,mSceneMgr,this);
    this->m_NorthWestTile = new TerrainTile(currentZoneX - 1, currentZoneY + 1,zoom,tileSize,mSceneMgr,this);

    m_water = 0;
}

GameTerrain::~GameTerrain()
{
    unloadTerrain();

    Ogre::SceneManager* mSceneMgr = GameManager::getSingletonPtr()->getSceneManager();
    std::map<std::string, Entity*>::iterator it;
    // Main foliage
    for(it = m_precachedFoliageList.begin(); it != m_precachedFoliageList.end(); ++it)
        mSceneMgr->destroyEntity(it->second);

    delete m_treeLoader;
    delete m_trees;
}

void GameTerrain::unloadTerrain()
{
    if(m_CenterTile != 0)
    {
        delete m_CenterTile;
        m_CenterTile = 0;
    }

    if(m_EastTile != 0)
    {
        delete m_EastTile;
        m_EastTile = 0;
    }

    if(m_NorthEastTile != 0)
    {
        delete m_NorthEastTile;
        m_NorthEastTile = 0;
    }

    if(m_WestTile != 0)
    {
        delete m_WestTile;
        m_WestTile = 0;
    }

    if(m_NorthWestTile != 0)
    {
        delete m_NorthWestTile;
        m_NorthWestTile = 0;
    }

    if(m_NorthTile != 0)
    {
        delete m_NorthTile;
        m_NorthTile = 0;
    }

    if(m_SouthTile != 0)
    {
        delete m_SouthTile;
        m_SouthTile = 0;
    }

    if(m_SouthEastTile != 0)
    {
        delete m_SouthEastTile;
        m_SouthEastTile = 0;
    }

    if(m_SouthWestTile != 0)
    {
        delete m_SouthWestTile;
        m_SouthWestTile = 0;
    }
}

void GameTerrain::loadTerrain(int zone_x, int zone_y, int tileSize, Ogre::SceneManager * mSceneMgr)
{
    float zoom = 3;

    if(m_CenterTile == 0)
        this->m_CenterTile = new TerrainTile(currentZoneX, currentZoneY,zoom,tileSize,mSceneMgr,this);

    if(m_EastTile == 0)
        this->m_EastTile = new TerrainTile(currentZoneX + 1, currentZoneY,zoom,tileSize,mSceneMgr,this);

    if(m_WestTile == 0)
        this->m_WestTile = new TerrainTile(currentZoneX - 1, currentZoneY,zoom,tileSize,mSceneMgr,this);

    if(m_SouthWestTile == 0)
        this->m_SouthWestTile = new TerrainTile(currentZoneX - 1, currentZoneY - 1,zoom,tileSize,mSceneMgr,this);

    if(m_SouthEastTile == 0)
        this->m_SouthEastTile = new TerrainTile(currentZoneX + 1, currentZoneY - 1,zoom,tileSize,mSceneMgr,this);

    if(m_NorthTile == 0)
        this->m_NorthTile = new TerrainTile(currentZoneX, currentZoneY + 1,zoom,tileSize,mSceneMgr,this);

    if(m_SouthTile == 0)
        this->m_SouthTile = new TerrainTile(currentZoneX, currentZoneY - 1,zoom,tileSize,mSceneMgr,this);

    if(m_NorthEastTile == 0)
        this->m_NorthEastTile = new TerrainTile(currentZoneX + 1, currentZoneY + 1,zoom,tileSize,mSceneMgr,this);

    if(m_NorthWestTile == 0)
        this->m_NorthWestTile = new TerrainTile(currentZoneX - 1, currentZoneY + 1,zoom,tileSize,mSceneMgr,this);
}

void GameTerrain::updateGrass(Vector3 position)
{
    this->lastGrassX = position.x;
    this->lastGrassY = position.z;

    m_CenterTile->m_Chunk->updateGrass(position);
}

void GameTerrain::Update(float X, float Y, Real timeElapsed)
{
    m_trees->update(); // Update paged geometry

    Vector3 PlayerPos = Vector3::Vector3(X,0,Y);
    if(!isIndoor)
    {
        m_grass_wave_sin += timeElapsed * 1.0;

        if(m_doWindWave)
        {
            m_CenterTile->m_Chunk->waveGrass(timeElapsed, m_grass_wave_sin);
            //m_CenterTile->m_Chunk->waveTrees(timeElapsed);
        }

        float terrainSize = 64;

        float curUpdateX = currentZoneX * (terrainSize * m_tileSize);
        float curUpdateY = currentZoneY * (terrainSize * m_tileSize);

        float centerOffset = (terrainSize * m_tileSize);

        bool didMove = true;
        if(((X - curUpdateX) ) > (terrainSize * m_tileSize))
        {
            MoveEast();
        }
        else if(((X - curUpdateX) - (centerOffset)) < (-terrainSize * m_tileSize))
        {
            MoveWest();
        }
        else if(((Y - curUpdateY)) > (terrainSize * m_tileSize))
        {
            MoveNorth();
        }
        else if(((Y - curUpdateY) - (centerOffset)) < (-terrainSize * m_tileSize))
        {
            MoveSouth();
        }
        else
        {
            //We didn't move anywhere
            didMove = false;
        }

        if(didMove == true)
        {
            this->m_CenterTile->m_Chunk->doIsNowCenterTile();

            if(m_water != 0)
                m_water->Update(PlayerPos);
        }

        curGrassX = ((((this->currentZoneX * 64)-32) * 512) - (int)(PlayerPos.x)) / 1024;
        curGrassY = ((((this->currentZoneY * 64)-32) * 512) - (int)(PlayerPos.z)) / 1024;

        Ogre::Vector3 distanceSinceLastUpdate = m_lastGrassUpdatePlayerPos - PlayerPos;
        float dist = distanceSinceLastUpdate.normalise();

        if(dist >= 450)
        {
            if(lastGrassX != curGrassX || lastGrassY != curGrassY)
            {
                updateGrass(PlayerPos);
                lastGrassX = curGrassX;
                lastGrassY = curGrassY;

                m_lastGrassUpdatePlayerPos = PlayerPos;
            }
        }
    }
}

void GameTerrain::MoveNorth()
{
    if(m_SouthTile != 0)
    {
        delete this->m_SouthTile;
    }
    if(m_SouthWestTile != 0)
    {
        delete m_SouthWestTile;
    }
    if(m_SouthEastTile != 0)
    {
        delete m_SouthEastTile;
    }

    TerrainTile * swapTile;

    swapTile = this->m_CenterTile;
    this->m_SouthTile = swapTile;

    swapTile = this->m_WestTile;
    this->m_SouthWestTile = swapTile;

    swapTile = this->m_EastTile;
    this->m_SouthEastTile = swapTile;

    this->m_CenterTile = this->m_NorthTile;
    this->m_EastTile = this->m_NorthEastTile;
    this->m_WestTile = this->m_NorthWestTile;

    this->m_NorthTile = new TerrainTile(currentZoneX, currentZoneY + 2,3,m_tileSize,m_SceneMgr,this);
    this->m_NorthWestTile = new TerrainTile(currentZoneX-1, currentZoneY+2,3,m_tileSize,m_SceneMgr,this);
    this->m_NorthEastTile = new TerrainTile(currentZoneX+1, currentZoneY+2,3,m_tileSize,m_SceneMgr,this);

    currentZoneY++;
}

void GameTerrain::MoveSouth()
{
    if(m_NorthTile != 0)
    {
        delete this->m_NorthTile;
    }
    if(m_NorthWestTile != 0)
    {
        delete m_NorthWestTile;
    }
    if(m_NorthEastTile != 0)
    {
        delete m_NorthEastTile;
    }

    TerrainTile * swapTile;

    swapTile = this->m_CenterTile;
    this->m_NorthTile = swapTile;

    swapTile = this->m_WestTile;
    this->m_NorthWestTile = swapTile;

    swapTile = this->m_EastTile;
    this->m_NorthEastTile = swapTile;

    this->m_CenterTile = this->m_SouthTile;
    this->m_EastTile = this->m_SouthEastTile;
    this->m_WestTile = this->m_SouthWestTile;

    this->m_SouthTile = new TerrainTile(currentZoneX, currentZoneY - 2,3,m_tileSize,m_SceneMgr,this);
    this->m_SouthWestTile = new TerrainTile(currentZoneX-1, currentZoneY-2,3,m_tileSize,m_SceneMgr,this);
    this->m_SouthEastTile = new TerrainTile(currentZoneX+1, currentZoneY-2,3,m_tileSize,m_SceneMgr,this);

    currentZoneY--;
}

void GameTerrain::MoveEast()
{
    //currentZoneX++;

    if(m_WestTile != 0)
    {
        delete this->m_WestTile;
    }
    if(m_NorthWestTile != 0)
    {
        delete m_NorthWestTile;
    }
    if(m_SouthWestTile != 0)
    {
        delete m_SouthWestTile;
    }

    TerrainTile * swapTile;

    swapTile = this->m_CenterTile;
    this->m_WestTile = swapTile;

    swapTile = this->m_NorthTile;
    this->m_NorthWestTile = swapTile;

    swapTile = this->m_SouthTile;
    this->m_SouthWestTile = swapTile;

    this->m_CenterTile = this->m_EastTile;
    this->m_NorthTile = this->m_NorthEastTile;
    this->m_SouthTile = this->m_SouthEastTile;

    this->m_EastTile = new TerrainTile(currentZoneX + 2, currentZoneY,3,m_tileSize,m_SceneMgr,this);
    this->m_NorthEastTile = new TerrainTile(currentZoneX + 2, currentZoneY+1,3,m_tileSize,m_SceneMgr,this);
    this->m_SouthEastTile = new TerrainTile(currentZoneX + 2, currentZoneY-1,3,m_tileSize,m_SceneMgr,this);

    currentZoneX++;
}

void GameTerrain::MoveWest()
{
    if(m_EastTile != 0)
    {
        delete this->m_EastTile;
    }
    if(m_NorthEastTile != 0)
    {
        delete m_NorthEastTile;
    }
    if(m_SouthEastTile != 0)
    {
        delete m_SouthEastTile;
    }

    TerrainTile * swapTile;

    swapTile = this->m_CenterTile;
    this->m_EastTile = swapTile;

    swapTile = this->m_NorthTile;
    this->m_NorthEastTile = swapTile;

    swapTile = this->m_SouthTile;
    this->m_SouthEastTile = swapTile;

    this->m_CenterTile = this->m_WestTile;
    this->m_NorthTile = this->m_NorthWestTile;
    this->m_SouthTile = this->m_SouthWestTile;

    this->m_WestTile = new TerrainTile(currentZoneX - 2, currentZoneY,3,m_tileSize,m_SceneMgr,this);
    this->m_NorthWestTile = new TerrainTile(currentZoneX - 2, currentZoneY+1,3,m_tileSize,m_SceneMgr,this);
    this->m_SouthWestTile = new TerrainTile(currentZoneX - 2, currentZoneY-1,3,m_tileSize,m_SceneMgr,this);

    currentZoneX--;
}

void GameTerrain::Move(int dir) //0 = N, 1 = S, 2 = W, 3 = E
{
    if(dir == 3)
    {
        currentZoneX++;

        if(m_WestTile != 0)
        {
            delete this->m_WestTile;
        }
        if(m_NorthWestTile != 0)
        {
            delete m_NorthWestTile;
        }
        if(m_SouthWestTile != 0)
        {
            delete m_SouthWestTile;
        }

        TerrainTile * swapTile;

        swapTile = this->m_CenterTile;
        this->m_WestTile = swapTile;

        swapTile = this->m_NorthTile;
        this->m_NorthWestTile = swapTile;

        swapTile = this->m_SouthTile;
        this->m_SouthWestTile = swapTile;

        this->m_CenterTile = this->m_EastTile;
        this->m_NorthTile = this->m_NorthEastTile;
        this->m_SouthTile = this->m_SouthEastTile;

        this->m_EastTile = new TerrainTile(currentZoneX, currentZoneY,3,m_tileSize,m_SceneMgr,this);
        this->m_NorthEastTile = new TerrainTile(currentZoneX, currentZoneY+1,3,m_tileSize,m_SceneMgr,this);
        this->m_SouthEastTile = new TerrainTile(currentZoneX, currentZoneY-1,3,m_tileSize,m_SceneMgr,this);
    }
    else if(dir == 2)
    {
        if(currentZoneX >= 2)
        {
            currentZoneX--;
        }
    }
    else if(dir == 1)
    {
        currentZoneY++;
    }
    else if(dir == 0)
    {
        if(currentZoneY >= 2)
        {
            currentZoneY--;
        }
    }
}

void GameTerrain::exitInterior()
{
    int centerX = this->currentZoneX;
    int centerY = this->currentZoneY;

    for(int x = centerX - 1; x <= centerX + 1; x++)
    {
        for(int y = centerY - 1; y <= centerY + 1; y++)
        {
            //Check if there is a location for this tile, and load it if we find one
            char zx[128];
            char zy[128];

            sprintf(zx, "%d", x);   //%G for float, %d for int
            sprintf(zy, "%d", y);

            const String zoneXS = zx;
            const String zoneYS = zy;
            String myZoneString = zoneXS + "," + zoneYS;

            Location * thisLoc = 0;
            thisLoc = GameWorld::getSingletonPtr()->gameLocations[myZoneString];

            if(thisLoc != 0)
            {
                thisLoc->LoadExterior();
            }
        }
    }
}

void GameTerrain::enterInterior()
{
    int centerX = this->currentZoneX;
    int centerY = this->currentZoneY;

    for(int x = centerX - 1; x <= centerX + 1; x++)
    {
        for(int y = centerY - 1; y <= centerY + 1; y++)
        {
            //Check if there is a location for this tile, and load it if we find one
            char zx[128];
            char zy[128];

            sprintf(zx, "%d", x);   //%G for float, %d for int
            sprintf(zy, "%d", y);

            const String zoneXS = zx;
            const String zoneYS = zy;
            String myZoneString = zoneXS + "," + zoneYS;

            Location * thisLoc = 0;
            thisLoc = GameWorld::getSingletonPtr()->gameLocations[myZoneString];

            if(thisLoc != 0)
            {
                thisLoc->UnloadExterior();
            }
        }
    }
}

///Terrain tile class
TerrainTile::TerrainTile(int zone_x, int zone_y, float zoom, float scale,Ogre::SceneManager * mSceneMgr, GameTerrain * parent)
{
    this->m_zoneX = zone_x;
    this->m_zoneY = zone_y;
    this->m_zoom = zoom;
    this->m_scale = scale;

    m_SceneMgr = mSceneMgr;

    this->m_Chunk = new TerrainChunk(m_zoneX,m_zoneY,m_zoom,m_scale,m_SceneMgr,parent);
}

TerrainTile::~TerrainTile()
{
    if(m_Chunk != 0)
    {
        delete m_Chunk;
    }
}

///Terrain chunk class
TerrainChunk::TerrainChunk(int zone_x, int zone_y, float zoom, float scale, Ogre::SceneManager * mSceneMgr, GameTerrain * parent)
{
    m_SceneNode = 0;
    m_TerrainEntity = 0;
    m_parent_terrain = parent;

    m_zoneX = zone_x;
    m_zoneY = zone_y;
    m_zoom = zoom;
    m_scale = scale;

    m_height = 64;
    m_width = 64;

    m_generatedGrassLastPass = false;

    m_mapArray = 0;

    m_SceneMgr = mSceneMgr;

    m_mapArray = new float[(64+1) * (64+1)];
    
    m_thisTileString = TerrainGenerator::createTerrainTile(m_mapArray,zoom,m_zoneX,m_zoneY,64, scale, m_SceneMgr);
    m_SceneNode = m_SceneMgr->getSceneNode(m_thisTileString);

    GameConfiguration* config = GameManager::getSingletonPtr()->getConfig();
    string terrainType = GameWorld::getSingletonPtr()->m_gameDataPath
        + config->getString("testTerrainDefinition");

    m_terrainDef = new TerrainDefType(terrainType);

    Entity * terrainHeightmap = m_SceneMgr->getEntity(m_thisTileString);
    if(terrainHeightmap != 0)
    {
        terrainHeightmap->setMaterialName(m_terrainDef->m_baseGroundMaterial);
        terrainHeightmap->setCastShadows(false);
    }
    m_TerrainEntity = terrainHeightmap;

    tileSeperation = 64 * scale;

    if(m_SceneNode != 0)
    {
        // Create shape
        BtOgre::StaticMeshToShapeConverter converter(terrainHeightmap);
        m_shape = converter.createTrimesh();

        btScalar mass = 0; // Fixed body

        // Create BtOgre MotionState (connects Ogre and Bullet)
        Vector3 pos = Vector3(tileSeperation * m_zoneX ,-2000, tileSeperation * m_zoneY);
        Quaternion rot = Quaternion();
        BtOgre::RigidBodyState* state = new BtOgre::RigidBodyState(
            btTransform(BtOgre::Convert::toBullet(rot), 
            BtOgre::Convert::toBullet(pos)), m_SceneNode);

        // Create the Body
        m_body = new btRigidBody(mass, state, m_shape);
        PhysicsManager::getSingletonPtr()->getWorld()->addRigidBody(m_body);
    }

    //Check if there is a location for this tile, and load it if we find one
    char zx[24];
    char zy[24];

    sprintf(zx, "%d", m_zoneX); //%G for float, %d for int
    sprintf(zy, "%d", m_zoneY);

    const String zoneXS = zx;
    const String zoneYS = zy;
    String myZoneString = zoneXS + "," + zoneYS;

    Location * thisLoc = 0;
    thisLoc = GameWorld::getSingletonPtr()->gameLocations[myZoneString];
    m_Location = thisLoc;

    m_terrainPos = Vector3(tileSeperation * (m_zoneX) ,-2000,tileSeperation * (m_zoneY));
    m_SceneNode->setPosition(Vector3(tileSeperation * m_zoneX ,-2000,tileSeperation * m_zoneY));

    if(m_parent_terrain->m_water != 0)
        m_parent_terrain->m_water->GetHeight();

    //Create texture splatting
    m_terrainMaterial = GetChunkMaterial();
    m_TerrainEntity->setMaterialName(m_terrainMaterial->getName());

    TextureGenerationOptions splat_1_options;
    splat_1_options.min_value = 125;
    splat_1_options.max_value = 125;

    TextureGenerationOptions splat_2_options;
    splat_2_options.min_value = 150;
    splat_2_options.max_value = 150;
    splat_2_options.min_height = 1705;

    m_splat1 = GenerateChunkTexture(1, 64, 64, splat_1_options);
    m_splat2 = GenerateChunkTexture(2, 64, 64, splat_2_options);

    //m_splat1 = GeneratePerlinZonedTerrainTexture(1,64,64,125,255,water_height);
    //m_splat2 = GeneratePerlinZonedTerrainTexture(2,64,64,150,255);

    if(m_splat1->isLoaded() == false)
        m_splat1->load();
    if(m_splat2->isLoaded() == false)
        m_splat2->load();

    m_splatBaseCreatesGrass = true;
    m_splat1CreatesGrass = true;
    m_splat2CreatesGrass = false;
    
    m_splatBaseCreatesVegetation = false;
    m_splat1CreatesVegetation = true;
    m_splat2CreatesVegetation = false;

    GenerateVegetation(0,0,64,64,"grass1");

    if(thisLoc != 0)
    {
        thisLoc->LoadExterior();
    }

    m_GrassBatchName = "grass_" + zoneXS + "," + zoneYS;
    m_SwapGrassBatchName = "grassSwap_" + zoneXS + "," + zoneYS;
    m_doUpdateGrass = false;
}

void TerrainChunk::waveGrass(Real timeElapsed, float grass_wave_sin)
{
    try
    {
        grassBatch = m_SceneMgr->getStaticGeometry(m_GrassBatchName);
    }
    catch(Exception e)
    {
        grassBatch = 0;
    }

    if(grassBatch != 0)
    {
        static Real xinc = Math::PI * 0.1;
        static Real zinc = Math::PI * 0.55;
        static Real xpos = Math::RangeRandom(-Math::PI, Math::PI);
        static Real zpos = Math::RangeRandom(-Math::PI, Math::PI);

        xpos += (xinc * timeElapsed) * 0.01;
        zpos += (zinc * timeElapsed) * 0.01;

        // Update vertex program parameters by binding a value to each renderable
        static Vector4 offset(0,0,0,0);
        static Vector4 wave(0,0,0,0);

        offset.x = 0.04;
        offset.z = 0.01;

        StaticGeometry::RegionIterator rit =  grassBatch->getRegionIterator();
        while (rit.hasMoreElements())
        {
            StaticGeometry::Region* reg = rit.getNext();

            // a little randomness
            xpos += reg->getCentre().x * 0.000000001;
            zpos += reg->getCentre().z * 0.000000001;

            StaticGeometry::Region::LODIterator lodit = reg->getLODIterator();
            while (lodit.hasMoreElements())
            {
                StaticGeometry::LODBucket* lod = lodit.getNext();
                StaticGeometry::LODBucket::MaterialIterator matit = 
                    lod->getMaterialIterator();
                while (matit.hasMoreElements())
                {
                    StaticGeometry::MaterialBucket* mat = matit.getNext();
                    StaticGeometry::MaterialBucket::GeometryIterator geomit = 
                        mat->getGeometryIterator();
                    while (geomit.hasMoreElements())
                    {
                        StaticGeometry::GeometryBucket* geom = geomit.getNext();
                        geom->setCustomParameter(999, offset);

                        wave.x = grass_wave_sin;
                        geom->setCustomParameter(998, wave);
                    }
                }
            }
        }
    }
}

void TerrainChunk::ClearGrass()
{
    try
    {
        grassBatch = m_SceneMgr->getStaticGeometry(m_GrassBatchName);
        grassBatch->reset();
    }
    catch(Exception e)
    {
    }
}

void TerrainChunk::removeGrass()
{
    try
    {
        grassBatch = m_SceneMgr->getStaticGeometry(m_GrassBatchName);
        m_SceneMgr->destroyStaticGeometry(grassBatch->getName());
    }
    catch(Exception e)
    {
    }
}

void TerrainChunk::removeTrees()
{
    int tileSize = m_width * 512; // TODO: Scale, '512' should be variable
    int x = m_zoneX * tileSize;
    int y = m_zoneY * tileSize;
    Forests::TBounds bounds(x, y, x + tileSize, y + tileSize);
    m_parent_terrain->m_treeLoader->deleteTrees(bounds);
}

void TerrainChunk::updateGrass(Vector3 position)
{
    if(!m_parent_terrain->m_doGrass)
        return; // Grass disabled

    int theTileSize = (64 * 512);

    int curXTile = Math::Floor(position.x / theTileSize);
    int curYTile = Math::Floor(position.z / theTileSize);

    float theXOffset = theTileSize * curXTile;
    float theYOffset = theTileSize * curYTile;

    theXOffset = position.x - theXOffset;
    theYOffset = position.z - theYOffset;

    float percentageX = theXOffset / theTileSize;
    float percentageY = theYOffset / theTileSize;

    int curX = Math::Floor(percentageX * 64);
    int curY = Math::Floor(percentageY * 64);

    int beginningX = (curX - 4);
    int beginningY = (curY - 4);
    int maxX = (curX + 4);
    int maxY = (curY + 4);

    //Check to see if we need to place the grass
    if(beginningX >= 0 || beginningY >= 0 || maxX <= 64 || maxY <= 64)
    {
        int startX = (beginningX >= 0) ? beginningX : 0;
        int startY = (beginningY >= 0) ? beginningY : 0;

        int endX = (maxX <= 63) ? maxX : 64;
        int endY = (maxY <= 63) ? maxY : 64;

        ClearGrass();
        GenerateGrass(startX,startY,endX,endY,"grassLand");
        m_generatedGrassLastPass = true;
    }
    else if(m_generatedGrassLastPass)
    {
        ClearGrass();
        m_generatedGrassLastPass = false;
    }
}

void TerrainChunk::GenerateGrass(int startX, int startY, int endX, int endY, String terrainType)
{
    int curVeg = 0;

    try
    {
        grassBatch = m_SceneMgr->getStaticGeometry(m_GrassBatchName);
    }
    catch(Exception e)
    {
        grassBatch = m_SceneMgr->createStaticGeometry(m_GrassBatchName);
        grassBatch->setRegionDimensions(Vector3(2000,20000,2000));
        grassBatch->setRenderingDistance(9000);
    }

    Entity * vegEntity;
    Entity * rockEntity;

    vegEntity = m_SceneMgr->createEntity("VegMesh","GrassClump2.mesh");
    rockEntity = m_SceneMgr->createEntity("GroundRockMesh","SmallGroundRocks.mesh");

    vegEntity->setNormaliseNormals(true);

    //Make grass for each tile on mesh
    //Check to see if we should actually make any grass first
    bool doGrass = false;
    if(m_splatBaseCreatesGrass || m_splat1CreatesGrass || m_splat2CreatesGrass || m_splat3CreatesGrass)
    {
        doGrass = true;
    }

    // Get the pixel buffers
    if(m_splat1->isLoaded() == false)
    {
        m_splat1->load();
    }
    if(m_splat2->isLoaded() == false)
    {
        m_splat2->load();
    }

    HardwarePixelBufferSharedPtr splat1PixelBuffer = m_splat1->getBuffer();
    HardwarePixelBufferSharedPtr splat2PixelBuffer = m_splat2->getBuffer();

    //Get the pixels for splat 1
    splat1PixelBuffer->lock(HardwareBuffer::HBL_NORMAL); // for best performance use HBL_DISCARD!
    const PixelBox& splat1PixelBox = splat1PixelBuffer->getCurrentLock();
    uint8* splat1PixelDest = static_cast<uint8*>(splat1PixelBox.data);

    //Get the pixels for splat 2
    splat2PixelBuffer->lock(HardwareBuffer::HBL_NORMAL); // for best performance use HBL_DISCARD!
    int texSplat2Width = splat2PixelBuffer->getWidth();
    int texSplat2Height = splat2PixelBuffer->getHeight();
    const PixelBox& splat2PixelBox = splat2PixelBuffer->getCurrentLock();
    uint8* splat2PixelDest = static_cast<uint8*>(splat2PixelBox.data);

    for(int x = startX; x < endX; x++)
    {
        for(int y = startY; y < endY; y++)
        {
            //bool * haveGenerated = &haveGeneratedGrassTile[(y * (this->m_height+1)) + x];

            if(doGrass == true)
            {
                srand(x + (y * 10000000));
                float grassDensity = 13;

                int numVegToMake = (int) grassDensity;

                for(int i = 0; i < numVegToMake; i++)
                {
                    float randXPos = (rand() % (int)m_scale);
                    float randYPos = (rand() % (int)m_scale);
                    

                    float xPos = (tileSeperation * m_zoneX) + (x * m_scale) + randXPos;
                    float yPos = (tileSeperation * m_zoneY) + (y * m_scale) + randYPos;

                    Vector3 Pos = Vector3(xPos ,500000, yPos);

                    //Find out where we're at on the tile, between 0 and 1, to use in finding out where to check on our splat texture
                    float minXPos = (tileSeperation * m_zoneX) + (0.2 * m_scale);
                    float minYPos = (tileSeperation * m_zoneY) + (0.2 * m_scale);
                    float maxXPos = (tileSeperation * m_zoneX) + (64 * m_scale);
                    float maxYPos = (tileSeperation * m_zoneY) + (64 * m_scale);

                    float curXScaler = (xPos - minXPos) / (maxXPos - minXPos);
                    float curYScaler = (yPos - minYPos) / (maxYPos - minYPos);

                    int texXPixel = (texSplat2Width) * curXScaler;
                    int texYPixel = (texSplat2Height) * curYScaler;
                    
                    //Check here to see if I should be making this here (don't make grass on dirt!)
                    bool canMakeGrassHere = true;

                    int checkLoc = ((texYPixel * (texSplat2Height)) * 4) + (texXPixel * 4);

                    if(splat2PixelDest[checkLoc+3] > (uint8)90)
                    {
                        if(!m_splat2CreatesGrass)
                            canMakeGrassHere = false;
                    }
                    else if(splat1PixelDest[checkLoc+3] > (uint8)90)
                    {
                        if(!m_splat1CreatesGrass)
                            canMakeGrassHere = false;
                    }
                    else if(!m_splatBaseCreatesGrass)
                    {
                        canMakeGrassHere = false;
                    }

                    if(canMakeGrassHere)
                    {
                        if(m_mapArray != 0)
                        {
                            float height = ((m_mapArray[(y * (this->m_height+1)) + x] * 70000) - 2000) + ((m_mapArray[((y + 1) * (this->m_height+1)) + (x+1)] * 70000) - 2000);

                            Pos.y = (height / 2.0) - 15;
                        }

                        // Find terrain surface by casting a ray
                        Vector3 dir = Vector3(0,-1,0);
                        Vector3 start = Pos;
                        start.y = 100000;
                        Real rayLength = 200000;

                        Vector3 end = (rayLength * dir) + start;

                        SingleObjectRayResultCallback rayCallback(BtOgre::Convert::toBullet(start),
                            BtOgre::Convert::toBullet(end), m_body);

                        PhysicsManager::getSingletonPtr()->getWorld()->rayTest(
                            BtOgre::Convert::toBullet(start), BtOgre::Convert::toBullet(end),
                            rayCallback);

                        Vector3 intersectNormal = Vector3(1,0,0);

                        if(rayCallback.hasHit())
                        {
                            Pos.y = rayCallback.m_hitPointWorld.y();
                            intersectNormal = BtOgre::Convert::toOgre(rayCallback.m_hitNormalWorld);
                        }

                        //Get rotation from the normal
                        Vector3 upVector = intersectNormal;
                        Vector3 dirVector = Vector3(1,0,0);

                        Vector3 rightVector = dirVector.crossProduct(upVector);
                        rightVector.normalise();

                        dirVector = rightVector.crossProduct(upVector);
                        dirVector.normalise();

                        Quaternion fromNormal;
                        fromNormal.FromAxes(rightVector, upVector, dirVector);

                        Quaternion randDir = Quaternion(Degree(rand() % 360), Vector3(0,1,0));
                        Quaternion initial = fromNormal * randDir;

                        float randScale = (rand() % 7);
                        Vector3 Scale = Vector3(35 + randScale,35 + randScale,35 + randScale);

                        //pick a random grass/bush material for this vegetation
                        //TODO: Base on foliage type area

                        int type = rand() % 100;

                        Entity * myEntity = vegEntity;
                        if(type < 80)
                        {
                            vegEntity->setMaterialName("Foliage/Grass1");
                        }
                        else if(type < 90)
                        {
                            myEntity = rockEntity;
                        }
                        else if(type < 97)
                        {
                            vegEntity->setMaterialName("Foliage/Weed2");
                        }
                        else if(type < 98)
                        {
                            vegEntity->setMaterialName("Foliage/Weed_Bush1");
                        }
                        else
                        {
                            vegEntity->setMaterialName("Foliage/PinkWhiteFlowers");
                        }

                        grassBatch->addEntity(myEntity,Pos,initial,Scale);
                    }
                    curVeg++;
                }
            }
        }
    }
    grassBatch->build();

    splat1PixelBuffer->unlock();
    splat2PixelBuffer->unlock();

    m_SceneMgr->destroyEntity("VegMesh");
    m_SceneMgr->destroyEntity("GroundRockMesh");
}

BillboardSet * TerrainChunk::getImposterBillboardSet(string imposterMaterial)
{
    BillboardSet * thisSet = 0;

    string imposterBillboardSetStr = m_thisTileString + "_TreeImposters_";
    imposterBillboardSetStr += imposterMaterial;

    try
    {
        thisSet = m_SceneMgr->getBillboardSet(imposterBillboardSetStr);
    }
    catch(Exception e)
    {
        thisSet = m_SceneMgr->createBillboardSet(imposterBillboardSetStr);
        m_SceneNode->attachObject(thisSet);

        thisSet->setMaterialName(imposterMaterial);
        thisSet->setCullIndividually(false);
        thisSet->setDefaultDimensions(1700,1700);
        thisSet->setAutoextend(true);
        thisSet->setSortingEnabled(false);
        thisSet->setCommonDirection(Vector3(1,0,0));
    }

    return thisSet;
}

void TerrainChunk::GenerateVegetation(int startX, int startY, int endX, int endY, String terrainType)
{
    //Check if there is a location for this tile, and load it if we find one
    char zx[128];
    char zy[128];

    sprintf(zx, "%d", m_zoneX); //%G for float, %d for int
    sprintf(zy, "%d", m_zoneY);

    const String zoneXS = zx;
    const String zoneYS = zy;
    String myZoneString2 = zoneXS + "," + zoneYS;

    Location * thisLoc = 0;
    thisLoc = GameWorld::getSingletonPtr()->gameLocations[myZoneString2];

    //terrain tile size is 512
    //Initialize used ground space array
    m_GenFoliageArray = new bool[(512+1) * (512+1)];
    m_FoliageShadowArray = new int[(512+1) * (512+1)];
    for(int gx = 0; gx < 512; gx++)
    {
        for(int gy = 0; gy < 512; gy++)
        {
            m_GenFoliageArray[(gy * (512+1)) + gx] = false;
            m_FoliageShadowArray[(gy * (512+1)) + gx] = 255;
        }
    }

    // Get the pixel buffers
    if(m_splat1->isLoaded() == false)
    {
        m_splat1->load();
    }
    if(m_splat2->isLoaded() == false)
    {
        m_splat2->load();
    }

    HardwarePixelBufferSharedPtr splat1PixelBuffer = m_splat1->getBuffer();
    HardwarePixelBufferSharedPtr splat2PixelBuffer = m_splat2->getBuffer();

    //Get the pixels for splat 1
    splat1PixelBuffer->lock(HardwareBuffer::HBL_NORMAL); // for best performance use HBL_DISCARD!
    const PixelBox& splat1PixelBox = splat1PixelBuffer->getCurrentLock();
    uint8* splat1PixelDest = static_cast<uint8*>(splat1PixelBox.data);

    //Get the pixels for splat 2
    splat2PixelBuffer->lock(HardwareBuffer::HBL_NORMAL); // for best performance use HBL_DISCARD!
    int texSplat2Width = splat2PixelBuffer->getWidth();
    int texSplat2Height = splat2PixelBuffer->getHeight();
    const PixelBox& splat2PixelBox = splat2PixelBuffer->getCurrentLock();
    uint8* splat2PixelDest = static_cast<uint8*>(splat2PixelBox.data);

    srand(m_zoneX * 500 + (m_zoneY * 10000000));

    int numTreesToMake = m_terrainDef->m_numToMake; //1860; //(int) treeDensity;

    for(int i = 0; i < numTreesToMake; i++)
    {
        float randXPos = (rand() % (int)m_scale * 64);
        float randYPos = (rand() % (int)m_scale * 64);

        float xPos = (tileSeperation * m_zoneX) + randXPos;
        float yPos = (tileSeperation * m_zoneY) + randYPos;

        bool doMake = false;

        int arraySize = 128;

        //arraySize = 64, += 256
        //arraySize = 128, += 128

        int xGenArrayMod = (int)xPos % (int)tileSeperation;
        int yGenArrayMod = (int)yPos % (int)tileSeperation;
        float xGenArrayScale = abs((float)xGenArrayMod / (float)tileSeperation);
        float yGenArrayScale = abs((float)yGenArrayMod / (float)tileSeperation);
        int xGenArrayPos = (int)(arraySize * xGenArrayScale);
        int yGenArrayPos = (int)(arraySize * yGenArrayScale);

        if(m_GenFoliageArray[(yGenArrayPos * (512+1)) + xGenArrayPos] == false)
        {
            m_GenFoliageArray[(yGenArrayPos * (512+1)) + xGenArrayPos] = true;
            xPos = (((float)xGenArrayPos / (float)arraySize) * (tileSeperation)) + (tileSeperation * m_zoneX);
            yPos = (((float)yGenArrayPos / (float)arraySize) * (tileSeperation)) + (tileSeperation * m_zoneY);
            xPos += 128.0;
            yPos += 128;
            doMake = true;
        }
        else {
            numTreesToMake++;
        }

        Vector3 Pos = Vector3(xPos ,500000, yPos);

        //Make sure trees don't grow on ground occupied / flattened by a location
        if(thisLoc && doMake == true)
        {
            float thisCheckX = randXPos;
            float thisCheckY = randYPos;
            float checkScale = (m_scale * 64);

            float checkMult = 0.45;
            if(thisCheckX > (checkScale * checkMult) && thisCheckX < checkScale - (checkScale * checkMult))
            {
                if(thisCheckY > (checkScale * checkMult) && thisCheckY < checkScale - (checkScale * checkMult))
                {
                    doMake = false;
                }
            }
        }

        //Find out where we're at on the tile, between 0 and 1, to use in finding out where to check on our splat texture
        float minXPos = (tileSeperation * m_zoneX) + (0.2 * m_scale);
        float minYPos = (tileSeperation * m_zoneY) + (0.2 * m_scale);
        float maxXPos = (tileSeperation * m_zoneX) + (64 * m_scale);
        float maxYPos = (tileSeperation * m_zoneY) + (64 * m_scale);

        float curXScaler = (xPos - minXPos) / (maxXPos - minXPos);
        float curYScaler = (yPos - minYPos) / (maxYPos - minYPos);

        int texXPixel = (texSplat2Width) * curXScaler;
        int texYPixel = (texSplat2Height) * curYScaler;

        //Make sure we're not underwater.
        if(m_mapArray != 0 && m_parent_terrain->m_water != 0)
        {
            float height = ((m_mapArray[(texYPixel * (this->m_height+1)) + texXPixel] * 70000) - 2000) + ((m_mapArray[((texYPixel + 1) * (this->m_height+1)) + (texXPixel+1)] * 70000) - 2000);
            Pos.y = (height / 2.0);

            if(Pos.y <= this->m_parent_terrain->m_water->GetHeight())
                doMake = false;
        }

        //Check the terrain splats to see if we should generate foliage here
        int checkLoc = ((texYPixel * (texSplat2Height)) * 4) + (texXPixel * 4);
        if(splat2PixelDest[checkLoc+3] > (uint8)90)
        {
            if(!m_splat2CreatesVegetation)
                doMake = false;
        }
        else if(splat1PixelDest[checkLoc+3] > (uint8)90)
        {
            if(!m_splat1CreatesVegetation)
                doMake = false;
        }
        else if(!m_splatBaseCreatesVegetation)
        {
            doMake = false;
        }

        /*int checkLoc = ((texYPixel * (texSplat2Height)) * 4) + (texXPixel * 4);
        if(splat2PixelDest[checkLoc+3] > (uint8)90)
        {
            doMake = false;
        }*/

        if(doMake)
        {
            Quaternion initial = Quaternion(Degree(rand() % 360), Vector3(0,1,0)) * Quaternion (Radian(1.57079633),Vector3(-1,0,0));

            float randScale = (rand() % 40);
            Vector3 Scale = Vector3(15 + randScale,15 + randScale,15 + randScale);

            int treeRadius = 0;

            int randGenerate = rand() % m_terrainDef->m_totalFoliageWeights;

            std::vector<TerrainFoliageDef*>::iterator it;
            int tDensity = 0;
            bool didMake = false;

            for (it = m_terrainDef->m_foliageDefList.begin(); it != m_terrainDef->m_foliageDefList.end(); ++it)
            {
                if(*it != 0 && !didMake)
                {
                    TerrainFoliageDef* thisDef = *it;

                    int startDensityNum = tDensity;
                    int endDensityNum = tDensity + thisDef->m_density;

                    if(randGenerate >= startDensityNum && randGenerate < endDensityNum)
                    {
                        placeTree(Pos, thisDef->m_Mesh, thisDef->m_baseScale);
                        didMake = true;

                        if(thisDef->m_numGrowAroundDefs > 0)
                        {
                            int toGenerateAroundNum = thisDef->m_growAroundMin + (rand() % (thisDef->m_growAroundMax - thisDef->m_growAroundMin));

                            for(int ig = 0; ig < toGenerateAroundNum; ig++)
                            {
                                bool didMake2 = false;
                                int tGenDensity = 0;
                                std::vector<TerrainFoliageDef*>::iterator git;
                                int randGenerateAround = rand() % thisDef->m_growAroundTotalDensity;

                                for (git = thisDef->m_foliageGrowAroundList.begin(); git != thisDef->m_foliageGrowAroundList.end(); ++git)
                                {
                                    if(*git != 0 && !didMake2)
                                    {
                                        TerrainFoliageDef* thisDef2 = *git;

                                        int tStartDensityNum = tGenDensity;
                                        int tEndDensityNum = tGenDensity + thisDef2->m_density;

                                        if(randGenerateAround >= tStartDensityNum && randGenerateAround < tEndDensityNum)
                                        {
                                            int growArRadius =  thisDef2->m_GrowAroundRadius;
                                            if(growArRadius == 0)
                                            {
                                                growArRadius = 100;
                                            }

                                            Ogre::Vector3 NewPos2 = Pos;
                                            NewPos2.x -= rand() % growArRadius;
                                            NewPos2.z -= rand() % growArRadius;
                                            NewPos2.x += rand() % growArRadius;
                                            NewPos2.z += rand() % growArRadius;

                                            placeTree(NewPos2, thisDef2->m_Mesh, thisDef2->m_baseScale);
                                            didMake2 = true;
                                        }

                                        tGenDensity += thisDef2->m_density;
                                    }
                                }
                            }
                        }
                    }

                    tDensity += thisDef->m_density;
                }
            }

            if(treeRadius > 0)
            {
                int sX = xGenArrayPos - treeRadius;
                int sY = yGenArrayPos - treeRadius;
                int eX = xGenArrayPos + treeRadius;
                int eY = yGenArrayPos + treeRadius;

                if(sX < 0)
                    sX = 0;
                if(sY < 0)
                    sY = 0;
                if(eX > 512)
                    eX = 512;
                if(eY > 512)
                    eY = 512;

                for(int fx = sX; fx < eX; fx++)
                {
                    for(int fy = sY; fy < eY; fy++)
                    {
                        m_GenFoliageArray[(fy * (512+1)) + fx] = true;
                    }
                }
            }
        }
    }

    splat1PixelBuffer->unlock();
    splat2PixelBuffer->unlock();
}

void TerrainChunk::placeTree(Vector3 pos, std::string meshName, float baseScale)
{
    // Find terrain surface by casting a ray
    Vector3 dir = Vector3(0,-1,0);
    Vector3 start = pos;
    start.y = 100000;
    Real rayLength = 200000;

    Vector3 end = (rayLength * dir) + start;

    SingleObjectRayResultCallback rayCallback(BtOgre::Convert::toBullet(start),
        BtOgre::Convert::toBullet(end), m_body);

    // Yes, you have to pass start and end twice
    PhysicsManager::getSingletonPtr()->getWorld()->rayTest(
        BtOgre::Convert::toBullet(start), BtOgre::Convert::toBullet(end),
        rayCallback);

    if(rayCallback.hasHit())
    {
        pos.y = rayCallback.m_hitPointWorld.y();
    }

    Degree rot = Degree(rand() % 360);

    // Add a bit of random scaling
    float randScale = rand() % (int)baseScale;
    float scale = baseScale + randScale;
    if (scale > 100.0) scale = 100.0;

    Entity* entity = m_parent_terrain->m_precachedFoliageList[meshName];

    // Add the tree at the desired position/rotation/scale
    m_parent_terrain->m_treeLoader->addTree(entity, pos, rot, scale);

    // Create physics body
    PhysicsBody* body = new PhysicsBody(*entity, 0, pos,
        Quaternion(rot, Vector3::UNIT_Y), Vector3::UNIT_SCALE * scale);
    m_terrainObjectBodies.push_back(body);
}

MaterialPtr TerrainChunk::GetChunkMaterial()
{
    MaterialPtr terrain_material;

    String material_name = "terrain_";
    material_name += StringConverter::toString(m_zoneX);
    material_name += ",";
    material_name += StringConverter::toString(m_zoneY);

    bool exists = false;
    try
    {
        terrain_material = Ogre::MaterialManager::getSingleton().getByName(material_name);
        if(terrain_material.get() != 0)
            exists = true;
        else
            exists = false;
    }
    catch(Exception e)
    {
        exists = false;
    }

    //If the material does not exist, make it
    if(exists == false)
    {
        Ogre::MaterialPtr matPtr = Ogre::MaterialManager::getSingleton().getByName("Terrain/Grass1");
        terrain_material = matPtr.get()->clone(material_name);
    }

    return terrain_material;
}

/*TexturePtr TerrainChunk::GeneratePerlinZonedTerrainTexture(int pass, int width, int height, float min, float max, float min_height, float max_height)
{
    int Scale = Math::Pow(2,1);

    //build the texture name
    char zoneXCharString[128];
    char zoneYCharString[128];
    char passCharString[2];

    itoa(m_zoneX,zoneXCharString,10);
    itoa(m_zoneY,zoneYCharString,10);
    itoa(pass,passCharString,10);

    String materialName = zoneXCharString;
    materialName += ",";
    materialName += zoneYCharString;

    //Check to see if we've made this material already
    bool exists = false;

    try
    {
        m_terrainMaterial = Ogre::MaterialManager::getSingleton().getByName(materialName + "_material");
        if(m_terrainMaterial.get() != 0)
            exists = true;
        else
            exists = false;
    }
    catch(Exception e)
    {
        exists = false;
    }

    //If the material does not exist, make it
    if(exists == false)
    {
        Ogre::MaterialPtr matPtr = Ogre::MaterialManager::getSingleton().getByName("Terrain/Grass1");
        m_terrainMaterial = matPtr.get()->clone(materialName + "_material");
    }

    //If this chunk has a location and is the dirt pass, then get the road texture
    if(m_Location != 0 && pass == 2)
    {
        if(m_Location->m_outdoorRoadSplatTexture != "")
        {
                m_terrainMaterial->getTechnique(0)->getPass(pass)->getTextureUnitState(1)->setTextureName(m_Location->m_outdoorRoadSplatTexture,TEX_TYPE_2D);
        }
    }
    else
    {
        TexturePtr texture = GeneratePerlinTexture(pass, width, height, min, max, min_height, max_height);
        m_terrainMaterial->getTechnique(0)->getPass(pass)->getTextureUnitState(1)->setTextureName(texture->getName(),TEX_TYPE_2D);
    }

    //Make sure the terrain is using the correct terrain material now
    m_TerrainEntity->setMaterialName(m_terrainMaterial->getName());

    return TextureManager::getSingleton().getByName(m_terrainMaterial->getTechnique(0)->getPass(pass)->getTextureUnitState(1)->getTextureName());
}*/

TexturePtr TerrainChunk::GenerateChunkTexture(int pass, int width, int height, TextureGenerationOptions options)
{
    String zone_x_str = StringConverter::toString(m_zoneX);
    String zone_y_str = StringConverter::toString(m_zoneY);
    String pass_str = StringConverter::toString(pass);

    String textureName = zone_x_str;
    textureName += ",";
    textureName += zone_y_str;
    textureName += ":";
    textureName += pass_str;

    //If we can, just try to return a cached texture
    try
    {
        TexturePtr this_texture = Ogre::TextureManager::getSingleton().getByName(textureName);
        if(this_texture.get() != 0)
            return this_texture;
    }
    catch(Exception e)
    {
    }

    //No texture find, looks like we'll have to make it
    TexturePtr texture = TextureManager::getSingleton().createManual(
    textureName,
    ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
    TEX_TYPE_2D,    // type
    width, height,  // width & height
    0,              // number of mipmaps
    PF_BYTE_BGRA,   // pixel format
    TU_STATIC);

    HardwarePixelBufferSharedPtr pixelBuffer = texture->getBuffer();

    // Lock the pixel buffer and get a pixel box
    pixelBuffer->lock(HardwareBuffer::HBL_NORMAL); // for best performance use HBL_DISCARD!
    const PixelBox& pixelBox = pixelBuffer->getCurrentLock();
    uint8* pDest = static_cast<uint8*>(pixelBox.data);

    float * perlin_noise_map = GeneratePerlinNoise(width, height);
    float * terrain_gen_map = GenerateTextureFromTerrain(width, height, options);

    //copy the contents of our image array to the texture
    for (int x = 0; x < width; x++)
    {
        for(int y = 0; y < height; y++)
        {
            int perlin_color = (int)(perlin_noise_map[(x * height) + y] * 256.0);
            int terrain_color = (int)(terrain_gen_map[(x * height) + y] * 256.0);
            int final_col = 0;

            if(perlin_color < options.min_value)
            {
                perlin_color = 0;
            }
            else if(perlin_color >= options.max_value)
            {
                perlin_color = 255;
            }

            final_col = perlin_color + terrain_color;

            if(final_col > 255)
            {
                final_col = 255;
            }

            *pDest++ = final_col; // B
            *pDest++ = final_col; // G
            *pDest++ = final_col; // R
            *pDest++ = final_col; // A
        }
    }

    pixelBuffer->unlock();
    delete[] perlin_noise_map;
    delete[] terrain_gen_map;

    //Assign the texture to the terrain material
    m_terrainMaterial->getTechnique(0)->getPass(pass)->getTextureUnitState(1)->setTextureName(texture->getName(),TEX_TYPE_2D);

    //If we are in a location, check if we have also to apply a road texture
    if(m_Location != 0 && pass == 2)
    {
        if(m_Location->m_outdoorRoadSplatTexture != "")
        {
            m_terrainMaterial->getTechnique(0)->getPass(2)->getTextureUnitState(1)->setTextureName(m_Location->m_outdoorRoadSplatTexture,TEX_TYPE_2D);

            // Garvek: Temp hack: copy scaled version of the Road texture -- need to replace the whole grass system by PG Grass
            TexturePtr tex_road = Ogre::TextureManager::getSingleton().getByName(m_Location->m_outdoorRoadSplatTexture);
            if (tex_road.get())
            {
                int th1 = pixelBuffer->getHeight();
                int tw1 = pixelBuffer->getWidth();

                HardwarePixelBufferSharedPtr pixelBuffer2 = tex_road.get()->getBuffer();
                int th2 = pixelBuffer2->getHeight();
                int tw2 = pixelBuffer2->getWidth();

                // We don't need to lock, already done in blit()
                pixelBuffer->blit(pixelBuffer2, Image::Box(0, 0, tw2-1, th2-1), Image::Box(0, 0, tw1-1, th1-1));
            }
        }
    }

    return texture;
}

float * TerrainChunk::GeneratePerlinNoise(int width, int height)
{
    int Scale = Math::Pow(2,1);

    float * bitmap = new float[(width+1) * (height+1)];

    perlinNoise newNoise;

    int x_offset = (m_zoneX * width) + Scale - (m_zoneX * 1);
    int y_offset = (m_zoneY * height) + Scale - (m_zoneY * 1);

    for(int x = 0 + x_offset; x < (width+1) + x_offset; x++)
    {
        for(int y = 0 + y_offset; y < (height+1) + y_offset; y++)
        {
            float col_value = newPerlinNoise::turbulence(bitmap, x, y, 4 * Scale, width, height, 2, 0.9) / 256.0;
            bitmap[(y - y_offset) * (height) + (x - x_offset)] = col_value;
        }
    }

    return bitmap;
}

float * TerrainChunk::GenerateTextureFromTerrain(int width, int height, TextureGenerationOptions options)
{
    float * bitmap = new float[(width+1) * (height+1)];

    for (int x = 0; x < width+1; x++)
    {
        for(int y = 0; y < height+1; y++)
        {
            float x_pos = x;
            float y_pos = y;

            x_pos = x_pos / width;
            y_pos = y_pos / height;

            float terrain_height = ApproximatedHeightAtPercentage(x_pos, y_pos);

            //Start at no color.
            bitmap[(y * height) + x] = 0;

            if(terrain_height < options.min_height)
            {
                bitmap[(y * height) + x] = 1.0;
            }
        }
    }

    return bitmap;

}

float TerrainChunk::TerrainHeightAt(int heightmap_x, int heightmap_y)
{
    return ( m_mapArray[(heightmap_y * (m_height + 1)) + heightmap_x] * 70000 ) - 2000;
}

//Aproximated height at a given map pixel
float TerrainChunk::ApproximatedHeightAt(int pos_x, int pos_y)
{
    //Return an approximation of the two corners of this quad
    if(pos_x > 63)
        pos_x = 63;
    if(pos_y > 63)
        pos_y = 63;

    float height = TerrainHeightAt(pos_x, pos_y) + TerrainHeightAt(pos_x + 1, pos_y + 1);
    return (height / 2.0) - 15;
}

//Aproximated height at a given percentage of the map
float TerrainChunk::ApproximatedHeightAtPercentage(float pos_x, float pos_y)
{
    int texXPixel = 64 * pos_x;
    int texYPixel = 64 * pos_y;

    return ApproximatedHeightAt(texXPixel, texYPixel);
}

/*TexturePtr TerrainChunk::GeneratePerlinTexture(int pass, int width, int height, int tex_min, int tex_max, float min_height, float max_height)
{
    int Scale = Math::Pow(2,1);

    //build the texture name
    char zoneXCharString[128];
    char zoneYCharString[128];
    char passCharString[2];

    itoa(m_zoneX,zoneXCharString,10);
    itoa(m_zoneY,zoneYCharString,10);
    itoa(pass,passCharString,10);

    String textureName = zoneXCharString;
    textureName += ",";
    textureName += zoneYCharString;
    textureName += ":";
    textureName += passCharString;

    try
    {
        TexturePtr this_texture = Ogre::TextureManager::getSingleton().getByName(textureName);
        if(this_texture.get() != 0)
            return this_texture;
    }
    catch(Exception e)
    {
    }

    // Create the texture
    TexturePtr texture = TextureManager::getSingleton().createManual(
    textureName, // name
    ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
    TEX_TYPE_2D,      // type
    width, height,         // width & height
    0,                // number of mipmaps
    PF_BYTE_BGRA,     // pixel format
    TU_DEFAULT);      // usage; should be TU_DYNAMIC_WRITE_ONLY_DISCARDABLE for
                    // textures updated very often (e.g. each frame)

    // Get the pixel buffer
    HardwarePixelBufferSharedPtr pixelBuffer = texture->getBuffer();

    // Lock the pixel buffer and get a pixel box
    pixelBuffer->lock(HardwareBuffer::HBL_NORMAL); // for best performance use HBL_DISCARD!
    const PixelBox& pixelBox = pixelBuffer->getCurrentLock();
    uint8* pDest = static_cast<uint8*>(pixelBox.data);

    //the array to hold our b&w image info
    float * mapArray = new float[(width+1) * (height+1)];

    perlinNoise newNoise;

    int xOffset = (m_zoneX * width) + Scale - (m_zoneX * 1);
    int yOffset = (m_zoneY * height) + Scale - (m_zoneY * 1);

    for(int x = 0 + xOffset; x < (width+1) + xOffset; x++)
    {
        for(int y = 0 + yOffset; y < (height+1) + yOffset; y++)
        {
            float curColValue = newPerlinNoise::turbulence(mapArray,x,y,4 * Scale,width,height,2,0.9) / 256.0; 
            mapArray[(y - yOffset) * (height) + (x - xOffset)] = curColValue;
        }
    }

    //copy the contents of our image array to the texture
    for (int j = 0; j < width; j++)
    {
        for(int i = 0; i < height; i++)
        {
            float col = mapArray[(j * height) + i] * 256;
            int finalCol = col;

            if(finalCol < tex_min)
                finalCol = 0;
            else
                finalCol = 250;

            if(finalCol > tex_max)
                finalCol = tex_max;

            *pDest++ = finalCol; // B
            *pDest++ = finalCol; // G
            *pDest++ = finalCol; // R
            *pDest++ = finalCol; // A
        }
    }

    // Unlock the pixel buffer
    pixelBuffer->unlock();

    delete[] mapArray;
    return texture;
}*/

void TerrainChunk::shadowGround(int x, int y, int radius)
{
    int startX = x - radius;
    int startY = y - radius;
    int endX = x + radius;
    int endY = y + radius;

    if(startX < 0)
    {
        startX = 0;
    }
    if(startY < 0)
    {
        startY = 0;
    }
    if(endX > 512)
    {
        endX = 512;
    }
    if(endY > 512)
    {
        endY = 512;
    }

    for(int ix = startX; ix < endX; ix++)
    {
        for(int iy = startY; iy < endY; iy++)
        {
            m_FoliageShadowArray[(iy * (512+1)) + ix] = 0;
        }
    }
}

void TerrainChunk::doIsNowCenterTile()
{
    //Check if there is a location for this tile, and load it if we find one
    char zx[128];
    char zy[128];

    sprintf(zx, "%d", m_zoneX); //%G for float, %d for int
    sprintf(zy, "%d", m_zoneY);

    const String zoneXS = zx;
    const String zoneYS = zy;
    String myZoneString2 = zoneXS + "," + zoneYS;

    Location * thisLoc = 0;
    thisLoc = GameWorld::getSingletonPtr()->gameLocations[myZoneString2];

    if(thisLoc != 0)
    {
        //Update Quest Scripts
        QuestManager::getSingletonPtr()->LocLoad(thisLoc);

        PlayState* theState = static_cast<PlayState*>(GameManager::getSingletonPtr()
            ->getCurrentState());
        theState->addStringToHUDInfo("Arrived at " + thisLoc->m_Name);
    }
}

TerrainChunk::~TerrainChunk()
{
    if(this != 0)
    {
        removeGrass();
        removeTrees();

        // Clean up physics stuff
        if(m_body->getMotionState())
            delete m_body->getMotionState();

        PhysicsManager::getSingletonPtr()->getWorld()->removeCollisionObject(m_body);

        delete m_body;
        delete m_shape; // TODO: Remove once shape caching is implemented

        for(unsigned int i = 0; i < m_terrainObjectBodies.size(); i++)
            delete m_terrainObjectBodies[i];

        if(this->m_SceneNode != 0)
        {
            this->m_SceneNode->removeAndDestroyAllChildren();
            m_SceneMgr->destroySceneNode(m_SceneNode->getName());
        }

        if(this->m_TerrainEntity != 0)
        {
            this->m_SceneMgr->destroyEntity(m_thisTileString);
        }

        if(m_terrainDef != 0)
        {
            delete m_terrainDef;
        }

        if(!m_terrainMaterial.isNull())
        {
            m_terrainMaterial.get()->unload();
        }

        delete[] m_mapArray;
        delete[] m_GenFoliageArray;
        delete[] m_FoliageShadowArray;
    }
}
