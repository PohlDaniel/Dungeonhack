#ifndef Monster_H
#define Monster_H

#include "GameObject.h"
#include "GameActor.h"

class Monster : public GameActor
{
public:
    Monster(Ogre::String type);
    ~Monster();

    virtual void Update(float MoveFactor);
    virtual void Init(Vector3 Position, Vector3 Scale, String model = "Zombie.mesh", String attackanim = "Attack1", String walkanim = "Walk", String idleanim = "Idle", String hurtsound = "MonsterRoar.wav", String diesound = "MonsterRoar.wav", String alertsound = "MonsterRoar.wav");

    //Placeholder for now until we can make seperate attacks with the below TODO
    virtual void setAttackWait(float minWait, float maxWait);

    //TODO: function to add a new attack and set the animation, animation speed, how often the monster uses the attack, and then the time to wait to do the next attack
    //virtual void createAttack(String anim, float animspeed, float attackUseWeight, float minWaitAfter, float maxWaitAfter);

    virtual void onHurt();
    virtual void onDie();
    virtual void onAggro();

    Real m_meleeRange;  //Range from target to start mellee attack
    Real m_sightRange;  //Range from target where the target is visible
    Real m_stopRange;   //Range to stop advancing towards target
    Real m_turnSpeed;

    Vector3 LastSeenPosition;
    Real m_TimeSinceLastSeen;
    Real m_mass;

    bool m_isAlert;
    Real m_SearchTime;  //How long to search for the target after we havent saw it

    bool isWalking;
    bool isIdle;
    bool isAttacking;

    float m_idleTime;
    float m_walkAnimScaler; //Scales the walk animation based on how fast we're moving
    float m_runAnimScaler;
    AnimationState * mIdleAnimation;
    AnimationState * mWalkAnimation;
    AnimationState * mAttackAnimation;

    Real rotateProgress;

    Vector3 m_weaponOffset; //Where projectiles spawn from and for hit checks too
    float moveSpeed;
    float scale;

    float m_walkSpeed;
    float m_runSpeed;

    //Delay between attacks
    float m_maxAttackDelay;
    float m_minAttackDelay;
    float m_curAttackWait;
    float m_nextAttackWait;

    string model;
    string runanim;
    string walkanim;
    string attackanim;
    string idleanim;

    string m_soundHurt;
    string m_soundAlert;
    string m_soundDie;

    virtual void startMoveForward();
    virtual void endMoveForward();
    
    virtual void startAttack();
    virtual void endAttack();
    virtual void checkMeleeAttackHit();

    bool doMoveForward;

    float attack1HitFrame;
    bool checkedAttack;

    //So monsters don't try to walk into a door forever trying to get to the player
    bool m_waitForDoor;
    bool m_wasWaitingForDoor;
};

#endif
