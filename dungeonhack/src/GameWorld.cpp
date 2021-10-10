#include "DungeonHack.h"
#include "GameWorld.h"
#include <iostream>

#include "GameManager.h"
#include "GameConfig.h"
#include "EntityManager.h"
#include "Monster.h"
#include "Location.h"
#include "FireEntity.h"
#include "PythonManager.h"
#include "ScriptEntity.h"

#define SCALE_FACTOR 60

template<> GameWorld* Ogre::Singleton<GameWorld>::ms_Singleton = 0;

GameWorld::GameWorld()
{
    GameConfiguration* config = GameManager::getSingletonPtr()->getConfig();
    m_gameDataPath = config->getString("gameDataPath");
    uniqueLocNum = 0; 
}

GameWorld* GameWorld::getSingletonPtr(void)
{
    return ms_Singleton;
}

GameWorld& GameWorld::getSingleton(void)
{
    assert(ms_Singleton);
    return *ms_Singleton;
}

bool GameWorld::update(Real MoveFactor)
{
    //Update all of the entites in the game
    std::vector<GameEntity *>::iterator itVectorData;
    for(itVectorData = mGameEntities.begin(); itVectorData != mGameEntities.end(); itVectorData++)
    {
        GameEntity * thisEnt = *(itVectorData);

        thisEnt->Update(MoveFactor);
    }

    return 1;
}

bool GameWorld::loadLocations(String locationXMLFile)
{
    TiXmlDocument doc(m_gameDataPath + locationXMLFile);

    if(doc.LoadFile())
    {
        TiXmlNode* location = 0;
        TiXmlElement* locElement = 0;

        location = doc.FirstChild();
        location = location->FirstChild("locations");
        location = location->FirstChild("location");

        while(location != NULL)
        {
            Vector2 LocMapPos;
            String LocationName;
            String LocationType;
            String first_exterior_block = "";
            String first_interior_block = "";
            String first_interior_music = "";
            String exterior_music = "";
            String roadSplatTexture = "";

            float flattenSize = 2.0;

            TiXmlNode* travNode = 0;

            locElement = location->ToElement();
            LocationName = locElement->Attribute("name");
            LocationType = locElement->Attribute("type");

            flattenSize = atof(locElement->Attribute("size"));

            if(locElement->Attribute("music"))
            {
                exterior_music = locElement->Attribute("music");
            }

            if(locElement->Attribute("roadSplatTexture"))
            {
                roadSplatTexture = locElement->Attribute("roadSplatTexture");
            }

            if(locElement)
                travNode = locElement->FirstChildElement();

            //Get location data
            while(travNode != NULL)
            {
                locElement = travNode->ToElement();
                String nodeValue = travNode->Value();

                //Get the location's... location :P
                if (nodeValue.compare("position") == 0)
                {
                    String x_attrib = locElement->Attribute("x");
                    String y_attrib = locElement->Attribute("y");

                    LocMapPos.x = atof(x_attrib.c_str());
                    LocMapPos.y = atof(y_attrib.c_str());
                }

                //Get the blocks that make up this location
                if(nodeValue.compare("blocks") == 0)
                {
                    //Loop through and get all of the blocks
                    TiXmlNode* blockNode = 0;
                    TiXmlElement* blockElement = 0;

                    blockNode = travNode->FirstChild("block");
                    while(blockNode != NULL)
                    {
                        Vector2 blockPos;
                        String exteriorXMLBlock;

                        blockElement = blockNode->ToElement();

                        String x_attrib = blockElement->Attribute("x");
                        String y_attrib = blockElement->Attribute("y");
                        exteriorXMLBlock = blockElement->Attribute("exterior");

                        if(first_exterior_block == "")
                            first_exterior_block = exteriorXMLBlock;

                        blockPos.x = atof(x_attrib.c_str());
                        blockPos.y = atof(y_attrib.c_str());

                        //Get all of the interiors
                        TiXmlNode* interiorNode = 0;
                        TiXmlElement* interiorElement = 0;

                        interiorNode = blockNode->FirstChild("interiors");
                        interiorNode = interiorNode->FirstChild("interior");

                        while(interiorNode != NULL)
                        {
                            int interiorNum;
                            String interiorXMLBlockFile;
                            String musicFile;

                            interiorElement = interiorNode->ToElement();

                            String id_num = interiorElement->Attribute("id");
                            interiorXMLBlockFile = interiorElement->Attribute("block");

                            if(interiorElement->Attribute("music"))
                                musicFile = interiorElement->Attribute("music");

                            if(first_interior_music == "")
                                first_interior_music = musicFile;

                            if(first_interior_block == "")
                                first_interior_block = interiorXMLBlockFile;

                            interiorNum = (int)atof(id_num.c_str());

                            interiorNode = interiorNode->NextSibling();
                        }

                        blockNode = blockNode->NextSibling();
                    }
                }

                travNode = travNode->NextSibling();
            }

            if(LocationType == "DUNGEON")
            {
                this->add_Dungeon_Location(LocMapPos.x,LocMapPos.y,flattenSize,1,first_exterior_block, first_interior_block, LocationName, LocationType, roadSplatTexture, first_interior_music, exterior_music);
            }
            else if(LocationType == "INN")
            {
                this->add_Dungeon_Location(LocMapPos.x,LocMapPos.y,flattenSize,1,first_exterior_block, first_interior_block, LocationName, LocationType, roadSplatTexture, first_interior_music, exterior_music);
            }

            //Go to the next location in the list
            location = location->NextSibling();
        }

        return 1;
    }

    return 0;
}

