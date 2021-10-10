#ifndef CHARACTERCONTROLLER_H
#define CHARACTERCONTROLLER_H

#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <OgreVector3.h>

enum MovementFlags {
    MF_NO_COLLIDE = 1,   /// Do not react to object collisions
    MF_FLY = 2          /// Do not apply gravity
};

class GameEntity;

/**
 * Class to control movement and physical simulation of characters
 *
 * Derived from Bullet's btKinematicCharacterController, this class provides
 * character movement functionality and proper physics reponses to collisions.
 * This is essentially the equivalent of btRigidBody for characters.
 */
class CharacterController : public btKinematicCharacterController
{
public:
    /**
     * Initialize a new character controller
     * \param initialPosition Starting position of character
     * \param height Character height
     * \param width Character width
     * \param entity The entity to which the btCollisionObject's user pointer points
     */
    CharacterController(const btVector3& initialPosition, btScalar height, btScalar width,
        GameEntity* entity);
    virtual ~CharacterController();

    const Ogre::Vector3 getPosition();
    const Ogre::Quaternion getOrientation();
    // warp() sets the position
    void setOrientation(Ogre::Quaternion orientation);

    /**
     * Tests whether the character is currently on the ground or some solid object
     * or not.
     * \return true if on solid ground, false otherwise
     */
    bool onGround();

    /**
     * This function is very similar to the superclass function (for now)
     * but uses movement flags to disable collisions.
     */
    void preStep(btCollisionWorld* collisionWorld);

    /**
     * This function is very similar to the superclass function (for now)
     * but uses movement flags to disable collisions.
     */
    void playerStep(btCollisionWorld* collisionWorld, btScalar dt);

    virtual void updateAction(btCollisionWorld* collisionWorld, btScalar deltaTime)
    {
        preStep(collisionWorld);
        playerStep(collisionWorld, deltaTime);
    }

    /// Sets the type of movement for this controller
    void setMovementFlags(int flags);
    /// Returns the type of movement this controller is using
    int getMovementFlags();

private:
    int m_movementFlags;
    btScalar m_lastFallSpeed;

    // TODO: Take this into account for movement once we are using more exact shapes
    btQuaternion m_orientation;
};
#endif // CHARACTERCONTROLLER_H
