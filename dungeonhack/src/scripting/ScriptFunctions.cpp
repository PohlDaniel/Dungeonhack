#include "DungeonHack.h"
#include "PyPrototypes.h"
#include "PythonManager.h"

#include "GameManager.h"
#include "PlayState.h"
#include "MessageState.h"
#include "SoundManager.h"

#include "EntityManager.h"
#include "EntityLoader.h"
#include "Items.h"
#include "Monster.h"
#include "Player.h"
#include "FireEntity.h"

#include "Location.h"
#include "GameWorld.h"
#include "GameTerrain.h"


//Game functions list
void addHudMessage(string message)
{
    PlayState * theState = (PlayState*)GameManager::getSingletonPtr()->getCurrentState();
    theState->addStringToHUDInfo(message);
}

int getLastButtonPressed()
{
    return MessageState::getInstance()->getReturnedValue();
}

void showMessage(string message)
{
    if (MessageState::getInstance()->hasMessage())
        return;

    MessageState::getInstance()->setMessageType("okButton"); //Let the game know what to do when this returns
    MessageState::getInstance()->setMessage(message);

    string gameOption[1];
    gameOption[0] = "OK";
    MessageState::getInstance()->setOptions(1, gameOption, PythonManager::getSingleton().getLastScript());

    MessageState::getInstance()->setReady();

    PythonManager::getSingleton().suspendThread();
}

void showYesNoMessage(string message)
{
    if (MessageState::getInstance()->hasMessage())
        return;

    MessageState::getInstance()->setMessageType("okButton"); //Let the game know what to do when this returns
    MessageState::getInstance()->setMessage(message);

    string gameOption[2];
    gameOption[0] = "Yes";
    gameOption[1] = "No";
    MessageState::getInstance()->setOptions(2, gameOption, PythonManager::getSingleton().getLastScript());

    MessageState::getInstance()->setReady();

    PythonManager::getSingleton().suspendThread();
}

void playMusic(string musicfile)
{
    SoundManager::getSingleton().playStreamingMusic(musicfile.c_str());
}

void playSound(string soundfile, float gain)
{
    SoundManager *sm = SoundManager::getSingletonPtr();
    sm->manageSound(sm->playSound(soundfile.c_str(),gain,false));
}

//Location types
bool LocationWrapper::isNull() const
{
    if(wrappedPtr == 0)
    {
        return true;
    }

    return false;
}

void LocationWrapper::setInUse(bool newVal) { wrappedPtr->m_isInUse = newVal; };
string LocationWrapper::getName() const { return wrappedPtr->m_Name; };
bool LocationWrapper::getInUse() const { return wrappedPtr->m_isInUse; };
bool LocationWrapper::getIsLoaded() const { return wrappedPtr->m_isLoaded; };

string LocationWrapper::getLocID()
{
    //Build LocID
    char zx[24];
    char zy[24];

    itoa(wrappedPtr->m_Zone.x,zx,10);
    itoa(wrappedPtr->m_Zone.y,zy,10);

    //sprintf(zx, "%d", wrappedPtr->m_Zone.x);  //%G for float, %d for int
    //sprintf(zy, "%d", wrappedPtr->m_Zone.y);

    //string zoneXS = zx;
    //string zoneYS = zy;

    string myZonestring = zx;
    myZonestring += ",";
    myZonestring += zy;

    return myZonestring;
}

int LocationWrapper::getZoneX() const { return wrappedPtr->m_Zone.x; };
int LocationWrapper::getZoneY() const { return wrappedPtr->m_Zone.y; };


QuestActorWrapper::QuestActorWrapper()
{
    wrappedPtr = 0;
}

QuestActorWrapper::~QuestActorWrapper() { }

bool QuestActorWrapper::isNull() const
{
    if(wrappedPtr == 0)
    {
        return true;
    }

    return false;
}

void QuestActorWrapper::setDieScriptFunction(string newFunction)
{
    wrappedPtr->onDieQuestFunction = newFunction;
}

void QuestActorWrapper::setHurtScriptFunction(string newFunction)
{
    wrappedPtr->onHurtQuestFunction = newFunction;
}

void QuestActorWrapper::setAggroScriptFunction(string newFunction)
{
    wrappedPtr->onAggroQuestFunction = newFunction;
}

MonsterActorWrapper::MonsterActorWrapper()
{
    wrappedPtr = 0;
}