bool GameWorld::loadMap(String mapXmlFile, Vector3 Offset, Ogre::SceneNode * rootNode, Location * curLocation, bool isInterior, int interiorNum)
{
    TiXmlDocument doc(m_gameDataPath + mapXmlFile);
    TiXmlNode* node = 0;
    TiXmlElement* nodeElement = 0;

    int thisMapNum = uniqueLocNum++;
    string mapName("_");
    char mapNum[20];

    itoa(thisMapNum,mapNum,10);
    mapName.append(mapNum);

    if(doc.LoadFile())
    {
        node = doc.FirstChild();
        node = node->FirstChild("nodes");
        node = node->FirstChild("node");

        //If this is an interior, load the ambient light colour
        if(isInterior == true)
        {
            TiXmlNode* environment = 0;
            environment = doc.FirstChild();
            environment = environment->FirstChild("environment");
            environment = environment->FirstChild("colourAmbient");

            TiXmlElement * environmentNodeElement = environment->ToElement();
            if(environmentNodeElement)
            {
                float r = atof(environmentNodeElement->Attribute("r"));
                float g = atof(environmentNodeElement->Attribute("g"));
                float b = atof(environmentNodeElement->Attribute("b"));

                Ogre::SceneManager * theScene = GameManager::getSingletonPtr()->getSceneManager();
                theScene->setAmbientLight(ColourValue(r,g,b));
            }
        }

        while(node != NULL)
        {
            TiXmlNode* travNode = 0;

            nodeElement = node->ToElement();
            if(nodeElement)
                travNode = nodeElement->FirstChildElement();

            String nodeName = nodeElement->Attribute("name");
            nodeName += mapName;

            bool isEntity = false;
            bool isStaticMesh = false;

            Vector3 Position;
            Vector3 Scale(1.0,1.0,1.0);
            Quaternion Rotation;

            String name;
            String mesh;

            while(travNode != NULL)
            {
                nodeElement = travNode->ToElement();
                String nodeValue = travNode->Value();

                if(nodeValue.compare("position") == 0)
                {
                    const char * x_attrib = nodeElement->Attribute("x");
                    const char * y_attrib = nodeElement->Attribute("y");
                    const char * z_attrib = nodeElement->Attribute("z");

                    Position.x = atof(x_attrib);
                    Position.y = atof(z_attrib);
                    Position.z = -(atof(y_attrib));
                }
                else if(nodeValue.compare("rotation") == 0)
                {
                    String qx_attrib = nodeElement->Attribute("qx");
                    String qy_attrib = nodeElement->Attribute("qy");
                    String qz_attrib = nodeElement->Attribute("qz");
                    String qw_attrib = nodeElement->Attribute("qw");

                    float qx = atof(qx_attrib.c_str());
                    float qy = atof(qy_attrib.c_str());
                    float qz = atof(qz_attrib.c_str());
                    float qw = atof(qw_attrib.c_str());
                    
                    Rotation = Quaternion(qw,qx,qy,qz);
                }
                else if(nodeValue.compare("scale") == 0)
                {
                    String scale_x_attrib = nodeElement->Attribute("x");
                    String scale_y_attrib = nodeElement->Attribute("y");
                    String scale_z_attrib = nodeElement->Attribute("z");

                    float scale_x = atof(scale_x_attrib.c_str());
                    float scale_y = atof(scale_y_attrib.c_str());
                    float scale_z = atof(scale_z_attrib.c_str());

                    Scale = Vector3(scale_x,scale_y,scale_z);
                }
                else if(nodeValue.compare("entity") == 0)
                {
                    isStaticMesh = true;
                    name = nodeElement->Attribute("name");
                    mesh = nodeElement->Attribute("meshFile");

                    name += mapName;
                }
                else if(nodeValue.compare("light") == 0)
                {
                    isStaticMesh = false;
                    String lightName = nodeElement->Attribute("name");
                    String lightType = nodeElement->Attribute("type");
                    Ogre::ColourValue lightColour;

                    lightName += mapName;

                    bool castShadows = false;

                    TiXmlNode* DataNode = nodeElement->FirstChildElement();
                    TiXmlElement* userDataElement = 0;

                    Ogre::SceneManager * theScene = GameManager::getSingletonPtr()->getSceneManager();
                    Light * newLight = theScene->createLight(nodeName);
                    EntityManager::getSingletonPtr()->addLight(newLight);

                    if(lightType == "point")
                    {
                        newLight->setType( Light::LT_POINT );
                    }
                    else
                    {
                        newLight->setType( Light::LT_POINT );
                    }

                    while(DataNode != 0)
                    {
                        String DataNodeValue = DataNode->Value();

                        if(DataNodeValue.compare("colourDiffuse") == 0)
                        {
                            userDataElement = DataNode->ToElement();

                            String diffuse_r = userDataElement->Attribute("r");
                            String diffuse_g = userDataElement->Attribute("g");
                            String diffuse_b = userDataElement->Attribute("b");

                            float r = atof(diffuse_r.c_str());
                            float g = atof(diffuse_g.c_str());
                            float b = atof(diffuse_b.c_str());

                            lightColour = ColourValue(ColourValue(r ,g, b) * 0.95);

                            newLight->setDiffuseColour(lightColour);
                            newLight->setSpecularColour(lightColour);
                        }
                        else if(DataNodeValue.compare("lightAttenuation") == 0)
                        {
                            userDataElement = DataNode->ToElement();

                            String atten_range = userDataElement->Attribute("range");
                            String atten_const = userDataElement->Attribute("constant");
                            String atten_linear = userDataElement->Attribute("linear");
                            String atten_quadratic = userDataElement->Attribute("quadratic");

                            float _const = atof(atten_const.c_str());
                            float _linear = atof(atten_linear.c_str());

                            newLight->setAttenuation(1000,_const + 0.3,_linear * 0.000005,0.0002273547 * (_linear * 1.5));
                        }

                        DataNode = DataNode->NextSibling();
                    }

                    GameLight* theLight = new GameLight();
                    EntityManager::getSingletonPtr()->addLocalEntity(theLight);
                    theLight->InitializeLight(newLight, (Position * 60) + Offset, rootNode);
                    theLight->setLightColour(lightColour);

                    newLight->setCastShadows(castShadows);
                }
                else if(nodeValue.compare("userData") == 0)
                {
                    TiXmlNode* userDataNode = nodeElement->FirstChildElement();
                    TiXmlElement* userDataElement = 0;

                    String userDataNodeValue = userDataNode->Value();

                    if(userDataNodeValue.compare("property") == 0)
                    {
                        String entityType;

                        userDataElement = userDataNode->ToElement();
                        String userdata_name = userDataElement->Attribute("name");
                        String userdata_data = userDataElement->Attribute("data");

                        //Find out what we are
                        if(userdata_name.compare("Type") == 0)
                        {
                            entityType = userdata_data;

                            //We've got some type of entity, find out what
                            if(entityType.compare("Prop") == 0)
                            {
                                isEntity = true;
                                isStaticMesh = false;

                                //Get the rest of the entity properties
                                userDataNode = userDataNode->NextSibling();

                                GameProp *theProp = 0;
                                theProp = loadPropFromMap(userDataNode,name, Scale,rootNode);

                                if(theProp != 0)
                                {
                                    theProp->setPosition((Position * 60) + Offset);
                                }
                            }
                            else if(entityType.compare("Emitter") == 0)
                            {
                                isEntity = true;

                                GameEmitter * theEmitter = 0;
                                theEmitter = loadEmmiterFromMap(userDataNode,name, Scale,rootNode);

                                if(theEmitter != 0)
                                {
                                    theEmitter->setPosition((Position * 60) + Offset);
                                    theEmitter->m_displayRepresentation->m_displaySceneNode->setPosition((Position * 60) + Offset);
                                }
                            }
                            else if(entityType.compare("Sound") == 0)
                            {
                                isEntity = true;

                                //TODO: Create new sound object here
                            }
                            else if(entityType.compare("Fire") == 0)
                            {
                                isEntity = true;

                                /*---Properties to get---
                                Size        Size of the flame
                                Emitter     Name of emitter to use
                                LightOffset How far to move the light away from the base along it's direction
                                Lit         bool, whether we start off lit
                                MakeLight   bool, if we need to create a light
                                */

                                float FireSize = 1.0;
                                String FireEmitter = GameManager::getSingletonPtr()->getConfig()->getString("defaultFireMaterial");
                                float LightOffsetFloat = 0;
                                bool Lit = true;
                                bool MakeLight = true;

                                loadFireEntityOptions(userDataNode,FireSize,FireEmitter,LightOffsetFloat,Lit,MakeLight);

                                //Get the orientation of the node so we know where to point it's offset
                                Quaternion initial(Radian(1.57079633),Vector3(-1,0,0));
                                Rotation = initial * Rotation;

                                Vector3 Direction = (Rotation) * Vector3(0,-1,0);
                                Vector3 LightOffset = (Direction * LightOffsetFloat);

                                FireEntity* theFireEntity = new FireEntity();
                                EntityManager::getSingletonPtr()->addLocalEntity(theFireEntity);
                                theFireEntity->init((Position * 60) + Offset,"Emmiters/TorchFire",Lit,-1.0,FireSize,LightOffset,MakeLight);

                                //This is a temporary hack to get to 0.1; whether to have a corona should be a light property
                                String CoronaName = name + ".Corona";
                                GameCorona* theCorona = new GameCorona();
                                EntityManager::getSingletonPtr()->addLocalEntity(theCorona);
                                theCorona->Init((Position * 60) + Offset,Scale,CoronaName);

                                //TODO: Create new sound object here
                            }
                            else if(entityType.compare("Corona1") == 0)
                            {
                                isEntity = true;

                                GameCorona* theCorona = new GameCorona();
                                EntityManager::getSingletonPtr()->addLocalEntity(theCorona);
                                theCorona->Init((Position * 60) + Offset,Scale,name);
                            }
                            else if(entityType.compare("Script") == 0)
                            {
                                isEntity = true;

                                String ScriptEntityName = "";
                                String ScriptFile = "";
                                String TriggerType = "";
                                String TriggerOther = "";

                                loadScriptEntityOptions(userDataNode,ScriptEntityName,ScriptFile,TriggerType,TriggerOther);

                                ScriptEntity* theScript = new ScriptEntity();
                                EntityManager::getSingletonPtr()->addLocalEntity(theScript);
                                theScript->Init((Position * 60),TriggerType,ScriptFile,TriggerOther);
                            }
                            else if(entityType.compare("Monster") == 0)
                            {
                                isEntity = true;

                                String monsterID = "";
                                String group = "Default";

                                loadMonsterEntityOptions(userDataNode,monsterID,group);

                                if(monsterID != "")
                                {
                                    if(monsterID == "Zombie" || monsterID == "zombie")
                                    {
                                        Monster* theMonster = new Monster("Zombie");
                                        EntityManager::getSingletonPtr()->addLocalEntity(theMonster);
                                        theMonster->Init((Position * 60) + Offset, Scale, "Zombie.mesh",
                                            "Attack1", "Walk", "Idle", "zombie_3.wav", "zombie_4.wav",
                                            "zombie_1.wav");
                                    }
                                    else if(monsterID == "Spider" || monsterID == "spider")
                                    {
                                        Monster* theMonster = new Monster("Spider");
                                        EntityManager::getSingletonPtr()->addLocalEntity(theMonster);
                                        theMonster->Init((Position * 60) + Offset,Scale,"Spider.mesh","Attack1","Walk","Idle","spider_cry.wav","spider_cry.wav","spider_idle.wav");
                                    }
                                }
                                else
                                {
                                    //TODO: Pick a monster from the given group/set here
                                    Monster* theMonster = new Monster("Zombie");
                                    EntityManager::getSingletonPtr()->addLocalEntity(theMonster);
                                    theMonster->Init((Position * 60) + Offset, Scale, "Zombie.mesh",
                                        "Attack1", "Walk", "Idle", "zombie_3.wav", "zombie_4.wav",
                                        "zombie_1.wav");
                                }
                            }
                            else if(entityType.compare("Door_1") == 0)
                            {
                                isStaticMesh = false;
                                /* TODO: re-enable once hinges are fixed
                                isEntity = true;
                                DoorProp* theDoor = new DoorProp;
                                EntityManager::getSingletonPtr()->addLocalEntity(theDoor);
                                theDoor->Init((Position * 60) + Offset,Rotation,Scale, mesh, name, rootNode);
                                */
                            }
                            else if(entityType.compare("Entrance") == 0)
                            {
                                isEntity = true;
                                isStaticMesh = false;
                                TeleportDoorProp* theDoor = new TeleportDoorProp();
                                theDoor->initPhysicsEntity(mesh, Scale * 60.0);
                                EntityManager::getSingletonPtr()->addLocalEntity(theDoor);
                                theDoor->m_Location = curLocation;

                                int exitNum = 0;
                                int blockNum = 0;
                                loadTeleportDoorOptions(userDataNode, exitNum, blockNum);
                                theDoor->m_exitNum = exitNum;
                                theDoor->m_interiorNum = blockNum;

                                theDoor->m_displayRepresentation->m_displayEntity->setNormaliseNormals(true);
                                theDoor->m_displayRepresentation->m_displayEntity->setCastShadows(false);

                                Quaternion initial(Radian(1.57079633),Vector3(-1,0,0));
                                Rotation = initial * Rotation;

                                theDoor->Init((Position * 60) + Offset, Rotation);
                            }
                            else if(entityType.compare("Enter") == 0)
                            {
                                isEntity = true;
                                isStaticMesh = false;

                                Quaternion initial(Radian(1.57079633),Vector3(-1,0,0));
                                Rotation = initial * Rotation;

                                int Num = 0;
                                loadEntranceMarkerOptions(userDataNode,Num);

                                if(!isInterior)
                                {
                                    curLocation->addEntranceMarker(Num,(Position * 60) + Offset, Rotation);
                                }
                                else
                                {
                                    curLocation->mInteriors[interiorNum]->addEntranceMarker(Num,(Position * 60) + Offset, Rotation);
                                }
                            }
                            else if(entityType.compare("Marker") == 0)
                            {
                                isEntity = true;
                                isStaticMesh = false;

                                Quaternion initial(Radian(1.57079633),Vector3(-1,0,0));
                                Rotation = initial * Rotation;

                                String markerType = "";
                                loadMarkerOptions(userDataNode,markerType);

                                int type = 0;

                                if(markerType == "Monster")
                                {
                                    type = MONSTER_MARKER;
                                }
                                else if(markerType == "Quest")
                                {
                                    type = QUEST_MARKER;
                                }
                                else if(markerType == "Treasure")
                                {
                                    type = TREASURE_MARKER;
                                }
                                /*else if(markerType == "NPC")
                                {
                                    type = NPC_MARKER;
                                }*/
                                else if(markerType == "Innkeep")
                                {
                                    type = NPC_MARKER;
                                }

                                if(!isInterior)
                                {
                                    curLocation->addMarker(type,(Position * 60) + Offset, Rotation);
                                }
                                else
                                {
                                    curLocation->mInteriors[interiorNum]->addMarker(type,(Position * 60) + Offset, Rotation);
                                }
                            }
                        }
                    }
                }

                travNode = travNode->NextSibling();
            }

            //We're done loading now, do something with the data
            if(isStaticMesh == true)
            {
                loadWorldMesh(name, mesh, (Position * 60) + Offset, Scale,
                        Rotation, rootNode);
            }

            node = node->NextSibling();
        }

        return 1;
    }
    std::cout << "Error loading map" << std::endl;
    return 0;   //Error while loading
}

