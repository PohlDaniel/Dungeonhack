#ifndef _PY_PROTOTYPES
#define _PY_PROTOTYPES

#include <string>
using namespace std;

void addHudMessage(string message);

int getLastButtonPressed();

void showMessage(string message);

void showYesNoMessage(string message);

void playMusic(string musicfile);

void playSound(string soundfile, float gain);

class Location;
class QuestActor;
class Monster;
class GamePlayer;
class FireEntity;
class Quest;

//Location types
class LocationWrapper
{
public:
    Location * wrappedPtr;

    bool isNull() const;

    void setInUse(bool newVal);
    string getName() const;
    bool getInUse() const;
    bool getIsLoaded() const;

    string getLocID();

    int getZoneX() const;
    int getZoneY() const;
};


class QuestActorWrapper
{
public:
    QuestActorWrapper();

    ~QuestActorWrapper();

    QuestActor * wrappedPtr;

    bool isNull() const;

    void setDieScriptFunction(string newFunction);

    void setHurtScriptFunction(string newFunction);

    void setAggroScriptFunction(string newFunction);
};


class MonsterActorWrapper
{
public:
    MonsterActorWrapper();

    ~MonsterActorWrapper();

    Monster * wrappedPtr;

    bool isNull() const;

    //property get functions
    float getHealth() const;
    float getFatigue() const;
    float getMagicka() const;
    float getSpeedMod() const;

    float getXPosition() const;

    float getYPosition() const;

    float getZPosition() const;

    bool getIsInvisible() const;

    //property set functions
    void setHealth(float newVal);
    void setFatigue(float newVal);
    void setMagicka(float newVal);
    void setSpeedMod(float newVal);
    void setIsInvisible(bool newVal);

    void setPosition(float x, float y, float z);

    void getPosition(float & x, float & y, float & z);

    void addImpulse(float x, float y, float z, float strength);
};


//Wrapper class of a player, used to expose the player class to LUA indirectly
class PlayerWrapper
{
public:
    PlayerWrapper();

    ~PlayerWrapper();

    GamePlayer * wrappedPtr;

    bool isNull() const;

    //passthrough functions
    void startJumpEvent();
    void wieldWeapon();
    void sheathWeapon();

    //property get functions
    float getHealth() const;
    float getFatigue() const;
    float getMagicka() const;
    float getSpeedMod() const;

    float getXPosition() const;

    float getYPosition() const;

    float getZPosition() const;

    bool getIsInvisible() const;

    //property set functions
    void setHealth(float newVal);
    void setFatigue(float newVal);
    void setMagicka(float newVal);
    void setSpeedMod(float newVal);
    void setIsInvisible(bool newVal);

    void setPosition(float x, float y, float z);

    void getPosition(float & x, float & y, float & z);

    void addImpulse(float x, float y, float z, float strength);

    int getZoneX() const;

    int getZoneY() const;

    bool getIsOutdoor() const;

    bool isAtLocation();

    LocationWrapper * getCurrentLoc();

    //Add a weapon to my inventory and equip it
    void createWeapon(string weaponID);
};


class FireEntityWrapper
{
public:
    FireEntityWrapper(float x, float y, float z, float size = 1.0);

    ~FireEntityWrapper();

    FireEntity * wrappedPtr;

    bool isNull() const;

    //Set and get the name so that we can get this entity by it's name in LUA later
    void setName(string newVal);
    string getName() const;

    void setPosition(float x, float y, float z);

    void getPosition(float & x, float & y, float & z);
};


class QuestWrapper
{
public:
    QuestWrapper();

    ~QuestWrapper();

    Quest * wrappedPtr;

    bool isNull() const;

    void Start();

    void End();

    void changeStage(int stage);

    bool isQuestVariableSet(string var);

    string getQuestVariable(string var);

    void setQuestVariable(string var, string newVal);

    int getStage() const;

    string getName() const;

    QuestActorWrapper * addQuestMonster(string type, LocationWrapper * loc);
};


void addQuest(string questName, string scriptFile);
bool questExists(string name);
QuestWrapper * getQuest(string name);

LocationWrapper * getFreeDungeonLoc();
LocationWrapper * getFreeInnLoc();
LocationWrapper * getLocFromID(string locID);

//Get the player
PlayerWrapper * getPlayer();


#endif
