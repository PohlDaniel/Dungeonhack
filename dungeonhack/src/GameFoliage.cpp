#include "DungeonHack.h"
#include "GameFoliage.h"

#include <iostream>
#include "GameManager.h"
#include "GameState.h"
#include "GameTerrain.h"
#include "TerrainGenerator.h"
#include "PlayState.h"
#include "Sky.h"


Foliage::Foliage()
{
    m_EntityType = "Foliage";
    m_canopyShadowFrustum = 0;
    m_displayRepresentation = new VisualRepresentation(m_EntityType);
    SceneManager * theScene = GameManager::getSingleton().getSceneManager();
    m_displayRepresentation->m_displaySceneNode = theScene->getRootSceneNode()->createChildSceneNode();

    theScene->getRootSceneNode()->removeChild(m_displayRepresentation->m_displaySceneNode);
    m_displayRepresentation->m_displaySceneNode->setVisible(false);
    m_isSceneNodeAttached = false;

    m_updateFrequency = 30;
    m_ticksSinceUpdated = rand()%30;

    entityGroup = 150;

    m_Imposter = 0;
    m_UseImposter = false;

    m_isStatic = false;
    m_hasBeenAddedToStaticMesh = false;

    m_wasDrawnLastFrame = false;
}

Foliage::~Foliage()
{
    SceneManager * theScene = GameManager::getSingleton().getSceneManager();

    if(m_displayRepresentation)
    {
        if(m_displayRepresentation->m_displaySceneNode)
        {
            m_displayRepresentation->m_displaySceneNode->detachAllObjects();
            m_displayRepresentation->m_displaySceneNode->removeAndDestroyAllChildren();
            theScene->destroySceneNode(m_displayRepresentation->m_displaySceneNode->getName());
            m_displayRepresentation->m_displaySceneNode = 0;
        }

        if(m_displayRepresentation->m_displayEntity)
        {
            theScene->destroyEntity(m_displayRepresentation->m_displayEntity->getName());
            m_displayRepresentation->m_displayEntity = 0;
        }

        delete m_displayRepresentation;
        m_displayRepresentation = 0;
    }

    m_doCollision = false;
    m_collisionRadius = 10;
    m_collisionHeight = 200;
}

void Foliage::createCanopyShadow()
{
    m_canopyShadowFrustum = new Ogre::Frustum();
    m_canopyShadowNode = m_displayRepresentation->m_displaySceneNode->createChildSceneNode();
    m_canopyShadowNode->attachObject(m_canopyShadowFrustum);    
}

void Foliage::destroyCanopyShadow()
{
    SceneManager * theScene = GameManager::getSingleton().getSceneManager();
    m_canopyShadowNode->removeAndDestroyAllChildren();
    theScene->destroySceneNode(m_canopyShadowNode->getName());

    delete m_canopyShadowFrustum;
}