void GameWorld::loadWorldMesh(String name, String mesh, Vector3 position,
        Vector3 scale, Quaternion orientation, SceneNode* locationNode)
{
    // TODO: Do something about loading markers and 'boxes' (script marker mesh)
    if(mesh == "Marker.mesh" || mesh == "box.mesh")
        return;

    Quaternion initial(Radian(1.57079633),Vector3(-1,0,0));
    orientation = initial * orientation;

    GeometryNode* geom = new GeometryNode(name, mesh, position, orientation,
            locationNode, scale * 60.0f);
    EntityManager::getSingletonPtr()->addGeometry(geom);
}

GameProp * GameWorld::loadPropFromMap(TiXmlNode * & node, String entityname, Vector3 Scale, Ogre::SceneNode * rootNode)
{
    GameProp * newProp = 0;
    TiXmlElement* nodeElement = 0;

    while(node != 0)
    {
        nodeElement = node->ToElement();

        String userdata_name = nodeElement->Attribute("name");
        String userdata_data = nodeElement->Attribute("data");

        if(userdata_name == "Set")
        {
            //Go choose an entity from this set
            newProp = getPropFromSet(m_gameDataPath + "prop_sets.xml", userdata_data,
                    entityname, Scale, rootNode);
        }
        else
        {
            //Load specific prop data here
        }

        node = node->NextSibling();
    }
    return newProp;
}

