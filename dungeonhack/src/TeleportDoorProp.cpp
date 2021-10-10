#include "DungeonHack.h"
#include "GameObject.h"
#include "GameManager.h"
#include "PlayState.h"

void TeleportDoorProp::Init(Vector3 position, Quaternion orientation)
{
    btTransform xform(BtOgre::Convert::toBullet(orientation),
            BtOgre::Convert::toBullet(position));
    m_physicsBody->getBody().setWorldTransform(xform);
    // Need to update the SceneNode manually, MotionState isn't notified
    m_displayRepresentation->m_displaySceneNode->setPosition(position);
    m_displayRepresentation->m_displaySceneNode->setOrientation(orientation);
    setMass(0.0f);
}

void TeleportDoorProp::enterInterior()
{
    this->m_Location->EnterInterior(m_interiorNum,m_exitNum);
}

void TeleportDoorProp::leaveInterior(int exitID)
{
    this->m_Location->ExitInterior(0,exitID);
}

void TeleportDoorProp::onUse(GameEntity * instigator)
{
    PlayState * theState = static_cast<PlayState *>(GameManager::getSingletonPtr()->getCurrentState());

    if(theState->isOutdoor)
    {
        enterInterior();
    }
    else
    {
        leaveInterior(m_exitNum);
    }
}
