#ifndef RAYCAST_H
#define RAYCAST_H

#include <btBulletDynamicsCommon.h>


/**
 * Raycast callback that will return the first entity hit
 */
class EntityRayResultCallback : public btCollisionWorld::ClosestRayResultCallback
{
public:
    EntityRayResultCallback()
        : btCollisionWorld::ClosestRayResultCallback(btVector3(0.0, 0.0, 0.0),
            btVector3(0.0, 0.0, 0.0)) {}

    virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult,
            bool normalInWorldSpace)
    {
        if(rayResult.m_collisionObject->getUserPointer() != NULL)
            return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);

        return 1.0; // Keep looking
    }
};

/**
 * Raycast callback that will only return hits on a particular collision object
 */
class SingleObjectRayResultCallback : public btCollisionWorld::ClosestRayResultCallback
{
public:
    SingleObjectRayResultCallback(const btVector3& rayFromWorld, const btVector3& rayToWorld,
        btCollisionObject* object)
        : btCollisionWorld::ClosestRayResultCallback(rayFromWorld, rayToWorld)
    {
        m_object = object;
    }

    virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult,
            bool normalInWorldSpace)
    {
        if(rayResult.m_collisionObject == m_object)
            return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);

        return 1.0; // Keep looking
    }

    btCollisionObject* m_object;
};

/**
 * Raycast callback that will return the first entity hit and will not hit the
 * passed in object.
 */
class NotThisEntityRayResultCallback : public btCollisionWorld::ClosestRayResultCallback
{
public:
    NotThisEntityRayResultCallback(PhysicsEntity* doNotHitThis)
        : btCollisionWorld::ClosestRayResultCallback(btVector3(0.0, 0.0, 0.0),
            btVector3(0.0, 0.0, 0.0))
    {
        m_notThis = doNotHitThis;
    }

    virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult,
            bool normalInWorldSpace)
    {
        if(rayResult.m_collisionObject->getUserPointer() == m_notThis)
            return 1.0; // Keep looking

        return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
    }

    PhysicsEntity* m_notThis;
};

/**
 * Convex cast callback that will return the first entity hit and will not hit the
 * passed in object.
 */
class NotThisEntityConvexResultCallback : public btCollisionWorld::ClosestConvexResultCallback
{
public:
    NotThisEntityConvexResultCallback(PhysicsEntity* doNotHitThis)
        : btCollisionWorld::ClosestConvexResultCallback(btVector3(0.0, 0.0, 0.0),
            btVector3(0.0, 0.0, 0.0))
    {
        m_notThis = doNotHitThis;
    }

    virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& cvxResult,
            bool normalInWorldSpace)
    {
        if(cvxResult.m_hitCollisionObject->getUserPointer() == m_notThis)
            return 1.0; // Keep looking

        return ClosestConvexResultCallback::addSingleResult(cvxResult, normalInWorldSpace);
    }

    PhysicsEntity* m_notThis;
};

/**
 * Convex cast callback that will return all the entities
 */
class EntityConvexMultipleResultCallback : public btCollisionWorld::ConvexResultCallback
{
public:
    EntityConvexMultipleResultCallback()
    {
    }

    btAlignedObjectArray<btVector3>    m_hitsNormalWorld;
    btAlignedObjectArray<btVector3>    m_hitsPointWorld;
    map<const btCollisionObject*, int> m_hitsCollisionObject;

    virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult,
            bool normalInWorldSpace)
    {
        if (!convexResult.m_hitCollisionObject->getUserPointer())
            return 1.0; // Keep looking

        if (m_hitsCollisionObject.find(convexResult.m_hitCollisionObject) != m_hitsCollisionObject.end())
            return 1.0; // Already in list

        m_closestHitFraction = convexResult.m_hitFraction; // Update to get results

        int incr = m_hitsCollisionObject.size();
        m_hitsCollisionObject[convexResult.m_hitCollisionObject] = incr; // Index of associated normal & point
        if (normalInWorldSpace)
        {
            m_hitsNormalWorld.push_back(convexResult.m_hitNormalLocal);
        }
        else
        {
            // need to transform normal into worldspace
            m_hitsNormalWorld.push_back(
                convexResult.m_hitCollisionObject->getWorldTransform().getBasis() * convexResult.m_hitNormalLocal);
        }
        m_hitsPointWorld.push_back(convexResult.m_hitPointLocal);
        return convexResult.m_hitFraction;
    }
};

/**
 * Convex cast callback that will return all the entities hit and will not hit the
 * passed in object.
 */
class NotThisEntityConvexMultipleResultCallback : public EntityConvexMultipleResultCallback
{
public:
    NotThisEntityConvexMultipleResultCallback(PhysicsEntity* doNotHitThis)
    {
        m_notThis = doNotHitThis;
    }

    virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& cvxResult,
            bool normalInWorldSpace)
    {
        if(cvxResult.m_hitCollisionObject->getUserPointer() == m_notThis)
            return 1.0; // Keep looking

        return EntityConvexMultipleResultCallback::addSingleResult(cvxResult, normalInWorldSpace);
    }

    PhysicsEntity* m_notThis;
};

#endif // RAYCAST_H