GameProp * GameWorld::getPropFromSet(String setFile, String setName, String entityName, Vector3 Scale, Ogre::SceneNode * rootNode)
{
    GameProp * newProp = 0;

    TiXmlDocument setDoc( setFile );
    TiXmlNode* setNode = 0;
    TiXmlElement* setNodeElement = 0;

    if(setDoc.LoadFile())
    {
        setNode = setDoc.FirstChild();
        setNode = setNode->FirstChild("sets");
        setNode = setNode->FirstChild("set");

        while(setNode != NULL)
        {
            TiXmlNode* travNode = 0;

            setNodeElement = setNode->ToElement();
            if(setNodeElement)
                travNode = setNodeElement->FirstChildElement();

            const char * nodeName = setNodeElement->Attribute("name");

            if(nodeName == setName)
            {
                TiXmlNode* DataNode = setNodeElement->FirstChildElement("entity");
                TiXmlElement* DataElement = 0;

                Vector3 DisplayScale(1.0,1.0,1.0);
                Vector3 DisplayRot;

                Vector3 ColScale(1.0,1.0,1.0);
                Vector3 ColRot;

                if(DataNode != 0)
                {
                    newProp = new GameProp(); // Create the entity
                    EntityManager::getSingletonPtr()->addLocalEntity(newProp);

                    DataElement = DataNode->ToElement();
                    if(DataElement)
                        travNode = DataElement->FirstChildElement();

                    while(travNode != 0)
                    {
                        DataElement = travNode->ToElement();

                        String value = DataElement->Value();

                        if(value == "display")
                        {
                            String mesh = DataElement->Attribute("mesh");
                            DisplayScale.x = atof(DataElement->Attribute("x_scale"));
                            DisplayScale.y = atof(DataElement->Attribute("y_scale"));
                            DisplayScale.z = atof(DataElement->Attribute("z_scale"));

                            DisplayRot.x = atof(DataElement->Attribute("x_rot"));
                            DisplayRot.y = atof(DataElement->Attribute("y_rot"));
                            DisplayRot.z = atof(DataElement->Attribute("z_rot"));

                            Radian x_r = Radian(Degree(DisplayRot.x));
                            Radian y_r = Radian(Degree(DisplayRot.y));
                            Radian z_r = Radian(Degree(DisplayRot.z));

                            newProp->m_DisplayMesh = mesh;
                            newProp->initPhysicsEntity(mesh, DisplayScale);

                            //Setup LODs
                            Ogre::Mesh::LodDistanceList ldl;
                            ldl.push_back(800);
                            newProp->m_displayRepresentation->m_displayEntity
                                ->getMesh()->generateLodLevels(ldl,
                                Ogre::ProgressiveMesh::VRQ_CONSTANT,
                                100);
                        }
                        // TODO: use collision hulls if they are provided
                        else if(value == "collision")
                        {
                            String primitive = DataElement->Attribute("primitive");

                            ColScale.x = atof(DataElement->Attribute("x_scale"));
                            ColScale.y = atof(DataElement->Attribute("y_scale"));
                            ColScale.z = atof(DataElement->Attribute("z_scale"));

                            double x_rot = atof(DataElement->Attribute("x_rot"));
                            double y_rot = atof(DataElement->Attribute("y_rot"));
                            double z_rot = atof(DataElement->Attribute("z_rot"));

                            Radian x_r = Radian(Degree(x_rot)) / 6.28318531;
                            Radian y_r = Radian(Degree(y_rot)) / 6.28318531;
                            Radian z_r = Radian(Degree(z_rot)) / 6.28318531;

                            newProp->m_CollisionScale = ColScale;

                            // TODO: Handle in PhysicsBody, leaving this for context
                            if(primitive == "cylinder")
                            {
                                // Basic collision hull
                            }
                            else
                            {
                                // Custom collision hull
                            }
                            newProp->m_CollisionMesh = primitive;
                        }
                        else if(value == "physics") //Enable physics
                        {
                            float mass = atof(DataElement->Attribute("mass"));
                            newProp->setMass(mass * 2); // Why *2 ?
                        }

                        travNode = travNode->NextSibling();
                    }
                }

                // Quit early for now to get the first, should pick one randomly
                // out of the list
                return newProp;
            }

            setNode = setNode->NextSibling();
        }
    }

    return newProp;
}

