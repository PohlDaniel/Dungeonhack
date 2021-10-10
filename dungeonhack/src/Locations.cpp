#include "DungeonHack.h"
#include "Location.h"

#include "GameManager.h"
#include "EntityManager.h"
#include "QuestManager.h"
#include "SoundManager.h"
#include "GameWorld.h"
#include "GameTerrain.h"
#include "PlayState.h"
#include "Monster.h"
#include "NpcActor.h"
#include "Items.h"
#include "WaterPlane.h"
#include "Sky.h"

Interior::Interior(int ID, int type, String name, String blockFile)
{
    m_Name = name;
    m_BlockFile = blockFile;
    m_ScriptFile = "";
}

void Interior::Enter(int startPos)
{
    PlayState * theState = static_cast<PlayState *>(GameManager::getSingletonPtr()->getCurrentState());
    Ogre::SceneManager * theScene = GameManager::getSingletonPtr()->getSceneManager();

    theState->m_WorldTerrain->enterInterior();

    //Clean up old map
    /*if(this->m_ExteriorLoc->mLocRootOutdoorSceneNode != 0)
    {
        //this->m_ExteriorLoc->mLocRootOutdoorSceneNode->removeAndDestroyAllChildren();
        //PlayState * thisState = (PlayState*)GameManager::getSingletonPtr()->getCurrentState();
        theState->getSceneManager()->destroySceneNode(this->m_ExteriorLoc->mLocRootOutdoorSceneNode->getName());

        //theState->getSceneManager()->destroySceneNode("OUTDOOR_NODE");
        //this->m_ExteriorLoc->mLocRootOutdoorSceneNode;
    }*/

    //Make new root node for the new map
    this->m_ExteriorLoc->mLocRootIndoorSceneNode = theScene->getRootSceneNode()->createChildSceneNode();

    //clear positions, reset player position at start marker
    Ogre::Vector3 Offset = Vector3(0,0,0);  //Reset world position to the origin
    GameWorld::getSingletonPtr()->loadMap(m_BlockFile, Offset,
            this->m_ExteriorLoc->mLocRootIndoorSceneNode, this->m_ExteriorLoc,
            true);
    LocationMarker * startMarker = findEntranceMarker(startPos);

    if(startMarker != NULL)
        theState->m_Player->setPosition(startMarker->m_Position);
    else
        theState->m_Player->setPosition(Vector3(0,10,0));

    theState->m_Player->resetMovementEvents();

    initMarkers();

    //Update Quest Scripts
    QuestManager::getSingletonPtr()->LocInteriorLoad(this);
}

void Interior::addMarker(int type, Vector3 Pos, Quaternion rot)
{
    LocationMarker * newMarker = new LocationMarker();

    newMarker->type = type;
    newMarker->m_Position = Pos;
    newMarker->m_Rotation = rot;

    this->m_Markers.push_back(newMarker);
}

void Interior::addEntranceMarker(int num, Vector3 Pos, Quaternion rot)
{
    LocationMarker * newMarker = new LocationMarker();

    newMarker->type = ENTRANCE_MARKER;
    newMarker->m_Position = Pos;
    newMarker->m_Rotation = rot;

    m_EntranceMarkers[num] = newMarker;
}

void Interior::clearMarkers()
{
    this->m_Markers.clear();
}

