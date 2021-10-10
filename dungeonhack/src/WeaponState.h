#ifndef WeaponState_H
#define WeaponState_H

#include <OgreEntity.h>
using namespace Ogre;


class GameActor;
class WeaponState;

class WeaponManager
{
public:
    WeaponManager();
    WeaponManager(GameActor * owner);

    ~WeaponManager();

    void start(WeaponState* state, Entity *m_WeaponEntity);
    void changeState(WeaponState* state, Entity *m_WeaponEntity);

    void doneChangingState(WeaponState* state);

    WeaponState * currentState;
    WeaponState * nextState;

    void StartAttack();
    void CheckHitFrameCallback();
    void EndAttack();

    void pressedAttack();
    void releasedAttack();

    void update(float MoveFactor);

    GameActor * m_Owner;
};

class WeaponState
{
public:
    virtual void enter() = 0;
    virtual void exit() = 0;

    virtual void StartAttack() = 0;
    virtual void CheckHitFrame() = 0;
    virtual void EndAttack() = 0;

    virtual void pressedAttack() = 0;
    virtual void releasedAttack() = 0;

    Entity *m_WeaponEntity;
    WeaponManager * m_WeaponManager;

    virtual void update(float MoveFactor) = 0;

protected:
    WeaponState() {m_WeaponEntity = 0; }
};

class BladeState : public WeaponState
{
public:
    BladeState();

    void enter();
    void exit();

    void StartAttack();
    void Swing();
    void CheckHitFrame();
    void EndAttack();
    void EndAttackHit();

    void update(float MoveFactor);

    AnimationState* mWeaponAttackAnimation;
    float m_attackAnimSpeed;

    AnimationState* mWeaponIdleAnimation;
    float m_idleAnimSpeed;

    void playIdleAnimation();
    void playWindupAnimation();
    void playHoldAnimation();
    void playAttackAnimation();
    void playReturnAnimation();

    void pressedAttack();
    void releasedAttack();

    bool isWindup; //Windup to ready state
    bool isReady; //waiting to attack
    bool isAttacking; //attacking
    bool checkHit; //Only check for the hit once.
    bool isReturning;

    float readyFrame;
    float startHitCheckFrame;   //Frame to start weapon collision checking
    float endHitCheckFrame;     //Frame to end weapon collision checking
    bool isIdle();

    float blendFactor;
    float returnFactor;
    float hitTimePosition;
    bool doHitBlend;

    bool isAttackPressed;
    bool doNextAttack;

    Vector3 AttackDirection;
    float AttackStrength;
};

#endif