//Emmiter loading
GameEmitter * GameWorld::loadEmmiterFromMap(TiXmlNode * & node, String entityname, Vector3 Scale,Ogre::SceneNode * rootNode)
{
    GameEmitter * newProp = 0;
    TiXmlElement* nodeElement = 0;

    while(node != 0)
    {
        nodeElement = node->ToElement();

        String userdata_name = nodeElement->Attribute("name");
        String userdata_data = nodeElement->Attribute("data");

        if(userdata_name == "Set")
        {
            //Go choose an entity from this set
            newProp = getEmmiterFromSet(m_gameDataPath + "emmiter_sets.xml",
                    userdata_data, entityname, Scale,rootNode);

        }
        else
        {
            //Load specific prop data here
        }

        node = node->NextSibling();
    }
    return newProp;
}

GameEmitter * GameWorld::getEmmiterFromSet(String setFile, String setName, String entityName, Vector3 Scale, Ogre::SceneNode * rootNode)
{
    GameEmitter * newProp = 0;

    TiXmlDocument setDoc( setFile );
    TiXmlNode* setNode = 0;
    TiXmlElement* setNodeElement = 0;

    if(setDoc.LoadFile())
    {
        setNode = setDoc.FirstChild();
        setNode = setNode->FirstChild("sets");
        setNode = setNode->FirstChild("set");

        while(setNode != NULL)
        {
            TiXmlNode* travNode = 0;

            setNodeElement = setNode->ToElement();
            if(setNodeElement)
                travNode = setNodeElement->FirstChildElement();

            const char * nodeName = setNodeElement->Attribute("name");

            if(nodeName == setName)
            {
                TiXmlNode* DataNode = setNodeElement->FirstChildElement("entity");
                TiXmlElement* DataElement = 0;

                Vector3 DisplayScale(1.0,1.0,1.0);
                Vector3 DisplayRot;

                Vector3 ColScale(1.0,1.0,1.0);
                Vector3 ColRot;

                if(DataNode != 0)
                {
                    newProp = new GameEmitter(); //Create the entity
                    EntityManager::getSingletonPtr()->addLocalEntity(newProp);

                    DataElement = DataNode->ToElement();
                    if(DataElement)
                        travNode = DataElement->FirstChildElement();

                    while(travNode != 0)
                    {
                        DataElement = travNode->ToElement();

                        String value = DataElement->Value();

                        if(value == "display")
                        {
                            String emitter = DataElement->Attribute("emmiter");
                            DisplayScale.x = atof(DataElement->Attribute("x_scale"));
                            DisplayScale.y = atof(DataElement->Attribute("y_scale"));
                            DisplayScale.z = atof(DataElement->Attribute("z_scale"));

                            DisplayRot.x = atof(DataElement->Attribute("x_rot"));
                            DisplayRot.y = atof(DataElement->Attribute("y_rot"));
                            DisplayRot.z = atof(DataElement->Attribute("z_rot"));

                            Radian x_r = Radian(Degree(DisplayRot.x));
                            Radian y_r = Radian(Degree(DisplayRot.y));
                            Radian z_r = Radian(Degree(DisplayRot.z));

                            Ogre::SceneManager * theScene = GameManager::getSingletonPtr()->getSceneManager();

                            newProp->m_displayRepresentation->m_displaySceneNode = theScene->getRootSceneNode()->createChildSceneNode();

                            newProp->m_Emitter = theScene->createParticleSystem(entityName, emitter);
                            newProp->m_Emitter->setDefaultNonVisibleUpdateTimeout(1000);

                            newProp->m_displayRepresentation->m_displaySceneNode->attachObject(newProp->m_Emitter);
                            newProp->m_displayRepresentation->m_displaySceneNode->setScale(DisplayScale);
                        }

                        travNode = travNode->NextSibling();
                    }

                    if(newProp->m_displayRepresentation->m_displaySceneNode != 0)
                    {
                        
                    }
                }

                // Quit early for now to get the first, should pick one randomly
                // out of the list
                return newProp;
            }

            setNode = setNode->NextSibling();
        }
    }

    return newProp;
}