//Adds monsters, treasure, and quest item to markers
void Interior::initMarkers()
{
    std::vector<LocationMarker*>::iterator it;
    for (it = m_Markers.begin(); it != m_Markers.end(); ++it)
    {
        if(*it)
        {
            LocationMarker * thisLoc = *it;
            if(thisLoc != 0)
            {
                if(thisLoc->type == MONSTER_MARKER)
                {
                    int randMonsterType = rand() % 3;

                    if(randMonsterType <= 1)
                    {
                        Monster* theMonster = new Monster("Zombie");
                        EntityManager::getSingletonPtr()->addLocalEntity(theMonster);
                        theMonster->Init(thisLoc->m_Position, Vector3::UNIT_SCALE, "Zombie.mesh",
                            "Attack1", "Walk", "Idle", "zombie_3.wav", "zombie_4.wav",
                            "zombie_1.wav");
                        theMonster->setName("Zombie_" +
                            Ogre::StringConverter::toString(theMonster->m_EntityID));
                    }
                    else
                    {
                        Monster* theMonster = new Monster("Spider");
                        EntityManager::getSingletonPtr()->addLocalEntity(theMonster);
                        theMonster->Init(thisLoc->m_Position,Vector3(1,1,1),"Spider.mesh","Attack1","Walk","Idle","spider_cry.wav","spider_cry.wav","spider_idle.wav");
                        theMonster->setName("Spider_" +
                            Ogre::StringConverter::toString(theMonster->m_EntityID));
                    }
                }
                else if(thisLoc->type == TREASURE_MARKER)
                {
                    //Create test weapon as treasure for now
                    Weapon* newWeapon = new Weapon();
                    EntityManager::getSingletonPtr()->addLocalEntity(newWeapon);
                    newWeapon->initialize("Iron Shortsword","SwordIcon", 10, 50, "SteelShortsword.mesh", "SteelShortsword.mesh", Vector3(1.0,1.0,1.0), Vector3(1.0,1.0,1.0), 20);
                    newWeapon->initializeWeapon(0, 2, 10, 10, 10);
                    newWeapon->loadLocalItem(thisLoc->m_Position);
                    newWeapon->setOrientation(thisLoc->m_Rotation);
                }
                else if(thisLoc->type == NPC_MARKER)
                {
                    NpcActor* theActor = new NpcActor();
                    EntityManager::getSingletonPtr()->addLocalEntity(theActor);
                    theActor->Init(thisLoc->m_Position,Vector3(1,1,1),"HUMAN_MALE_BODY.mesh","Walk","WalkBaked","IdleNew");
                    theActor->setOrientation(thisLoc->m_Rotation);
                    theActor->setName("InnKeeper_" +
                            Ogre::StringConverter::toString(theActor->m_EntityID));

                    // TODO: script loader should handle the path
                    theActor->m_onActivateScript = "bartender_talk.py";
                }
            }
        }
    }
}


//Start of location functions
LocationMarker * Interior::findEntranceMarker(int entranceNum)
{
    LocationMarker * entranceMarker = 0;
    entranceMarker = m_EntranceMarkers[entranceNum];

    return entranceMarker;
}

void Location::setRoadSplatTexture(String newSplat)
{
    m_outdoorRoadSplatTexture = newSplat;
}

void Location::EnterInterior(int interiorID, int entranceID)
{
    PlayState * theState = static_cast<PlayState *>(GameManager::getSingletonPtr()->getCurrentState());

    Ogre::SceneManager * theScene = GameManager::getSingletonPtr()->getSceneManager();

    //theState->m_WorldTerrain->enterInterior();

    theState->m_WorldTerrain->isIndoor = true;
    //theState->m_WorldTerrain->unloadTerrain();

    //Clear any markers loaded by the exterior
    clearMarkers();

    Camera * playerCam = theScene->getCamera("GameCamera");
    if(playerCam)
        playerCam->setFarClipDistance(2500);

    theState->m_water->setActive(false);
    
    theState->isOutdoor = false;
    theState->m_sky->m_sunLight->setVisible(false);
    theState->m_sky->mSkyNode->setVisible(false,true);
    theState->m_sky->theSun->showSun(false);
    theState->m_sky->m_sunLight->setDiffuseColour(ColourValue(0,0,0));
    theScene->setAmbientLight(ColourValue(0,0,0));
    theScene->setFog(FOG_EXP2,ColourValue(0,0,0,1),0.000500,0.0,1.0);

    this->mInteriors[interiorID]->Enter(entranceID);

    SoundManager::getSingletonPtr()->playStreamingOutdoorAmbient("");
    SoundManager::getSingletonPtr()->playStreamingMusic(mInteriors[interiorID]->m_MusicFile);
}

