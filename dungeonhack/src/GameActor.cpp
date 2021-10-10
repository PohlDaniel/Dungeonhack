#include "DungeonHack.h"
#include "GameActor.h"
#include "GameObject.h"
#include "GameManager.h"
#include "PlayState.h"
#include "PythonManager.h"
#include "CharacterController.h"
#include "PhysicsManager.h"
#include "Raycast.h"

#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include "BtOgreGP.h"

GameActor::GameActor()
{
    m_EntityType = "GameActor";
    entityGroup = 2;
    m_isWieldingWeapon = false;
    m_equippedWeapon = 0;
    isActive = true;
    m_EntityType = "GamePawn";
    entityGroup = 2;
    requestDelete = false;
    m_isInvisible = false;
    m_isQuestActor = false;
    resetStats();
    m_charControl = 0;
    m_physicsBody = 0;

    // TODO: Fix scale
    m_Height = 35;
    m_Radius = 10;
}

GameActor::~GameActor()
{
    delete m_charControl;
}

const Ogre::Quaternion GameActor::getOrientation()
{
    return m_charControl->getOrientation();
}

const Ogre::Vector3 GameActor::getPosition()
{
    return m_charControl->getPosition();
}

void GameActor::setOrientation(const Ogre::Quaternion& quat)
{
    m_charControl->setOrientation(quat);
}

void GameActor::setPosition(const Ogre::Vector3& position)
{
    m_charControl->warp(BtOgre::Convert::toBullet(position));
}

void GameActor::resetStats()
{
    memset(&m_actorStats, 0, sizeof(m_actorStats));
}

bool GameActor::CanSee(GameActor * otherActor)
{
    if(!otherActor)
        return false;

    if(otherActor->m_isInvisible)
        return false;

    Vector3 startPos = getPosition();
    startPos.y += 3;

    Vector3 otherActorEyePos = otherActor->getPosition();
    otherActorEyePos.y -= 5;
    Vector3 thisActorEyePos = getPosition();
    thisActorEyePos.y += 15;

    Vector3 toTargetDir = otherActorEyePos - thisActorEyePos;
    Vector3 direction = toTargetDir;
    float rayLength = direction.normalise();

    if(rayLength < 0)
        rayLength *= -1.0;

    Vector3 start = thisActorEyePos;
    Vector3 end = (rayLength * direction) + start;

    bool gotIt = false;

    NotThisEntityRayResultCallback ray(this);

    PhysicsManager::getSingletonPtr()->getWorld()->rayTest(
        BtOgre::Convert::toBullet(start), BtOgre::Convert::toBullet(end), ray);

    if(ray.hasHit())
    {
        PhysicsEntity* hitEntity = static_cast<PhysicsEntity*>(
            ray.m_collisionObject->getUserPointer());
        if(hitEntity == otherActor)
            gotIt = true;
    }

    return gotIt;
}

void GameActor::equipItem(Item * theItem)
{
    if(theItem->m_ItemType == "weapon")
    {
        this->m_equippedWeapon = theItem;
    }
}

void GameActor::equipWeapon(Item * newWeapon)
{
    this->m_equippedWeapon = newWeapon;
}

void GameActor::callQuestFunction(String questFunction)
{
    if(m_quest != 0 && questFunction != "")
    {
        PythonManager::getSingletonPtr()->callQuestFunction(m_quest->m_ScriptFile,questFunction,m_quest);
    }
}

void GameActor::onHurt()
{ 
    if(m_isQuestActor)
    {
        callQuestFunction(onHurtQuestFunction);
    }
}

void GameActor::onDie()
{ 
    if(m_isQuestActor)
    {
        callQuestFunction(onDieQuestFunction);
    }
}

void GameActor::onAggro()
{
    if(m_isQuestActor)
    {
        callQuestFunction(onAggroQuestFunction);
    }
}

void GameActor::onDamage(GameEntity* instigator)
{
    /// for now just do the random damage no matter the source
    float dmg = 4.0f + (rand() % 12);

    setHealth(getHealth() - dmg);

    // signal to quests that the creature has been wounded
    onHurt();
    
    Ogre::LogManager::getSingletonPtr()->logMessage(getName() + " was hit by " +
        instigator->getName() + " for " + Ogre::StringConverter::toString(dmg) + " damage.");
}