bool Foliage::Place(String mesh, Vector3 pos, float BaseScale, TerrainChunk * myTerrain, int foliageSize, float sinkOffset, bool useImposter, string imposterMaterial, float imposterScale, float viewDistance, float imposterViewDistance, bool doCollision, float collisionRadius, float collisionHeight, string collisionMesh)
{
    // Set scale before initializing physics
    float randScale = (rand() % (int)BaseScale);
    Vector3 Scale = Vector3(BaseScale + randScale,BaseScale + randScale,BaseScale
            + randScale);

    initPhysicsEntity(mesh, Scale);
    m_displayMesh = mesh;

    SceneManager * theScene = GameManager::getSingleton().getSceneManager();

    m_imposterMaterial = imposterMaterial;

    m_treeSize = foliageSize;
    mOwnerTerrain = myTerrain;
    m_UseImposter = useImposter;

    setPosition(pos);

    m_doCollision = doCollision;

    //Place tree on ground
    Vector3 Dir = Vector3(0,-1,0);
    Vector3 Start = getPosition();
    Start.y = 100000;

    Ogre::Ray camray(Start,Dir);
    Real rayLength = 200000;

    Vector3 start = camray.getOrigin();
    Vector3 end = camray.getPoint(rayLength);

    /* // TODO FIXME: RAYCAST
    OgreNewt::BasicRaycast* ray = new OgreNewt::BasicRaycast( PhysicsManager::getSingletonPtr()->getWorld(), start, end );

    if (ray->getHitCount() > 0)
    {
        for(int i = 0; i < ray->getHitCount(); i++)
        {
            OgreNewt::BasicRaycast::BasicRaycastInfo info = ray->getInfoAt(i);

            if(info.mBody == myTerrain->m_Body)
            {
                Ogre::Vector3 globalpt = camray.getPoint( (rayLength) * info.mDistance );
                Ogre::Vector3 newPos = getPosition();
                newPos.y = globalpt.y + sinkOffset;
                setPosition(newPos);
            }
        }
    }
    */

    Vector3 OriginPos1 = getOwnerTerrain()->m_terrainPos;
    Vector3 myPos1;
    myPos1.x = getPosition().x - OriginPos1.x;
    myPos1.z = getPosition().z - OriginPos1.z;
    myPos1.y = (getPosition().y - OriginPos1.y);

    //Setup collision scale
    m_collisionRadius = collisionRadius * ((BaseScale * ((randScale + BaseScale) / BaseScale))) / 30.0;
    m_collisionHeight = collisionHeight * ((BaseScale * ((randScale + BaseScale) / BaseScale))) / 30.0;

    Quaternion initial = Quaternion(Degree(rand() % 360), Vector3(0,1,0)) * Quaternion (Radian(1.57079633),Vector3(-1,0,0));
    setPosition(myPos1);
    setOrientation(initial);

    if(m_treeSize == -1)
    {           
        m_maxVisibleDistance = 2600;
    }
    else if(m_treeSize == 0)
    {           
        m_maxVisibleDistance = 5000;
    }
    else if(m_treeSize == 1)
    {
        m_maxVisibleDistance = 6000;
    }
    else if(m_treeSize == 2)
    {
        m_maxVisibleDistance = 9500;
    }
    else if(m_treeSize == 3)
    {
        m_maxVisibleDistance = 15000;
    }
    else if(m_treeSize == 4)
    {
        m_maxVisibleDistance = 25000;
    }
    else if(m_treeSize == 5)
    {
        m_maxVisibleDistance = 30000;
    }

    if(viewDistance != -1) 
        m_maxVisibleDistance = viewDistance;

    if(imposterViewDistance != -1)
    {
        m_imposterViewDistance = imposterViewDistance;
        m_doImposterViewDistance = true;
    }
    else
    {
        m_doImposterViewDistance = false;
    }

    m_imposterScale = (((BaseScale * 30) * imposterScale) * ((randScale + BaseScale) / BaseScale));

    Camera * playerCam = theScene->getCamera("GameCamera");
    if(playerCam && m_treeSize < 6)
    {
        Ogre::Vector3 distToPlayer = playerCam->getDerivedPosition() - getPosition();
        float distance = distToPlayer.normalise();

        if(distance >= m_maxVisibleDistance)
        {
            if(m_UseImposter)
            {
                Vector3 OriginPos = getOwnerTerrain()->m_terrainPos;
                Vector3 myPos;
                myPos.x = getPosition().x - OriginPos.x;
                myPos.z = getPosition().z - OriginPos.z;
                myPos.y = (getPosition().y - OriginPos.y) + (m_imposterScale / 2.0);

                m_Imposter = this->getOwnerTerrain()->getImposterBillboardSet(m_imposterMaterial)->createBillboard(myPos);
                float flipX = 1.0;

                //Flip the X scale at random so not ever tree sprite faces the same way
                if((rand() % 6) <= 3)
                {
                    flipX = -1.0;
                }

                m_Imposter->setDimensions(m_imposterScale * flipX,m_imposterScale);
            }
        }
        else
        {
            m_isStatic = true;

            if(!m_isSceneNodeAttached)
            {
                m_displayRepresentation->m_displayEntity->setNormaliseNormals(true);
                m_displayRepresentation->m_displayEntity->setCastShadows(true);

                m_displayRepresentation->m_displaySceneNode->setVisible(true);
                m_isSceneNodeAttached = true;
                if(getOwnerTerrain() != 0)
                {
                    if(m_displayRepresentation->m_displaySceneNode->isInSceneGraph())
                    {
                        Ogre::Node* parent = m_displayRepresentation
                            ->m_displaySceneNode->getParent();
                        static_cast<SceneNode*>(parent)->removeChild(
                            m_displayRepresentation->m_displaySceneNode);
                    }
                    if (m_displayRepresentation->m_displaySceneNode->getParentSceneNode() == NULL)
                    {
                        getOwnerTerrain()->m_SceneNode->addChild(m_displayRepresentation->m_displaySceneNode);
                    }
                }
            }
        }
    }

    return true;
}

