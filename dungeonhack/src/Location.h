#ifndef Location_H
#define Location_H

#include <Ogre.h>
#include "GamePrereqs.h"

using namespace Ogre;
using namespace std;
using namespace stdext;

#define ENTRANCE_MARKER 1
#define MONSTER_MARKER 2
#define TREASURE_MARKER 3
#define QUEST_MARKER 4
#define NPC_MARKER 5

class Location;

class LocationMarker
{
public:
    int type;
    Vector3 m_Position;
    Quaternion m_Rotation;
};

class Interior
{
public:
    Interior(int ID, int type, String name, String blockFile);

    Location * m_ExteriorLoc;
    stdext::hash_map<int, LocationMarker *, DH_intHash> m_EntranceMarkers;
    std::vector<LocationMarker *> m_Markers;

    String m_Name;
    String m_BlockFile;     //Block XML file to load for this interior. eg: TestTomb.map.xml
    String m_MusicFile;
    String m_ScriptFile;

    int m_InteriorType;     //Dungeon, house, castle, etc...

    LocationMarker * findEntranceMarker(int entranceNum);
    ColourValue m_ambientLightColour;

    virtual void Enter(int entranceNum);

    void initMarkers();

    void addMarker(int type, Vector3 Pos, Quaternion rot);
    void addEntranceMarker(int num, Vector3 Pos, Quaternion rot);
    void clearMarkers();
};

class Location
{
public:
    int m_CountryID;        //ID # of the containing country. Used for population race and building types
    Vector2 m_Zone;

    bool m_isLoaded;    //Is the location loaded in the game world?
    bool m_isInUse;     //Is the location being used by a quest?

    float m_radius;         //Area to flatten around this location. 1.0 = whole tile, 0.5 = half tile, TODO: Go greater than 1.0
    float m_LocHeight;      //The elevation to draw this Location at when it's loaded.

    bool m_flattenGround;   //Flattens the area around it. EG: Houses, cities, dungeon entrances
    bool m_clearTrees;      //Clear large foliage in it's area. EG: Trees, rocks, bushes
    
    int m_locType;          //for colour and type of location. EG: Dungeon exterior, town, city, farm, ruin

    string m_locTypeStr;
    String m_ScriptFile;
    String m_Name;

    String getLocID();

    virtual void initializeLocation(int zoneX,int zoneY, float radius, int countryID);
    void setRoadSplatTexture(String newSplat);

    void initMarkers();

    std::vector<Interior*> mInteriors;

    stdext::hash_map<int, LocationMarker *, DH_intHash> m_EntranceMarkers;
    std::vector<LocationMarker *> m_Markers;

    std::vector<Ogre::Entity *> mMeshEntities;
    std::vector<Ogre::SceneNode *> mSceneNodes;

    virtual void EnterInterior(int interiorID, int entranceID);
    virtual void ExitInterior(int interiorID, int exitID);

    virtual void LoadExterior() { };
    virtual void UnloadExterior() { };

    string getLocationMusic();

    Interior * addInterior(int interiorID, int type, String name, String interiorMap);

    void addMarker(int type, Vector3 Pos, Quaternion rot);
    void addEntranceMarker(int num, Vector3 Pos, Quaternion rot);
    void clearMarkers();

    LocationMarker * findExitMarker(int entranceNum);

    Ogre::SceneNode * mLocRootOutdoorSceneNode;
    Ogre::SceneNode * mLocRootIndoorSceneNode;

    string m_outdoorMusic;

    String m_outdoorRoadSplatTexture;

    Location() {
        m_ScriptFile = "";
        m_locType = -1;     //No type for a null location
        m_isInUse = false;
        m_isLoaded = false;
        m_outdoorRoadSplatTexture = "";
    }
};

class Town : public Location
{
public:
    Vector2 m_Size;         //x, y size (in blocks) of the town
    bool m_IsWalled;        //Is a walled city
    bool m_IsCapital;       //Is a capital city

    virtual void initializeLocation(int zoneX,int zoneY, float radius, int countryID);

    Town() {
        m_locType = 1;

        m_Size.x = 1;
        m_Size.y = 1;

        m_IsWalled = false;
        m_IsCapital = false;

        m_flattenGround = true;
        m_clearTrees = true;
    }
};

class Dungeon : public Location
{
public:
    String m_OutdoorBlock;  //Outdoor block to load
    String m_IndoorBlock;   //Indoor block to load
    String m_IndoorMusic;

    int dungeonType;    //For which monsters spawn

    virtual void initializeDungeon(int zoneX,int zoneY, float radius, int countryID, String ExteriorBlock, String InteriorBlock, String Name, String interior_music);
    virtual void LoadExterior();
    virtual void UnloadExterior();

    Dungeon() {
        m_locType = 5;
        dungeonType = 1;

        m_flattenGround = true;
        m_clearTrees = true;
    }
};

#endif