void GameWorld::loadScriptEntityOptions(TiXmlNode * & node,String & name,String & scriptFile,String & triggerType,String & triggerOther)
{
    TiXmlElement* nodeElement = 0;

    while(node != 0)
    {
        nodeElement = node->ToElement();

        String userdata_name = nodeElement->Attribute("name");
        String userdata_data = nodeElement->Attribute("data");

        if(userdata_name == "File")
        {
            scriptFile = userdata_data;
        }
        else if(userdata_name == "Name")
        {
            name = userdata_data;
        }
        else if(userdata_name == "Triggered")
        {
            triggerType = userdata_data;
        }
        else if(userdata_name == "Trigger")
        {
            triggerOther = userdata_data;
        }

        node = node->NextSibling();
    }
}

void GameWorld::loadFireEntityOptions(TiXmlNode * & node,float & Size,String & Emitter,float & LightOffset,bool & Lit,bool & MakeLight)
{
    TiXmlElement* nodeElement = 0;

    while(node != 0)
    {
        nodeElement = node->ToElement();

        String userdata_name = nodeElement->Attribute("name");
        String userdata_data = nodeElement->Attribute("data");

        if(userdata_name == "Size")
        {
            Size = atof(userdata_data.c_str());
        }
        else if(userdata_name == "Emitter")
        {
            Emitter = userdata_data;
        }
        else if(userdata_name == "LightOffset")
        {
            LightOffset = atof(userdata_data.c_str());
        }
        else if(userdata_name == "Lit")
        {
            if(userdata_data == "1")
            {
                Lit = true;
            }
            else
            {
                Lit = false;
            }
        }
        else if(userdata_name == "MakeLight")
        {
            if(userdata_data == "1")
            {
                MakeLight = true;
            }
            else
            {
                MakeLight = false;
            }
        }

        node = node->NextSibling();
    }
}

