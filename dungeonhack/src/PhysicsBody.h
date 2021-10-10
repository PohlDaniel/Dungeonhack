#ifndef PHYSICSBODY_H
#define PHYSICSBODY_H

#include <Ogre.h>
#include <btBulletDynamicsCommon.h>
#include "BtOgrePG.h"

class VisualRepresentation;

/// List of possible collision shapes to create from a mesh
enum ShapeType
{
    ST_SPHERE,
    ST_BOX,
    ST_TRIMESH,
    ST_CYLINDER,
    ST_CONVEX
};

/**
 * \brief Dynamics information relating the owner to the game world.
 *
 * This class stores pointers to the Bullet body and collision shape to be used
 * in the game's dynamics simulation. Changes to the body are handed off to a
 * MotionState which automatically updates the scene node of the entity for
 * which a given PhysicsBody was created.
 *
 * A number of parameters can be retrieved and set in order to interact with
 * the Bullet body, or the body itself can be retrieved for more complicated
 * tasks. Overall the class is meant to be a loose wrapper providing the most
 * commonly used functionality.
 */
class PhysicsBody
{
public:
    /// Creates a PhysicsBody stored in a VisualRepresentation
    PhysicsBody(const VisualRepresentation& visual, float mass,
            Ogre::Vector3 position, Ogre::Quaternion rotation, ShapeType shape = ST_CONVEX);
    /// Creates a PhysicsBody from an Entity's mesh
    PhysicsBody(Ogre::Entity& displayEntity, float mass, Ogre::Vector3 position,
            Ogre::Quaternion rotation, Ogre::Vector3 scale = Ogre::Vector3::UNIT_SCALE,
            ShapeType shape = ST_CONVEX);
    ~PhysicsBody();

    btRigidBody& getBody(); /// Retrieve the Bullet body

    // Getters
    const btVector3 getBtPosition();
    const btQuaternion getBtOrientation();
    const Ogre::Vector3 getPosition();
    const Ogre::Quaternion getOrientation();
    const Ogre::Vector3 getVelocity();
    const Ogre::Vector3 getGravity();

    // Setters
    void setPosition(Ogre::Vector3 position);
    void setOrientation(Ogre::Quaternion quat);
    void setVelocity(Ogre::Vector3 velocity); /// Returns the linear velocity
    void setGravity(Ogre::Vector3 gravity);

private:
    btRigidBody* m_body;
    btCollisionShape* m_shape;
};
#endif  // PHYSICSBODY_H