MonsterActorWrapper::~MonsterActorWrapper() { }

bool MonsterActorWrapper::isNull() const
{
    if(wrappedPtr == 0)
    {
        return true;
    }

    return false;
}

//property get functions
float MonsterActorWrapper::getHealth() const { return wrappedPtr->getHealth(); };
float MonsterActorWrapper::getFatigue() const { return wrappedPtr->getFatigue(); };
float MonsterActorWrapper::getMagicka() const { return wrappedPtr->getMagicka(); };
float MonsterActorWrapper::getSpeedMod() const { return wrappedPtr->m_SpeedMod; };

float MonsterActorWrapper::getXPosition() const
{
    return wrappedPtr->getPosition().x;
}

float MonsterActorWrapper::getYPosition() const
{
    return wrappedPtr->getPosition().y;
}

float MonsterActorWrapper::getZPosition() const
{
    return wrappedPtr->getPosition().z;
}

bool MonsterActorWrapper::getIsInvisible() const { return wrappedPtr->m_isInvisible; };

//property set functions
void MonsterActorWrapper::setHealth(float newVal) { wrappedPtr->setHealth(newVal); };
void MonsterActorWrapper::setFatigue(float newVal) { wrappedPtr->setFatigue(newVal); };
void MonsterActorWrapper::setMagicka(float newVal) { wrappedPtr->setMagicka(newVal); };
void MonsterActorWrapper::setSpeedMod(float newVal) { wrappedPtr->m_SpeedMod = newVal; };
void MonsterActorWrapper::setIsInvisible(bool newVal) { wrappedPtr->m_isInvisible = newVal; };

void MonsterActorWrapper::setPosition(float x, float y, float z)
{
    wrappedPtr->setPosition(Vector3(x, y, z));
}

void MonsterActorWrapper::getPosition(float & x, float & y, float & z)
{
    Vector3 thisPos = wrappedPtr->getPosition();

    x = thisPos.x;
    y = thisPos.y;
    z = thisPos.z;
}

void MonsterActorWrapper::addImpulse(float x, float y, float z, float strength)
{
	Vector3 StartPos = wrappedPtr->getPosition();
    Vector3 Direction = Vector3(x,y,z);
    Direction.normalise();

    Direction *= strength;

    // TODO FIXME: Need function for adding impulse/force
    //wrappedPtr->m_collisionObject->m_PhysicsBody->addImpulse(Direction,StartPos);
}

//Wrapper class of a player, used to expose the player class to LUA indirectly
PlayerWrapper::PlayerWrapper()
{
    wrappedPtr = EntityManager::getSingleton().getPlayer();
}

PlayerWrapper::~PlayerWrapper() { }

bool PlayerWrapper::isNull() const
{
    if(wrappedPtr == 0)
    {
        return true;
    }

    return false;
}

//passthrough functions
void PlayerWrapper::startJumpEvent() { wrappedPtr->startJumpEvent(); };
void PlayerWrapper::wieldWeapon() { wrappedPtr->wieldWeapon(); };
void PlayerWrapper::sheathWeapon() { wrappedPtr->sheathWeapon(); };

//property get functions
float PlayerWrapper::getHealth() const { return wrappedPtr->getHealth(); };
float PlayerWrapper::getFatigue() const { return wrappedPtr->getFatigue(); };
float PlayerWrapper::getMagicka() const { return wrappedPtr->getMagicka(); };
float PlayerWrapper::getSpeedMod() const { return wrappedPtr->m_SpeedMod; };

float PlayerWrapper::getXPosition() const
{
    return wrappedPtr->getPosition().x;
}

float PlayerWrapper::getYPosition() const
{
    return wrappedPtr->getPosition().y;
}

float PlayerWrapper::getZPosition() const
{
    return wrappedPtr->getPosition().z;
}

bool PlayerWrapper::getIsInvisible() const { return wrappedPtr->m_isInvisible; };

//property set functions
void PlayerWrapper::setHealth(float newVal) { wrappedPtr->setHealth(newVal); };
void PlayerWrapper::setFatigue(float newVal) { wrappedPtr->setFatigue(newVal); };
void PlayerWrapper::setMagicka(float newVal) { wrappedPtr->setMagicka(newVal); };
void PlayerWrapper::setSpeedMod(float newVal) { wrappedPtr->m_SpeedMod = newVal; };
void PlayerWrapper::setIsInvisible(bool newVal) { wrappedPtr->m_isInvisible = newVal; };