void Foliage::destroy()
{
    SceneManager * theScene = GameManager::getSingleton().getSceneManager();

    if(m_displayRepresentation)
    {
        if(m_displayRepresentation->m_displaySceneNode)
        {
            m_displayRepresentation->m_displaySceneNode->detachAllObjects();
            m_displayRepresentation->m_displaySceneNode->removeAndDestroyAllChildren();
            theScene->destroySceneNode(m_displayRepresentation->m_displaySceneNode->getName());
            m_displayRepresentation->m_displaySceneNode = 0;
        }

        if(m_displayRepresentation->m_displayEntity)
        {
            theScene->destroyEntity(m_displayRepresentation->m_displayEntity->getName());
            m_displayRepresentation->m_displayEntity = 0;
        }

        if(m_Imposter != 0)
        {
            this->getOwnerTerrain()->getImposterBillboardSet(m_imposterMaterial)->removeBillboard(m_Imposter);
            //this->mOwnerTerrain->m_TreeImposters->removeBillboard(m_Imposter);
            m_Imposter = 0;
        }

        delete m_displayRepresentation;
        m_displayRepresentation = 0;
    }
}

void Foliage::Update(float MoveFactor)
{
    SceneManager * theScene = GameManager::getSingleton().getSceneManager();

    bool doThisUpdate = true;

    //Was checking here to see if the object was on the screen before trying to update it, didn't work as most of the far away objects are sprite and they have no bounding box.
    //I can build a custom bounding box for a sprite in time to have as a foliage object member variable to check against.

    /*if(m_ticksSinceUpdated >= m_updateFrequency)
    {
        doThisUpdate = true;

        if(m_isSceneNodeAttached == true && m_updateFrequency >= 10)
        {
            Ogre::Camera * theCam = GameManager::getSingletonPtr()->getCurrentState()->getCamera();

            if(m_displayRepresentation != 0 && theCam != 0)
            {
                if(m_displayRepresentation->m_displaySceneNode != 0)
                {
                    if(theCam->isVisible(m_displayRepresentation->m_displaySceneNode->_getWorldAABB()) == false)
                    {
                        m_ticksSinceUpdated = 0;
                        doThisUpdate = false;
                    }
                }
            }
        }
    }*/

    if(doThisUpdate)
    {
        m_ticksSinceUpdated = 0;
        Camera * playerCam = theScene->getCamera("GameCamera");
        if(playerCam && m_treeSize < 6)
        {
            Ogre::Vector3 distToPlayer = playerCam->getDerivedPosition() - getPosition();
            float distance = distToPlayer.normalise();

            if(distance >= m_maxVisibleDistance)
            {
                m_displayRepresentation->m_displaySceneNode->setVisible(false);
                m_wasDrawnLastFrame = false;

                if(m_isSceneNodeAttached)
                {
                    this->getOwnerTerrain()->m_SceneNode->removeChild(m_displayRepresentation->m_displaySceneNode->getName());

                    m_isSceneNodeAttached = false;
                    m_updateFrequency = 20; //We'll call the update function once every 16 frames
                    m_ticksSinceUpdated = rand()%20; //pick a random number to start at so that every folaige object doesn't update at the same time

                    m_isStatic = false;
                    m_hasBeenAddedToStaticMesh = false;

                    //create sprite imposter
                    if(m_UseImposter)
                    {
                        Vector3 OriginPos = getOwnerTerrain()->m_terrainPos;
                        Vector3 myPos;
                        myPos.x = getPosition().x - OriginPos.x;
                        myPos.z = getPosition().z - OriginPos.z;
                        myPos.y = (getPosition().y - OriginPos.y) + (m_imposterScale / 2.0);

                        m_Imposter = this->getOwnerTerrain()->getImposterBillboardSet(m_imposterMaterial)->createBillboard(myPos);
                        m_Imposter->setDimensions(m_imposterScale,m_imposterScale);
                    }
                }
            }
            else if(m_wasDrawnLastFrame == false)
            {
                m_displayRepresentation->m_displaySceneNode->setVisible(true);
                m_wasDrawnLastFrame = true;

                if(m_displayRepresentation->m_displayEntity == 0)
                {
                    m_displayRepresentation->m_displayEntity = theScene->createEntity(m_displayRepresentation->m_displaySceneNode->getName() + "Tree",m_displayMesh);
                    m_displayRepresentation->m_displaySceneNode->attachObject(m_displayRepresentation->m_displayEntity);
                    m_displayRepresentation->m_displayEntity->setNormaliseNormals(true);
                    m_displayRepresentation->m_displayEntity->setCastShadows(true);
                }

                if(!m_isSceneNodeAttached)
                {
                    if(this->getOwnerTerrain() != 0)
                    {
                        this->getOwnerTerrain()->m_SceneNode->addChild(m_displayRepresentation->m_displaySceneNode);
                    }
                    else
                    {
                        std::cout << "NO TERRAIN" << endl;
                    }
                    m_isSceneNodeAttached = true;
                    m_updateFrequency = 5;
                    m_ticksSinceUpdated = rand()%5;

                    m_isStatic = true;
                    m_hasBeenAddedToStaticMesh = false;

                    if(m_UseImposter && m_Imposter)
                    {
                        this->getOwnerTerrain()->getImposterBillboardSet(m_imposterMaterial)->removeBillboard(m_Imposter);
                        //this->getOwnerTerrain()->m_TreeImposters->removeBillboard(m_Imposter);
                        //delete m_Imposter;
                        m_Imposter = 0;
                        //m_Imposter = this->getOwnerTerrain()->m_TreeImposters->createBillboard(this->m_pos);
                    }
                }
            }

            if(m_Imposter)
            {
                PlayState* theState = dynamic_cast<PlayState*>(GameManager::getSingleton().getCurrentState());
                
                if(theState) {
                    ColourValue newVal = theState->m_sky->m_sunLight->getDiffuseColour();

                    if(newVal.r + theState->m_ambientLightColour.r < 1)
                        newVal.r += theState->m_ambientLightColour.r;
                    else
                        newVal.r = 1.0;

                    if(newVal.g + theState->m_ambientLightColour.g < 1)
                        newVal.g += theState->m_ambientLightColour.g;
                    else
                        newVal.g = 1.0;

                    if(newVal.b + theState->m_ambientLightColour.b < 1)
                        newVal.b += theState->m_ambientLightColour.b;
                    else
                        newVal.b = 1.0;

                    m_Imposter->setColour(ColourValue::White);

                    //Don't draw the imposter if it's too far away from us
                    if(m_doImposterViewDistance == true)
                    {
                        if(distance >= m_imposterViewDistance)
                        {
                            this->getOwnerTerrain()->getImposterBillboardSet(m_imposterMaterial)->removeBillboard(m_Imposter);
                            m_Imposter = 0;
                        }
                    }
                }
            }
            else if(m_doImposterViewDistance)
            {
                if(distance < m_imposterViewDistance && distance > m_maxVisibleDistance)
                {
                    //Create imposter
                    Vector3 OriginPos = getOwnerTerrain()->m_terrainPos;
                    Vector3 myPos;
                    myPos.x = getPosition().x - OriginPos.x;
                    myPos.z = getPosition().z - OriginPos.z;
                    myPos.y = (getPosition().y - OriginPos.y) + (m_imposterScale / 2.0);

                    m_Imposter = this->getOwnerTerrain()->getImposterBillboardSet(m_imposterMaterial)->createBillboard(myPos);
                    m_Imposter->setDimensions(m_imposterScale,m_imposterScale);
                }
            }
        }
    }

    m_ticksSinceUpdated++;
}

