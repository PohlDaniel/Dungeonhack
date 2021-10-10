#include "DungeonHack.h"
#include "BtOgreGP.h"
#include "BtOgrePG.h"

#include "PhysicsBody.h"
#include "PhysicsManager.h"
#include "GameObject.h"

PhysicsBody::PhysicsBody(const VisualRepresentation& visual, float mass,
        Ogre::Vector3 position, Ogre::Quaternion rotation, ShapeType shape)
{
    if(visual.m_displayEntity != 0)
    {
        m_shape = PhysicsManager::getSingletonPtr()
            ->convertShape(visual.m_displayEntity, shape);
    }
    else // Default to 1m^3 sphere
        m_shape = new btSphereShape(18); // TODO: Fix scale

    // Scale the shape to whatever the visual is
    Ogre::Vector3 scale = visual.m_displaySceneNode->getScale();
    m_shape->setLocalScaling(BtOgre::Convert::toBullet(scale));

    // Set up inertia
    btVector3 inertia;
    m_shape->calculateLocalInertia(mass, inertia);

    // Set up MotionState for automatically updating SceneNode
    BtOgre::RigidBodyState* motionState = new BtOgre::RigidBodyState(
        btTransform(BtOgre::Convert::toBullet(rotation),
        BtOgre::Convert::toBullet(position)), 
        visual.m_displaySceneNode);
    
    m_body = new btRigidBody(mass, motionState, m_shape, inertia);
    PhysicsManager::getSingletonPtr()->getWorld()->addRigidBody(m_body);
}

PhysicsBody::PhysicsBody(Ogre::Entity& displayEntity, float mass,
        Ogre::Vector3 position, Ogre::Quaternion rotation, Ogre::Vector3 scale,
        ShapeType shape)
{
    m_shape = PhysicsManager::getSingletonPtr()->getCachedShape(&displayEntity, shape);
    m_shape->setLocalScaling(BtOgre::Convert::toBullet(scale));

    // Set up inertia
    btVector3 inertia;
    m_shape->calculateLocalInertia(mass, inertia);

    // Set up MotionState for automatically updating SceneNode
    btDefaultMotionState* motionState = new btDefaultMotionState(
        btTransform(BtOgre::Convert::toBullet(rotation),
        BtOgre::Convert::toBullet(position)));

    m_body = new btRigidBody(mass, motionState, m_shape, inertia);
    PhysicsManager::getSingletonPtr()->getWorld()->addRigidBody(m_body);
}

PhysicsBody::~PhysicsBody()
{
    if(m_body->getMotionState())
        delete m_body->getMotionState();

    PhysicsManager::getSingletonPtr()->getWorld()->removeCollisionObject(m_body);

    delete m_body;
}

btRigidBody& PhysicsBody::getBody()
{
    return *m_body;
}

// Getters
const btVector3 PhysicsBody::getBtPosition()
{
    return m_body->getCenterOfMassPosition();
}

const btQuaternion PhysicsBody::getBtOrientation()
{
    return m_body->getOrientation();
}

const Ogre::Vector3 PhysicsBody::getPosition()
{
    return BtOgre::Convert::toOgre(m_body->getCenterOfMassPosition());
}

const Ogre::Quaternion PhysicsBody::getOrientation()
{
    return BtOgre::Convert::toOgre(m_body->getOrientation());
}

const Ogre::Vector3 PhysicsBody::getVelocity()
{
    return BtOgre::Convert::toOgre(m_body->getLinearVelocity());
}

const Ogre::Vector3 PhysicsBody::getGravity()
{
    return BtOgre::Convert::toOgre(m_body->getGravity());
}

// Setters
void PhysicsBody::setPosition(Ogre::Vector3 position)
{
    btTransform trans = btTransform(m_body->getWorldTransform().getRotation(),
            BtOgre::Convert::toBullet(position));
    m_body->setWorldTransform(trans);
}

void PhysicsBody::setOrientation(Ogre::Quaternion quat)
{
    btTransform trans = btTransform(BtOgre::Convert::toBullet(quat),
            m_body->getWorldTransform().getOrigin());
    m_body->setWorldTransform(trans);
}

void PhysicsBody::setVelocity(Ogre::Vector3 velocity)
{
    m_body->setLinearVelocity(BtOgre::Convert::toBullet(velocity));
}

void PhysicsBody::setGravity(Ogre::Vector3 gravity)
{
    m_body->setGravity(BtOgre::Convert::toBullet(gravity));
}
