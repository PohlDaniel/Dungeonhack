#include "DungeonHack.h"

#include <OgreMath.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include "BtOgreGP.h"
#include "BtOgrePG.h"
#include "PhysicsManager.h"
#include "GameManager.h"
#include "GameObject.h"
#include "GameConfig.h"

using namespace Ogre;

template<> PhysicsManager* Ogre::Singleton<PhysicsManager>::ms_Singleton = 0;


/**
    Constructor
*/
PhysicsManager::PhysicsManager()
{
    m_debugMode = false;

    m_collisionConfiguration = new btDefaultCollisionConfiguration();
    m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
    m_overlappingPairCache = new btDbvtBroadphase();
    m_constraintSolver = new btSequentialImpulseConstraintSolver();
    m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_overlappingPairCache, m_constraintSolver, m_collisionConfiguration);
    m_debugDrawer = 0; // Need to wait until Ogre is fully initialized

    // Enable ghost object collision caching
    m_overlappingPairCallback = new btGhostPairCallback();
    m_overlappingPairCache->getOverlappingPairCache()
        ->setInternalGhostPairCallback(m_overlappingPairCallback);
}


/**
    Destructor
*/
PhysicsManager::~PhysicsManager()
{
    if(m_debugDrawer) // This is not always initialized
        delete m_debugDrawer;

    delete m_dynamicsWorld;
    delete m_constraintSolver;
    delete m_overlappingPairCache;
    delete m_dispatcher;
    delete m_collisionConfiguration;
}


/**
    Accessor for the manager
*/
PhysicsManager* PhysicsManager::getSingletonPtr(void)
{
    return ms_Singleton;
}


/**
    Accessor for the manager
*/
PhysicsManager& PhysicsManager::getSingleton(void)
{
    assert(ms_Singleton);
    return *ms_Singleton;
}


/**
    Perform simulation
*/
void PhysicsManager::update()
{
    btScalar timeStep = m_stepTimer.getMilliseconds() / 1000.f;
    int maxSubStepsCap = 20;

    // Compute maxSubSteps
    float maxSubSteps = timeStep / (1/60.f);
    maxSubSteps = Math::Floor(maxSubSteps) + 1; // Always round up

    if(maxSubSteps > maxSubStepsCap)
        maxSubSteps = maxSubStepsCap;

    m_dynamicsWorld->stepSimulation(timeStep, (int)maxSubSteps);
    checkCollisions();
    m_stepTimer.reset();

    if(m_debugDrawer && m_debugMode)
        m_debugDrawer->step();
}


/**
    Enable / disable drawing of Physics debug
*/
void PhysicsManager::toggleDebugDraw()
{
    // Initialize if not already done
    if(!m_debugDrawer)
    {
        Ogre::SceneManager* sceneMgr = GameManager::getSingletonPtr()->getSceneManager();
        m_debugDrawer = new BtOgre::DebugDrawer(sceneMgr->getRootSceneNode(), m_dynamicsWorld);
        m_dynamicsWorld->setDebugDrawer(m_debugDrawer);
    }

    m_debugMode = !m_debugMode; // Invert
}


/**
    Accessor
*/
void PhysicsManager::setGravity(float newgravity)
{
    m_gravity = newgravity;
    m_dynamicsWorld->setGravity(btVector3(0, -m_gravity, 0));
}


/**
    Accessor
*/
btOverlappingPairCache& PhysicsManager::getOverlappingPairCache()
{
    return *m_overlappingPairCache->getOverlappingPairCache();
}


/**
    Detect collision and dispatch collision signals
*/
void PhysicsManager::checkCollisions()
{
    map<btCollisionObject*, CollisionInfo> newContacts;

    /* Browse all collision pairs */
    int numManifolds = m_dispatcher->getNumManifolds();
    for (int i=0; i<numManifolds; i++)
    {
        btPersistentManifold* contactManifold = m_dispatcher->getManifoldByIndexInternal(i);
        btCollisionObject* obA = static_cast<btCollisionObject*>(contactManifold->getBody0()); // collider
        btCollisionObject* obB = static_cast<btCollisionObject*>(contactManifold->getBody1()); // collidee

        /* Check all contacts points */
        int numContacts = contactManifold->getNumContacts();
        for (int j=0;j<numContacts;j++)
        {
            btManifoldPoint& pt = contactManifold->getContactPoint(j);
            if (pt.getDistance() < 1.0f)
            {
                //std::cout << "Contact found" << std::endl;

                const btVector3& ptA = pt.getPositionWorldOnA();
                const btVector3& ptB = pt.getPositionWorldOnB();
                const btVector3& normalOnB = pt.m_normalWorldOnB;

                if (newContacts.find(obA) == newContacts.end())
                {
                    newContacts[obA] = CollisionInfo(obB, ptA, ptB, normalOnB);
                }
                if (newContacts.find(obB) == newContacts.end())
                {
                    newContacts[obB] = CollisionInfo(obA, ptB, ptA, -1.0f * normalOnB);
                }
            }
        }
    }

    /* Check for added contacts ... */
    map<btCollisionObject*, CollisionInfo>::iterator it;
    for (it = newContacts.begin(); it != newContacts.end(); it++)
    {
        if (m_contacts.find((*it).first) == m_contacts.end())
        {
            //std::cout << "Collision detected" << std::endl;

            if ((*it).first->getUserPointer())
            {
                //std::cout << "Is a Physics object" << std::endl;

                PhysicsEntity* p = (PhysicsEntity*) ((*it).first->getUserPointer());
                if (p->m_EntityType == "GameEmitterProjectile"
                 || p->m_EntityType == "GameProjectile")
                {
                    GameProjectile* pp = (GameProjectile*)p;
                    pp->onHit();

                    /*  TODO: Use CollisionInfo data */
                }
            }
        }
        else
        {
            // Remove to filter no more active contacts
            m_contacts.erase((*it).first);
        }
    }

    /* ... and removed contacts */
    for (it = m_contacts.begin(); it != m_contacts.end(); it++)
    {
        //std::cout << "End of collision detected" << std::endl;

        /* TODO: notify releases ? */
    }

    m_contacts.clear();
    m_contacts = newContacts;
}

btCollisionShape* PhysicsManager::getCachedShape(Ogre::Entity* entity,
        const ShapeType shapeType)
{
    std::map<Ogre::Entity*, btCollisionShape*>::iterator it;
    it = m_cachedShapes.find(entity);

    if(it == m_cachedShapes.end()) // Not cached, convert
    {
        btCollisionShape* shape = convertShape(entity, shapeType);
        m_cachedShapes[entity] = shape;
        return shape;
    }
    else // Got cached shape, return
        return it->second;
}

btCollisionShape* PhysicsManager::convertShape(Ogre::Entity* entity,
        const ShapeType shape)
{
    btCollisionShape* collShape;

    // Create shape from mesh
    BtOgre::StaticMeshToShapeConverter converter(entity);
    // TODO: Get cached shapes from PhysicsManager
    switch(shape)
    {
        case ST_SPHERE:
            collShape = converter.createSphere();
            break;
        case ST_BOX:
            collShape = converter.createBox();
            break;
        case ST_TRIMESH:
            collShape = converter.createTrimesh();
            break;
        case ST_CYLINDER:
            collShape = converter.createCylinder();
            break;
        case ST_CONVEX:
            collShape = converter.createConvex();
            break;
    }
    return collShape;
}