void PlayerWrapper::setPosition(float x, float y, float z)
{
    wrappedPtr->setPosition(Vector3(x, y, z));
}

void PlayerWrapper::getPosition(float & x, float & y, float & z)
{
    Vector3 thisPos = wrappedPtr->getPosition();

    x = thisPos.x;
    y = thisPos.y;
    z = thisPos.z;
}

void PlayerWrapper::addImpulse(float x, float y, float z, float strength)
{
	Vector3 StartPos = wrappedPtr->getPosition();
    Vector3 Direction = Vector3(x,y,z);
    Direction.normalise();

    Direction *= strength;

    // TODO FIXME: Need function for adding impulse/force
    //wrappedPtr->m_collisionObject->m_PhysicsBody->addImpulse(Direction,StartPos);
}

int PlayerWrapper::getZoneX() const
{
    PlayState * theState = static_cast<PlayState *>(GameManager::getSingletonPtr()->getCurrentState());
    return theState->m_WorldTerrain->currentZoneX;

    return -1;
}

int PlayerWrapper::getZoneY() const
{
    PlayState * theState = static_cast<PlayState *>(GameManager::getSingletonPtr()->getCurrentState());
    return theState->m_WorldTerrain->currentZoneY;
}

bool PlayerWrapper::getIsOutdoor() const
{
    PlayState * theState = static_cast<PlayState *>(GameManager::getSingletonPtr()->getCurrentState());
    return theState->isOutdoor;
}

