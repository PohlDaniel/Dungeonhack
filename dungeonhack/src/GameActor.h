#ifndef GameActor_H
#define GameActor_H

#include "GameObject.h"
#include "Items.h"
#include "QuestManager.h"

class CharacterController;

class GameActor : public PhysicsEntity
{
public:

    GameActor();
    virtual ~GameActor();

    //Inventory items
    std::vector<Item *> m_InventoryItems;

    //ActorStatus m_Status;
    //ActorStats m_Stats;

    //EquippedItems m_Equipped;

    virtual void Update(float MoveFactor) { UpdateStats(MoveFactor);};
    virtual void UpdateStats(float MoveFactor) { };
    virtual void Delete() {delete this;};
    virtual bool CanSee(GameActor * otherActor);

    void callQuestFunction(String questFunction);
    virtual void onHurt();
    virtual void onDie();
    virtual void onAggro();
    virtual void onDamage(GameEntity* instigator);

    bool m_isInvisible;

    Vector3 m_weaponOffset; //Where projectiles spawn from and for hit checks too

    bool isAlive() { return (m_actorStats.curHealth > 0); };
    void startRunning() {isRunning = true;};
    void stopRunning() {isRunning = false;};
    bool isRunning;

    virtual const Ogre::Quaternion getOrientation();
    virtual const Ogre::Vector3 getPosition();
    virtual void setOrientation(const Ogre::Quaternion& quat);
    virtual void setPosition(const Ogre::Vector3& position);

    void setHealth(float newHealth)
    {
        if(newHealth < m_actorStats.maxHealth)
            m_actorStats.curHealth = newHealth;
        else
            m_actorStats.curHealth = m_actorStats.maxHealth;

        if(newHealth < 0)
            m_actorStats.curHealth = 0;
    };
    void setFatigue(float newFatigue)
    {
        if(newFatigue < m_actorStats.maxFatigue)
            m_actorStats.curFatigue = newFatigue;
        else
            m_actorStats.curFatigue = m_actorStats.maxFatigue;

        if(newFatigue < 0)
            m_actorStats.curFatigue = 0;
    };
    void setMagicka(float newMagicka)
    {
        if(newMagicka < m_actorStats.maxMagic)
            m_actorStats.curMagic = newMagicka;
        else
            m_actorStats.curMagic = m_actorStats.maxMagic;

        if(newMagicka < 0)
            m_actorStats.curMagic = 0;
    };

    float getStrength() {return m_actorStats.strength + m_actorStats.adj_strength; };
    float getEndurance() {return m_actorStats.endurance + m_actorStats.adj_endurance; };
    float getIntelligence() {return m_actorStats.intelligence+ m_actorStats.adj_intelligence; };
    float getWillpower() {return m_actorStats.willpower + m_actorStats.adj_willpower; };
    float getAgility() {return m_actorStats.agility + m_actorStats.adj_agility; };
    float getSpeed() {return m_actorStats.speed + m_actorStats.adj_speed; };
    float getPersonality() {return m_actorStats.personality + m_actorStats.adj_personality; };
    float getLuck() {return m_actorStats.luck + m_actorStats.adj_luck; };

    float getHealth() {return m_actorStats.curHealth; };
    float getFatigue() {return m_actorStats.curFatigue; };
    float getMagicka() {return m_actorStats.curMagic; };

    float getTotalHealth() {return m_actorStats.maxHealth; };
    float getTotalFatigue() {return m_actorStats.maxFatigue; };
    float getTotalMagicka() {return m_actorStats.maxMagic; };

    void setTotalHealth(float newHealth) {m_actorStats.maxHealth = newHealth; };
    void setTotalFatigue(float newFatigue) {m_actorStats.maxFatigue = newFatigue; };
    void setTotalMagicka(float newMagicka) {m_actorStats.maxMagic = newMagicka; };

    void resetStats();

    void setQuest(Quest * myQuest) { m_isQuestActor = true; m_quest = myQuest; };
    bool getIsQuestActor() { return m_isQuestActor; };

    float m_SpeedMod;

    VisibleEntity * m_Target;

    int faction_id; //0 is player, others used for other actor types. used so that actors know what other actors to attack or be friendly to

    GameActorStats m_actorStats;

    //Pawn settings
    int m_UpdateFrequency;
    int m_UpdateFreqCounter;

    Item * m_equippedWeapon;
    void equipWeapon(Item * newWeapon);
    virtual void equipItem(Item * theItem);
    int getClassID();

    WeaponManager m_WeaponManager;

    virtual void wieldWeapon() { };
    virtual void sheathWeapon() { };

    bool m_isWieldingWeapon;

    //Quest actor settings
    bool m_isQuestActor;
    Quest * m_quest;
    String onDieQuestFunction;
    String onHurtQuestFunction;
    String onAggroQuestFunction;

    void setQuestDieFunction(String function) { onDieQuestFunction = function; };
    void setQuestHurtFunction(String function){ onHurtQuestFunction = function; };
    void setQuestAggroFunction(String function){ onAggroQuestFunction = function; };

    // For collision
    float m_Height;
    float m_Radius;

protected:
    CharacterController* m_charControl;

    //GameActorStats m_actorStats;
    //float m_Health;
    //float m_Fatigue;
    //float m_Magicka;
};

#endif