void GameWorld::loadMonsterEntityOptions(TiXmlNode * & node,String & monsterID, String & monsterGroup)
{
    TiXmlElement* nodeElement = 0;

    while(node != 0)
    {
        nodeElement = node->ToElement();

        String userdata_name = nodeElement->Attribute("name");
        String userdata_data = nodeElement->Attribute("data");

        if(userdata_name == "MonsterID")
        {
            monsterID = userdata_data;
        }
        else if(userdata_name == "Group")
        {
            monsterGroup = userdata_data;
        }

        node = node->NextSibling();
    }
}

void GameWorld::loadEntranceMarkerOptions(TiXmlNode * & node,int & num)
{
    TiXmlElement* nodeElement = 0;

    while(node != 0)
    {
        nodeElement = node->ToElement();

        String userdata_name = nodeElement->Attribute("name");
        String userdata_data = nodeElement->Attribute("data");

        if(userdata_name == "Num")
        {
            num = atoi(userdata_data.c_str());
        }

        node = node->NextSibling();
    }
}

void GameWorld::loadMarkerOptions(TiXmlNode * & node,String & MarkerType)
{
    TiXmlElement* nodeElement = 0;

    while(node != 0)
    {
        nodeElement = node->ToElement();

        String userdata_name = nodeElement->Attribute("name");
        String userdata_data = nodeElement->Attribute("data");

        if(userdata_name == "Marker")
        {
            MarkerType = userdata_data;
        }

        node = node->NextSibling();
    }
}

void GameWorld::loadTeleportDoorOptions(TiXmlNode * & node,int & ExitNum, int & blockNum)
{
    TiXmlElement* nodeElement = 0;

    while(node != 0)
    {
        nodeElement = node->ToElement();

        String userdata_name = nodeElement->Attribute("name");
        String userdata_data = nodeElement->Attribute("data");

        if(userdata_name == "ExitNum")
        {
            ExitNum = atoi(userdata_data.c_str());
        }
        else if(userdata_name == "EnterNum")
        {
            ExitNum = atoi(userdata_data.c_str());
        }
        else if(userdata_name == "Interior")
        {
            blockNum = atoi(userdata_data.c_str());
        }

        node = node->NextSibling();
    }
}