void Location::ExitInterior(int interiorID, int exitID)
{
    PlayState * theState = static_cast<PlayState *>(GameManager::getSingletonPtr()->getCurrentState());
    Ogre::SceneManager * theScene = GameManager::getSingletonPtr()->getSceneManager();

    //theState->m_WorldTerrain->exitInterior();

    //Clear any markers loaded by this interior
    this->mInteriors[interiorID]->clearMarkers();
    EntityManager::getSingletonPtr()->unloadLocalEntities();

    //Clean up old map
    mLocRootIndoorSceneNode->removeAndDestroyAllChildren();
    PlayState * thisState = (PlayState*)GameManager::getSingletonPtr()->getCurrentState();
    thisState->getSceneManager()->destroySceneNode(mLocRootIndoorSceneNode->getName());

    theState->m_WorldTerrain->exitInterior();

    //this->LoadExterior();

    //Place player at exit
    LocationMarker * exitMarker = findExitMarker(exitID);
    if(exitMarker != 0)
        theState->m_Player->setPosition(exitMarker->m_Position);

    int terrainTileSize = 512;
    int curTerrainTile_x = m_Zone.x;
    int curTerrainTile_y = m_Zone.y;
    theState->m_WorldTerrain->loadTerrain(curTerrainTile_x, curTerrainTile_y,terrainTileSize,theScene);

    theState->m_water->setActive(true);

    theState->m_WorldTerrain->isIndoor = false;
    theState->isOutdoor = true;
    theState->m_sky->mSkyNode->setVisible(true,true);
    theState->m_sky->theSun->showSun(true);
    theState->m_sky->m_sunLight->setVisible(true);

    Camera * playerCam = theScene->getCamera("GameCamera");
    if(playerCam)
        playerCam->setFarClipDistance(100000);

    thisState->playOutdoorMusic();
    theState->m_Player->resetMovementEvents();

    //Loop through markers here
    initMarkers();

    //Update Quest Scripts
    QuestManager::getSingletonPtr()->LocLoad(this);

    SoundManager::getSingletonPtr()->playStreamingMusic("RobertHood_DF-TheVirtuous.ogg");
}

String Location::getLocID()
{
    //Build LocID
    char zx[24];
    char zy[24];

    itoa(m_Zone.x,zx,10);
    itoa(m_Zone.y,zy,10);

    String myZoneString = zx;
    myZoneString += ",";
    myZoneString += zy;

    return myZoneString;
}

//Adds monsters, treasure, and quest item to markers
void Location::initMarkers()
{
    std::vector<LocationMarker*>::iterator it;
    for (it = m_Markers.begin(); it != m_Markers.end(); ++it)
    {
        if(*it)
        {
            LocationMarker * thisLoc = *it;
            if(thisLoc != 0)
            {
                if(thisLoc->type == MONSTER_MARKER)
                {
                    int randMonsterType = rand() % 2;

                    if(randMonsterType <= 1)
                    {
                        Monster* theMonster = new Monster("Monster");
                        EntityManager::getSingletonPtr()->addLocalEntity(theMonster);
                        theMonster->Init(thisLoc->m_Position, Vector3::UNIT_SCALE, "Zombie.mesh",
                            "Attack1", "Walk", "Idle", "zombie_3.wav", "zombie_4.wav",
                            "zombie_1.wav");
                    }
                    else
                    {
                        Monster* theMonster = new Monster("Monster");
                        EntityManager::getSingletonPtr()->addLocalEntity(theMonster);
                        theMonster->Init(thisLoc->m_Position,Vector3(1,1,1),"Spider.mesh","Attack1","Walk","Idle","spider_cry.wav","spider_cry.wav","spider_idle.wav");
                    }
                }
                else if(thisLoc->type == TREASURE_MARKER)
                {
                    //Create test weapon as treasure for now
                    Weapon* newWeapon = new Weapon();
                    EntityManager::getSingletonPtr()->addLocalEntity(newWeapon);
                    newWeapon->initialize("Iron Shortsword","SwordIcon", 10, 50, "SteelShortsword.mesh", "SteelShortsword.mesh", Vector3(1.0,1.0,1.0), Vector3(1.0,1.0,1.0), 20);
                    newWeapon->initializeWeapon(0, 2, 10, 10, 10);
                    newWeapon->loadLocalItem(thisLoc->m_Position);
                }
            }
        }
    }
}

void Location::initializeLocation(int zoneX, int zoneY, float radius, int countryID)
{
    m_Zone.x = zoneX;
    m_Zone.y = zoneY;

    this->m_radius = radius;
    this->m_CountryID = countryID;
}

