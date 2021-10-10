#ifndef GameWorld_H
#define GameWorld_H

#include <string>
#include <vector>
#include <Ogre.h>
#include <OgreSingleton.h>

#include "GamePrereqs.h"
#include "GameObject.h"
#include "GameLight.h"

#include "tinyXML/tinyxml.h"

using namespace std;
using namespace stdext;
using namespace Ogre;


class GameWorld : public Ogre::Singleton<GameWorld>
{
public:
    GameWorld();

    static GameWorld& getSingleton(void);
    static GameWorld* getSingletonPtr(void);

    bool update(Real MoveFactor);

    std::vector<GameEntity*> mGameEntities;
    std::vector<GameEntity*> mGameEmmiters; //Might be able to be combined with mGameEntities

    //Entity set cache
    typedef pair <int, GameEntity *> GameEntity_Hash_Pair;
    hash_map <int, GameEntity *>::iterator GameEntity_Hash_Iter;
    hash_map <int, GameEntity *, DH_intHash> propSetCache;

    //Game World variables
    Ogre::Root * worldRootNode;
    Ogre::SceneManager * mSceneMgr;

    bool loadMap(String mapXmlFile, Vector3 Offset, Ogre::SceneNode * rootNode, Location * curLocation = 0, bool isInterior = false, int interiorNum = 0);
    bool loadLocations(String locationXMLFile);

    void loadFireEntityOptions(TiXmlNode * & node,float & Size,String & Emitter,float & LightOffset,bool & Lit,bool & MakeLight);
    void loadMonsterEntityOptions(TiXmlNode * & node,String & monsterID, String & monsterGroup);
    void loadScriptEntityOptions(TiXmlNode * & node,String & name,String & scriptFile,String & triggerType,String & triggerOther);
    void loadEntranceMarkerOptions(TiXmlNode * & node, int & num);
    void loadMarkerOptions(TiXmlNode * & node, String & MarkerType);
    void loadTeleportDoorOptions(TiXmlNode * & node, int & ExitNum, int & blockNum);

    void loadWorldMesh(String name, String mesh, Vector3 position,
            Vector3 scale, Quaternion orientation, SceneNode* locationNode);

    GameProp * loadPropFromMap(TiXmlNode * & node, String entityName, Vector3 Scale, Ogre::SceneNode * rootNode);
    GameProp * getPropFromSet(String setFile, String set, String entityName, Vector3 Scale, Ogre::SceneNode * rootNode);

    GameEmitter * loadEmmiterFromMap(TiXmlNode * & node, String entityName, Vector3 Scale, Ogre::SceneNode * rootNode);
    GameEmitter * getEmmiterFromSet(String setFile, String set, String entityName, Vector3 Scale, Ogre::SceneNode * rootNode);

    //Location functions
    int uniqueLocNum;   //Keeps track of how many areas have been loaded since the game started

    Location * getLocationFromID(String locID);
    Location * getLocation(int zoneX, int zoneY);
    Location * getFreeDungeon();
    Location * getFreeInn();

    hash_map <String, Location *>::iterator Location_Iter;
    hash_map<String, Location *, DH_StringHash> gameLocations;

    void add_Dungeon_Location(int zoneX,int zoneY, float radius, int countryID, String ExteriorBlock, String InteriorBlock, String Name, String TypeStr, String roadSplatTexture, String interiorMusic, String exterior_music);
    void add_Location(Location * newLocation);
    //add_Town_Location

    void getItem(String itemID);
    void loadWeaponItem(TiXmlNode * & node);

    /*! Path to directory containing all world data */
    String m_gameDataPath;

    //TODO: Add these
    //bool loadActorFromMap(TiXmlNode* node);
    //bool getActorFromSet(String set);
};

#endif
