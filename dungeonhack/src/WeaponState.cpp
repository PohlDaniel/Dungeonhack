#include "DungeonHack.h"
#include "WeaponState.h"
#include <Ogre.h>

WeaponManager::WeaponManager()
{
    m_Owner = NULL;
    currentState = 0;
    nextState = 0;
}

WeaponManager::WeaponManager(GameActor * owner)
{
    m_Owner = owner;
    currentState = 0;
    nextState = 0;
}

WeaponManager::~WeaponManager()
{
    currentState = 0;
    nextState = 0;
}

void WeaponManager::start(WeaponState *state,  Entity *m_WeaponEntity)
{
    this->currentState = state;
    this->nextState = 0;

    state->m_WeaponEntity = m_WeaponEntity;
    state->m_WeaponManager = this;
    currentState->enter();
}

void WeaponManager::changeState(WeaponState *state,  Entity *m_WeaponEntity)
{
    this->currentState->exit();
    delete currentState;

    if(state != 0)
    {
        this->nextState = state;
        state->m_WeaponEntity = m_WeaponEntity;
    }
}

void WeaponManager::CheckHitFrameCallback()
{
    //Weapon hit callback. Can be used for something :)
}

void WeaponManager::doneChangingState(WeaponState * state)
{
    currentState = nextState;

    if(nextState != 0)
    {
        currentState->enter();
    }
}

void WeaponManager::pressedAttack()
{
    if(currentState != 0)
    {
        currentState->pressedAttack();
    }
}

void WeaponManager::releasedAttack()
{
    if(currentState != 0)
    {
        currentState->releasedAttack();
    }
}

void WeaponManager::update(float MoveFactor)
{
    if(currentState != NULL)
    {
        currentState->update(MoveFactor);
    }
}