//Grass Foliage Class
GrassFoliage::GrassFoliage()
{
    m_canopyShadowFrustum = 0;
    m_displayRepresentation = new VisualRepresentation("GrassFoliage");
    SceneManager * theScene = GameManager::getSingleton().getSceneManager();
    m_displayRepresentation->m_displaySceneNode = theScene->getRootSceneNode()->createChildSceneNode();

    theScene->getRootSceneNode()->removeChild(m_displayRepresentation->m_displaySceneNode);
    m_displayRepresentation->m_displaySceneNode->setVisible(false);
    m_isSceneNodeAttached = false;

    m_updateFrequency = 20;
    m_ticksSinceUpdated = rand()%20;

    entityGroup = 150;
}

GrassFoliage::~GrassFoliage()
{
    SceneManager * theScene = GameManager::getSingleton().getSceneManager();

    if(m_displayRepresentation)
    {
        if(m_displayRepresentation->m_displaySceneNode)
        {
            m_displayRepresentation->m_displaySceneNode->detachAllObjects();
            m_displayRepresentation->m_displaySceneNode->removeAndDestroyAllChildren();
            theScene->destroySceneNode(m_displayRepresentation->m_displaySceneNode->getName());
            m_displayRepresentation->m_displaySceneNode = 0;
        }

        if(m_displayRepresentation->m_displayEntity)
        {
            theScene->destroyEntity(m_displayRepresentation->m_displayEntity->getName());
            m_displayRepresentation->m_displayEntity = 0;
        }

        delete m_displayRepresentation;
        m_displayRepresentation = 0;
    }
}

