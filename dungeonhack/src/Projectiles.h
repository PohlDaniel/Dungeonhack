#ifndef Projectiles_H
#define Projectiles_H

#include <OgreSceneManager.h>
using namespace Ogre;

#include "GameObject.h"


class GameEmitterProjectile : public GameProjectile
{
public:
    virtual void Update(float MoveFactor);
    virtual void Delete() {delete this;};

    virtual void onSpawn();
    virtual void onHit();

    virtual void Init(Vector3 Position,Vector3 Direction, float Speed, Vector3 Size, float Mass, String DisplayMesh);

    Vector3 m_Direction;
    float m_Speed;

    Ogre::ParticleSystem * m_particleEmitter;
    Light * theLight;

	GameEmitterProjectile()
    {
        m_EntityType = "GameEmitterProjectile";
        entityGroup = 20;   //20 for projectiles
        requestDelete = false;

        theLight = 0;

        m_beenAliveTime = 0;
        m_timeToLive = 1500;

        m_particleEmitter = 0;

        m_DisplayScale = Vector3(1,1,1);
        m_CollisionScale = Vector3(1,1,1);
    };

    ~GameEmitterProjectile();
};

#endif
