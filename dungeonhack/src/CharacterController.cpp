#include "DungeonHack.h"
#include "CharacterController.h"
#include "PhysicsManager.h"

#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include "BtOgreGP.h"

#include "GameObject.h"

CharacterController::CharacterController(const btVector3& initialPosition, btScalar height,
    btScalar width, GameEntity* entity)
    : btKinematicCharacterController(0, 0, 0)
{
    m_movementFlags = 0;
    m_stepHeight = 7.2f; // TODO Scale: Assume 1/5 of height 36 for now
    m_lastFallSpeed = m_stepHeight;
    m_orientation = btQuaternion::getIdentity();

    btTransform position;
    position.setIdentity();
    position.setOrigin(initialPosition);

    m_ghostObject = new btPairCachingGhostObject();
    m_ghostObject->setWorldTransform(position);
    m_convexShape = new btCapsuleShape(width, height);
    m_ghostObject->setCollisionShape(m_convexShape);
    m_ghostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
    m_ghostObject->setUserPointer(entity);

    m_ghostObject->setWorldTransform(position);
    btDynamicsWorld* world = PhysicsManager::getSingleton().getWorld();
    world->addCollisionObject(m_ghostObject, btBroadphaseProxy::StaticFilter
        | btBroadphaseProxy::DefaultFilter);
    world->addAction(this);
}

CharacterController::~CharacterController()
{
    btDynamicsWorld* world = PhysicsManager::getSingleton().getWorld();
    world->removeCollisionObject(m_ghostObject);
    world->removeAction(this);

    delete m_ghostObject->getCollisionShape();
    delete m_ghostObject;
}

const Ogre::Vector3 CharacterController::getPosition()
{
    return BtOgre::Convert::toOgre(m_currentPosition);
}

void CharacterController::setOrientation(Ogre::Quaternion orientation)
{
    m_orientation = BtOgre::Convert::toBullet(orientation);
}

const Ogre::Quaternion CharacterController::getOrientation()
{
    return BtOgre::Convert::toOgre(m_orientation);
}

bool CharacterController::onGround()
{
    btTransform start, end;
    btVector3 startpos = m_ghostObject->getWorldTransform().getOrigin();
    btVector3 direction(0,-1,0);
    start.setIdentity();
    end.setIdentity();
    btScalar radius, height;
    radius = static_cast<btCapsuleShape*>(m_ghostObject->getCollisionShape())->getRadius();
    height = static_cast<btCapsuleShape*>(m_ghostObject->getCollisionShape())->getHalfHeight();
    btDynamicsWorld* world = PhysicsManager::getSingleton().getWorld();

    start.setOrigin(startpos);
    end.setOrigin(startpos + (direction * (radius + height)));

    btCollisionWorld::ClosestConvexResultCallback callback(btVector3(0.0, 0.0, 0.0),
        btVector3(0.0, 0.0, 0.0));
    callback.m_collisionFilterGroup = m_ghostObject->getBroadphaseHandle()
        ->m_collisionFilterGroup;
    callback.m_collisionFilterMask = m_ghostObject->getBroadphaseHandle()
        ->m_collisionFilterMask;

    m_ghostObject->convexSweepTest(m_convexShape, start, end, callback,
        world->getDispatchInfo().m_allowedCcdPenetration);

    return callback.hasHit();
}



void CharacterController::preStep(btCollisionWorld* collisionWorld)
{
    if(!(MF_NO_COLLIDE & m_movementFlags))
    {
        int numPenetrationLoops = 0;
        m_touchingContact = false;

        while(recoverFromPenetration (collisionWorld))
        {
            numPenetrationLoops++;
            m_touchingContact = true;
            if (numPenetrationLoops > 4)
            {
                break;
            }
        }
    }
    m_currentPosition = m_ghostObject->getWorldTransform().getOrigin();
    m_targetPosition = m_currentPosition;
}

void CharacterController::playerStep(btCollisionWorld* collisionWorld, btScalar dt)
{
    btTransform xform = m_ghostObject->getWorldTransform();

    if(MF_NO_COLLIDE & m_movementFlags)
    {
        m_targetPosition = m_currentPosition + m_walkDirection;
        m_currentPosition = m_targetPosition;
    }
    else
    {
        stepUp(collisionWorld);
        stepForwardAndStrafe(collisionWorld, m_walkDirection);
        stepDown(collisionWorld, dt);
    }
    xform.setOrigin(m_currentPosition);
    m_ghostObject->setWorldTransform(xform);

    // Update owner entity's SceneNode
    Ogre::SceneNode* ownerNode = static_cast<PhysicsEntity*>(m_ghostObject->getUserPointer())
        ->m_displayRepresentation->m_displaySceneNode;
    ownerNode->setPosition(BtOgre::Convert::toOgre(xform.getOrigin()));
    ownerNode->setOrientation(BtOgre::Convert::toOgre(m_orientation));
}

void CharacterController::setMovementFlags(int flags)
{
    if(MF_NO_COLLIDE & flags)
        m_ghostObject->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
    else
        m_ghostObject->setCollisionFlags(0);

    if(MF_FLY & flags)
    {
        m_lastFallSpeed = m_stepHeight; // Bullet currently uses stepHeight for gravity??
        m_stepHeight = 0;
    }
    else
        m_stepHeight = m_lastFallSpeed;

    m_movementFlags = flags;
}

int CharacterController::getMovementFlags()
{
    return m_movementFlags;
}