bool GrassFoliage::Place(String mesh, Vector3 pos, float BaseScale, TerrainChunk * myTerrain, int foliageSize)
{
    SceneManager * theScene = GameManager::getSingleton().getSceneManager();
    m_displayRepresentation->m_displayEntity = theScene->createEntity(m_displayRepresentation->m_displaySceneNode->getName() + "Tree",mesh);
    m_displayRepresentation->m_displaySceneNode->attachObject(m_displayRepresentation->m_displayEntity);
    m_displayRepresentation->m_displayEntity->setNormaliseNormals(true);

    m_treeSize = foliageSize;
    mOwnerTerrain = myTerrain;
    setPosition(pos);

    float randScale = (rand() % (int)BaseScale);
    Vector3 Scale = Vector3(BaseScale + randScale,BaseScale + randScale,BaseScale + randScale);
    m_displayRepresentation->m_displaySceneNode->setScale(Scale);

    Quaternion initial = Quaternion(Degree(rand() % 360), Vector3(0,1,0)) * Quaternion (Radian(1.57079633),Vector3(-1,0,0));
    m_displayRepresentation->m_displaySceneNode->setOrientation(initial);

    //Place tree on ground
    Vector3 Dir = Vector3(0,-1,0);
    Vector3 Start = getPosition();
    Start.y = 100000;

    Ogre::Ray camray(Start,Dir);
    Real rayLength = 200000;

    Vector3 start = camray.getOrigin();
    Vector3 end = camray.getPoint(rayLength);

    /* // TODO FIXME: raycast
    OgreNewt::BasicRaycast* ray = new OgreNewt::BasicRaycast( PhysicsManager::getSingletonPtr()->getWorld(), start, end );

    if (ray->getHitCount() > 0)
    {
        for(int i = 0; i < ray->getHitCount(); i++)
        {
            OgreNewt::BasicRaycast::BasicRaycastInfo info = ray->getInfoAt(i);

            if(info.mBody == myTerrain->m_Body)
            {
                Ogre::Vector3 globalpt = camray.getPoint( (rayLength) * info.mDistance );
            }
        }
    }
    */

    if(m_treeSize == 0)
    {           
        m_maxVisibleDistance = 7000;
    }
    else if(m_treeSize == 1)
    {
        m_maxVisibleDistance = 10000;
    }
    else if(m_treeSize == 2)
    {
        m_maxVisibleDistance = 15000;
    }
    else if(m_treeSize == 3)
    {
        m_maxVisibleDistance = 25000;
    }
    else if(m_treeSize == 4)
    {
        m_maxVisibleDistance = 35000;
    }
    else if(m_treeSize == 5)
    {
        m_maxVisibleDistance = 50000;
    }


    return true;
}
