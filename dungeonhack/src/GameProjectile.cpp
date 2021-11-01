#include "DungeonHack.h"
#include "Projectiles.h"

#include "GameManager.h"
#include "EntityManager.h"
#include "GameObject.h"
#include "SoundManager.h"
#include "FireEntity.h"
#include "PhysicsManager.h"
#include "BtOgreExtras.h"
#include "Raycast.h"
#include "PlayState.h"


void GameProjectile::Init(Vector3 Position, Vector3 Size, float Mass, String DisplayMesh)
{
    initPhysicsEntity(DisplayMesh);
    setPosition(Position);
    onSpawn();
}

void GameProjectile::onSpawn() {}

void GameProjectile::onHit()
{
    requestDelete = true;
}

void GameProjectile::Update(float MoveFactor)
{
    m_beenAliveTime += 1.0 * MoveFactor;

    if(m_beenAliveTime >= m_timeToLive && m_timeToLive != 0)
    {
        requestDelete = true;
    }
}

GameEmitterProjectile::~GameEmitterProjectile()
{
    if(m_particleEmitter)
    {
        m_particleEmitter->clear();
    }

    if(theLight)
    {
        Ogre::SceneManager * mSceneMgr = GameManager::getSingletonPtr()->getSceneManager();
        mSceneMgr->destroyLight(theLight->getName());
    }
}

//Emitter Projectile, for magic
void GameEmitterProjectile::Init(Vector3 Position, Vector3 Direction, float Speed, Vector3 Size, float Mass, String DisplayMesh)
{
    initPhysicsEntity("");
    setPosition(Position);
    setMass(Mass);
    getPhysicsBody()->setGravity(Vector3(0,0,0));
    m_Direction = Direction;
    m_Speed = Speed;
    getPhysicsBody()->getBody().setLinearVelocity(BtOgre::Convert::toBullet(m_Speed * m_Direction)); // TODO: ramp

    m_beenAliveTime = 0;
    m_timeToLive = 1000;

    Ogre::SceneManager * mSceneMgr = GameManager::getSingletonPtr()->getSceneManager();

    m_particleEmitter = mSceneMgr->createParticleSystem(this->m_displayRepresentation->m_displaySceneNode->getName(), DisplayMesh);
    m_displayRepresentation->m_displaySceneNode->attachObject(m_particleEmitter);

    SceneManager * theScene = GameManager::getSingletonPtr()->getSceneManager();

    theLight = theScene->createLight(m_displayRepresentation->m_displaySceneNode->getName() + "light");
    theLight->setAttenuation(1000,1.0,0.4 * 0.000005,0.0002273547 * (0.4 * 1.5));
    theLight->setDiffuseColour(1.0,0.534611,0.090050);
    theLight->setDiffuseColour(theLight->getDiffuseColour() * 0.95);
    theLight->setSpecularColour(theLight->getDiffuseColour());
    m_displayRepresentation->m_displaySceneNode->attachObject(theLight);

    m_particleEmitter->setKeepParticlesInLocalSpace(true);

    onSpawn();
}

void GameEmitterProjectile::Update(float MoveFactor)
{
    m_beenAliveTime += 1.0 * MoveFactor;

    if(m_beenAliveTime >= m_timeToLive && m_timeToLive != 0)
    {
        requestDelete = true;
    }
}

void GameEmitterProjectile::onSpawn()
{
}

void GameEmitterProjectile::onHit()
{
    SoundManager *sm = SoundManager::getSingletonPtr();
    sm->manageSound(sm->playPositionedSound("Explode.wav",
            getPosition(), 1.0, 5000, false));

    isActive = false;

    FireEntity* theFireEntity = new FireEntity();
    EntityManager::getSingletonPtr()->addLocalEntity(theFireEntity);
    theFireEntity->init(getPosition(), "Emmiters/TorchFire", true, 10, 4.0);

    float rayLength = 48;
    Vector3 start = getPosition();
    Vector3 end = (0.5f * rayLength * m_Direction) + start;
    btSphereShape castShape(rayLength);

    NotThisEntityConvexMultipleResultCallback ray(this);
    PhysicsManager::getSingletonPtr()->getWorld()->convexSweepTest(
        &castShape,
        btTransform(btQuaternion(), BtOgre::Convert::toBullet(start)),
        btTransform(btQuaternion(), BtOgre::Convert::toBullet(end)),
        ray);

    //map<const btCollisionObject*, int> checkObjects;
    if (ray.hasHit())
    {
        PlayState* ps = static_cast<PlayState*>(GameManager::getSingletonPtr()->getCurrentState());

        map<const btCollisionObject*, int>::iterator it = ray.m_hitsCollisionObject.begin();
        for (; it != ray.m_hitsCollisionObject.end(); it++)
        {
            PhysicsEntity* entity = static_cast<PhysicsEntity*>((*it).first->getUserPointer());
            assert(entity); // Should be filtered by callback
            entity->onDamage(ps->m_Player);

            /*
                FIXME: damage values from Sword and Fireball should be separate
            */
        }
    }

    requestDelete = true;
}