void GameWorld::add_Dungeon_Location(int zoneX,int zoneY, float radius, int countryID, String ExteriorBlock, String InteriorBlock, String Name, String TypeStr, String roadSplatTexture, String interior_music, String exterior_music)
{
    Dungeon * newDungeon = new Dungeon();
    newDungeon->initializeDungeon(zoneX, zoneY, radius, countryID, ExteriorBlock, InteriorBlock, Name, interior_music);

    newDungeon->m_locTypeStr = TypeStr;

    newDungeon->m_outdoorMusic = exterior_music;

    newDungeon->setRoadSplatTexture(roadSplatTexture);

    char zx[24];
    char zy[24];

    sprintf(zx, "%d", zoneX);   //%G for float, %d for int
    sprintf(zy, "%d", zoneY);

    const String zoneXS = zx;
    const String zoneYS = zy;
    String myZoneString = zoneXS + "," + zoneYS;

    gameLocations[myZoneString] = (Location *)newDungeon;
}

void GameWorld::add_Location(Location * newLocation)
{

}

Location * GameWorld::getLocation(int zoneX, int zoneY)
{
    char zx[24];
    char zy[24];

    sprintf(zx, "%d", zoneX);   //%G for float, %d for int
    sprintf(zy, "%d", zoneY);

    const String zoneXS = zx;
    const String zoneYS = zy;
    String myZoneString = zoneXS + "," + zoneYS;

    Location * thisLoc = 0;
    thisLoc = this->gameLocations[myZoneString];

    return thisLoc;
}

Location * GameWorld::getLocationFromID(String locID)
{
    Location * thisLoc = 0;
    thisLoc = this->gameLocations[locID];
    return thisLoc;
}

Location * GameWorld::getFreeDungeon()
{
    Location * new_quest_dungeon = 0;

    vector<Location *> foundLocs;
    int foundNum = 0;

    //Loop through all of the locations
    hash_map <String, Location *>::iterator getLocationLoop;
    for(getLocationLoop = gameLocations.begin(); getLocationLoop != gameLocations.end(); ++getLocationLoop)
    {
        Location * checkLoc = gameLocations[getLocationLoop->first];

        if(checkLoc != 0)
        {

            //Check to see if this is a free dungeon
            if(checkLoc->m_isInUse == false && checkLoc->m_locTypeStr == "DUNGEON")
            {
                foundLocs.push_back(checkLoc);
                foundNum++;
            }
        }
    }

    if(foundNum == 0)
    {
        return 0;
    }

    int randLoc = rand() % foundNum;
    new_quest_dungeon = foundLocs[randLoc];

    return new_quest_dungeon;
}

Location * GameWorld::getFreeInn()
{
    Dungeon * new_quest_inn = 0;

    vector<Location *> foundLocs;
    int foundNum = 0;

    //Loop through all of the locations
    hash_map <String, Location *>::iterator getLocationLoop;
    for(getLocationLoop = gameLocations.begin(); getLocationLoop != gameLocations.end(); ++getLocationLoop)
    {
        Location * checkLoc = getLocationLoop->second;

        if(checkLoc != 0)
        {
            //Check to see if this is a free Inn location
            if(checkLoc->m_isInUse == false && checkLoc->m_locTypeStr == "INN")
            {
                foundLocs.push_back(checkLoc);
                foundNum++;
            }
        }
    }

    if(foundNum == 0)
    {
        return 0;
    }

    int randLoc = rand() % foundNum;
    new_quest_inn = (Dungeon*)foundLocs[randLoc];

    return new_quest_inn;
}

void GameWorld::getItem(String itemID)
{
    TiXmlDocument doc(m_gameDataPath + "items.xml");
    TiXmlNode* theNode = 0;
    TiXmlElement* theElement = 0;

    if(doc.LoadFile())
    {
        bool foundItem = false;
        theNode = doc.FirstChild();
        theNode = theNode->FirstChild("items");
        theNode = theNode->FirstChild("item");

        while(theNode != NULL && foundItem == false)
        {
            //Read this item node
            if(theElement->Attribute("id") && theElement->Attribute("type"))
            {
                //Base item id and type
                String id;
                String type;

                theElement = theNode->ToElement();
                id = theElement->Attribute("id");
                type = theElement->Attribute("type");

                //We found our item, now load it
                if(id == itemID)
                {
                    loadWeaponItem(theNode);
                    foundItem = true;
                }

                //Go to the next location in the list
                theNode = theNode->NextSibling();
            }
        }
    }
}

void GameWorld::loadWeaponItem(TiXmlNode *&node)
{
    TiXmlNode* theNode = 0;
    TiXmlElement* nodeElement = 0;

    while(node != 0)
    {
        theNode = node->FirstChild("name");

        nodeElement = theNode->ToElement();

        node = node->NextSibling();
    }
}
