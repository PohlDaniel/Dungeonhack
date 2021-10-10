#ifndef PHYSICSMANAGER_H
#define PHYSICSMANAGER_H

#include <Ogre.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

#include "BtOgreExtras.h"
#include "CollisionInfo.h"
#include "PhysicsBody.h"
#include <map>


class PhysicsManager : public Ogre::Singleton<PhysicsManager>
{
public:
    PhysicsManager();
    virtual ~PhysicsManager();

    inline btDynamicsWorld* getWorld() const { return m_dynamicsWorld; }

    static PhysicsManager& getSingleton(void);
    static PhysicsManager* getSingletonPtr(void);

    void update();
    void toggleDebugDraw();
    void setGravity(float newgravity);

    btOverlappingPairCache& getOverlappingPairCache();

    /**
     * Returns the cached shape of an entity, or converts a new one
     * Note: Use of this takes away the choice of conversion shape type
     * This is actually a temporary, rough implementation
     */
    btCollisionShape* getCachedShape(Ogre::Entity* entity, const ShapeType shapeType);
    // TODO: implement me, requires counting users (might be able to do this with Ogre shared ptrs)
    //void releaseCachedShape(Ogre::Entity* entity);
    // TODO: Should be private, but there is a scaling bug
    btCollisionShape* convertShape(Ogre::Entity* entity, const ShapeType shape);

    // Physical constants
    float m_gravity;

private:
    btCollisionConfiguration* m_collisionConfiguration;
    btCollisionDispatcher* m_dispatcher;
    btDbvtBroadphase* m_overlappingPairCache;
    btConstraintSolver* m_constraintSolver;
    btDynamicsWorld* m_dynamicsWorld;

    // Currently just kept for memory management
    btOverlappingPairCallback* m_overlappingPairCallback;

    Ogre::Timer m_stepTimer; // Counts time between physics steps
    BtOgre::DebugDrawer* m_debugDrawer; // Draws debug wire frames

    bool m_debugMode; // Debug wireframes drawn when enabled

    std::map<btCollisionObject*, CollisionInfo> m_contacts; // List of current contacts
    std::map<Ogre::Entity*, btCollisionShape*> m_cachedShapes; // Cache of collision shapes

    void checkCollisions();
};

#endif // PHYSICSMANAGER_H