bool PlayerWrapper::isAtLocation()
{
    PlayState * theState = static_cast<PlayState *>(GameManager::getSingletonPtr()->getCurrentState());
    Location * thisLoc = theState->m_WorldTerrain->m_CenterTile->m_Chunk->m_Location;

    if(thisLoc != 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

LocationWrapper * PlayerWrapper::getCurrentLoc()
{
    PlayState * theState = static_cast<PlayState *>(GameManager::getSingletonPtr()->getCurrentState());
    Location * thisLoc = theState->m_WorldTerrain->m_CenterTile->m_Chunk->m_Location;

    if(thisLoc != 0)
    {
        LocationWrapper * newLocWrapper = new LocationWrapper();
        newLocWrapper->wrappedPtr = thisLoc;
        return newLocWrapper;
    }
    else
    {
        return 0;
    }
}

//Add a weapon to my inventory and equip it
void PlayerWrapper::createWeapon(string weaponID)
{
    if(weaponID == "shortsword")
    {
        /*
        Weapon* newWeapon = new Weapon();
        newWeapon->initialize("Iron Shortsword","weapon_sword.png", 10, 50, "SteelShortsword.mesh", "SteelShortsword.mesh", Vector3(1.0,1.0,1.0), Vector3(1.0,1.0,1.0), 20);
        EntityManager::getSingletonPtr()->addLocalEntity(newWeapon);
        newWeapon->initializeWeapon(0, 2, 10, 10, 10);

        wrappedPtr->m_InventoryItems.push_back(newWeapon);
        wrappedPtr->equipItem(newWeapon);
        */

        EntityLoadingRequest req;

        req.type = ENTITY_LOAD_WEAPON;

        req.name = "Iron Shortsword";
        req.inventoryIcon = "weapon_sword.png";
        req.weight = 10;
        req.worth = 50;
        req.Mesh = "SteelShortsword.mesh";
        req.collisionMesh = "SteelShortsword.mesh";
        req.displayScale = Vector3(1.0,1.0,1.0);
        req.collisionScale = Vector3(1.0,1.0,1.0);
        req.mass = 20;
        req.displayOffset = Vector3(0,0,0);
        req.collisionOffset = Vector3(0,0,0);
        req.centerOfGravity = Vector3(0,0,0);

        req.weaponType = 0;
        req.damage = 2;
        req.reach = 10;
        req.curCondition = 10;
        req.maxCondition = 10;

        EntityManager::getSingletonPtr()->enqueueRequest(&req);
    }
}

FireEntityWrapper::FireEntityWrapper(float x, float y, float z, float size)
{
    bool burning = true;
    float fuel = -1;

	wrappedPtr = new FireEntity();
    EntityManager::getSingletonPtr()->addLocalEntity(wrappedPtr);
    wrappedPtr->init(Vector3(x,y,z),"Emmiters/TorchFire",burning,fuel,size);
}

FireEntityWrapper::~FireEntityWrapper() { }

bool FireEntityWrapper::isNull() const
{
    if(wrappedPtr == 0)
    {
        return true;
    }

    return false;
}

//Set and get the name so that we can get this entity by it's name in LUA later
void FireEntityWrapper::setName(string newVal) { wrappedPtr->m_Name = newVal; };
string FireEntityWrapper::getName() const { return wrappedPtr->m_Name; };

void FireEntityWrapper::setPosition(float x, float y, float z)
{
	wrappedPtr->setPosition(Vector3(x, y, z));
}

void FireEntityWrapper::getPosition(float & x, float & y, float & z)
{
	Vector3 thisPos = wrappedPtr->getPosition();

    x = thisPos.x;
    y = thisPos.y;
    z = thisPos.z;
}

QuestWrapper::QuestWrapper()
{
    wrappedPtr = 0;
}

QuestWrapper::~QuestWrapper()
{

}

bool QuestWrapper::isNull() const
{
    if(wrappedPtr == 0)
    {
        return true;
    }

    return false;
}

void QuestWrapper::Start()
{
    wrappedPtr->Start();
}

void QuestWrapper::End()
{
    wrappedPtr->End();
}

void QuestWrapper::changeStage(int stage)
{
    wrappedPtr->changeStage(stage);
}

bool QuestWrapper::isQuestVariableSet(string var)
{
    if(wrappedPtr->getQuestVariable(var) != "")
    {
        return true;
    }
    else
    {
        return false;
    }
}

string QuestWrapper::getQuestVariable(string var)
{
    return wrappedPtr->getQuestVariable(var);
}

void QuestWrapper::setQuestVariable(string var, string newVal)
{
    wrappedPtr->setQuestVariable(var,newVal);
}

int QuestWrapper::getStage() const
{
    return wrappedPtr->m_currentStage;
}

string QuestWrapper::getName() const
{
    return wrappedPtr->m_Name;
}

QuestActorWrapper * QuestWrapper::addQuestMonster(string type, LocationWrapper * loc)
{
    QuestActorWrapper * newActor = new QuestActorWrapper();
    newActor->wrappedPtr = wrappedPtr->AddQuestMonster(type,loc->getLocID());
    return newActor;
}

void addQuest(string questName, string scriptFile)
{
    QuestManager::getSingletonPtr()->addQuest(questName, scriptFile);
}

bool questExists(string name)
{
    Quest * gotQuest = QuestManager::getSingleton().m_activeQuests[name];
    if(gotQuest != 0)
    {
        return true;
    }
    return false;
}

QuestWrapper * getQuest(string name)
{
    QuestWrapper * gotQuest = new QuestWrapper;
    gotQuest->wrappedPtr = QuestManager::getSingleton().m_activeQuests[name];
    return gotQuest;
}

/*class InnLocationWrapper
{
public:
    Dungeon * wrappedPtr;

    void setInUse(bool newVal) { wrappedPtr->m_isInUse = newVal; };
    string getName() const { return wrappedPtr->m_Name; };
    bool getInUse() const { return wrappedPtr->m_isInUse; };
    bool getIsLoaded() const { return wrappedPtr->m_isLoaded; };

    int getZoneX() const { return wrappedPtr->m_Zone.x; };
    int getZoneY() const { return wrappedPtr->m_Zone.y; };
};*/

LocationWrapper * getFreeDungeonLoc()
{
    Location * gotLoc = GameWorld::getSingleton().getFreeDungeon();
    LocationWrapper * newLocWrapper = new LocationWrapper();
    newLocWrapper->wrappedPtr = gotLoc;

    return newLocWrapper;
}

LocationWrapper * getFreeInnLoc()
{
    Location * gotLoc = GameWorld::getSingleton().getFreeInn();
    LocationWrapper * newLocWrapper = new LocationWrapper();
    newLocWrapper->wrappedPtr = gotLoc;

    return newLocWrapper;
}

LocationWrapper * getLocFromID(string locID)
{
    Location * gotLoc = GameWorld::getSingletonPtr()->getLocationFromID(locID);

    LocationWrapper * newLocWrapper = new LocationWrapper();
    newLocWrapper->wrappedPtr = gotLoc;

    return newLocWrapper;
}

//Get the player
PlayerWrapper * getPlayer()
{
    PlayerWrapper * thePlayer = new PlayerWrapper();
    return (thePlayer);
}
