#ifndef GameFoliage_H
#define GameFoliage_H

#include <OgreSceneManager.h>
using namespace Ogre;

#include "GameObject.h"

class TerrainChunk;

class Foliage : public PhysicsEntity
{
public:
    SceneNode * m_canopyShadowNode;
    Frustum * m_canopyShadowFrustum;

    Foliage();
    ~Foliage();

    Quaternion m_rotation;
    Vector3 m_scale;

    void destroy();
    void createCanopyShadow();
    void destroyCanopyShadow();
    virtual bool Place(String mesh, Vector3 pos, float BaseScale, TerrainChunk * myTerrain, int treeSize = 3, float sinkOffset = -15, bool useImposter = false, string imposterMaterial = "", float imposterScale = 1.0, float viewDistance = -1, float imposterViewDistance = -1, bool doCollision = false, float collisionRadius = 10, float collisionHeight = 200, string collisionMesh = "");
    TerrainChunk * getOwnerTerrain() { return mOwnerTerrain; }

    virtual void Update(float MoveFactor);

    int m_ticksSinceUpdated;
    int m_updateFrequency;
    float m_maxVisibleDistance;
    int m_treeSize; //Used for draw distance 0 = tiny, 1 = small, 2= medium, 3 = large
    bool m_wasDrawnLastFrame; //Used for distance culling
    bool m_isSceneNodeAttached;
    TerrainChunk * mOwnerTerrain;
    string m_imposterMaterial;

    string m_colMesh;
    string m_displayMesh;

    float m_imposterScale;

    Billboard * m_Imposter;
    bool m_UseImposter;

    bool m_isStatic;
    bool m_hasBeenAddedToStaticMesh;

    float m_collisionRadius;
    float m_collisionHeight;
    bool m_doCollision;

    bool m_doImposterViewDistance;
    float m_imposterViewDistance;
};

class GrassFoliage : public Foliage
{
public:
    GrassFoliage();
    ~GrassFoliage();
    virtual bool Place(String mesh, Vector3 pos, float BaseScale, TerrainChunk * myTerrain, int treeSize = 3);
};

#endif