Interior * Location::addInterior(int interiorID, int type, String name, String blockFile)
{
    Interior * newInterior = new Interior(interiorID,type,name,blockFile);
    newInterior->m_ExteriorLoc = this;
    mInteriors.push_back(newInterior);

    return newInterior;
}

void Location::addMarker(int type, Vector3 Pos, Quaternion rot)
{
    LocationMarker * newMarker = new LocationMarker();

    newMarker->type = type;
    newMarker->m_Position = Pos;
    newMarker->m_Rotation = rot;

    this->m_Markers.push_back(newMarker);
}

void Location::addEntranceMarker(int num, Vector3 Pos, Quaternion rot)
{
    LocationMarker * newMarker = new LocationMarker();

    newMarker->type = ENTRANCE_MARKER;
    newMarker->m_Position = Pos;
    newMarker->m_Rotation = rot;

    this->m_EntranceMarkers[num] = newMarker;
}

void Location::clearMarkers()
{
    this->m_Markers.clear();
}

string Location::getLocationMusic()
{
    return ( m_outdoorMusic );
}

LocationMarker * Location::findExitMarker(int entranceNum)
{
    LocationMarker * entranceMarker = 0;
    entranceMarker = m_EntranceMarkers[entranceNum];
    return entranceMarker;
}

void Town::initializeLocation(int zoneX, int zoneY, float radius, int countryID)
{
    m_Zone.x = zoneX;
    m_Zone.y = zoneY;

    this->m_radius = radius;
    this->m_CountryID = countryID;

    this->m_flattenGround = true;
    this->m_clearTrees = true;
}

void Dungeon::initializeDungeon(int zoneX, int zoneY, float radius, int countryID, String ExteriorBlock, String InteriorBlock, String Name, String m_IndoorMusic)
{
    m_Zone.x = zoneX;
    m_Zone.y = zoneY;

    this->m_radius = radius;
    this->m_CountryID = countryID;

    this->m_flattenGround = true;
    this->m_clearTrees = true;

    m_OutdoorBlock = ExteriorBlock;
    m_IndoorBlock = InteriorBlock;

    Ogre::SceneManager * theScene = GameManager::getSingletonPtr()->getSceneManager();
    mLocRootOutdoorSceneNode = theScene->getRootSceneNode()->createChildSceneNode();

    //LoadExterior();

    m_Name = Name;
    Interior * locInterior = this->addInterior(0,5,Name,m_IndoorBlock);
    locInterior->m_MusicFile = m_IndoorMusic;
}

void Dungeon::LoadExterior()
{
    Ogre::SceneManager * theScene = GameManager::getSingletonPtr()->getSceneManager();

    //GameManager::getSingletonPtr()->unloadLocalEntities();

    mLocRootOutdoorSceneNode = theScene->getRootSceneNode()->createChildSceneNode();
    Ogre::Vector3 Offset = Vector3((m_Zone.x + 0.5) * (64 * 512), m_LocHeight - 2000, (m_Zone.y + 0.5) * (64 * 512));
    GameWorld::getSingletonPtr()->loadMap(m_OutdoorBlock, Offset,
            mLocRootOutdoorSceneNode, this);

    /*
    LocationMarker * exitMarker = findExitMarker(0);
    if(exitMarker != 0)
    {
        theState->m_Player->m_pos = exitMarker->m_Position;
        theState->m_Player->m_collisionObject->m_PhysicsBody->setPositionOrientation(Quaternion(),exitMarker->m_Position);
    }*/

    //int terrainTileSize = 512;  //256

    m_isLoaded = true;
}

void Dungeon::UnloadExterior()
{
    if(mLocRootOutdoorSceneNode != 0)
    {
        mLocRootOutdoorSceneNode->detachAllObjects();
    }

    EntityManager::getSingletonPtr()->unloadLocalEntities();

    //Clean up old map
    if(mLocRootOutdoorSceneNode != 0)
    {
        mLocRootOutdoorSceneNode->removeAndDestroyAllChildren();
        PlayState * thisState = (PlayState*)GameManager::getSingletonPtr()->getCurrentState();
        thisState->getSceneManager()->destroySceneNode(mLocRootOutdoorSceneNode->getName());
        //theState->getSceneManager()->destroySceneNode();
    }

    m_isLoaded = false;
    //clear positions, reset player position at start marker
}
