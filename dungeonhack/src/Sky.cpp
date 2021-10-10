#include "DungeonHack.h"
#include "Sky.h"
#include "EntityManager.h"
#include "GameObject.h"

Sky::Sky()
{
    fogColorImage = new Image();
    fogColorImage->load("fogColor.png","General");

    sunColorImage = new Image();
    sunColorImage->load("sunColor.png","General");
}

void Sky::Init(int iX, int iY, int terrain_tile_size, float center_offset, SceneManager * scene_manager, Camera * camera, MovablePlane * clip_plane)
{
    m_scene_manager = scene_manager;
    m_camera = camera;
    m_clip_plane = clip_plane;

    cloud_scaler = 1.0;

    m_sunLight = scene_manager->createLight("SunLight");
    m_sunLight->setCastShadows(true);
    m_sunLight->setType(Light::LT_DIRECTIONAL);
    m_sunLight->setDirection(Vector3(0.2,-0.8,0));
    m_sunLight->setDiffuseColour(ColourValue(1.0,1.0,0.64,1));

    int pX = iX;
    int pY = iY;

    mSkyDomeEntity = scene_manager->createEntity("SkyDome","SkyDome.mesh");
    mNightDomeEntity = scene_manager->createEntity("NightDome","NightDome.mesh");
    mFogRingEntity = scene_manager->createEntity("FogRingEnt","SkyDome.mesh");
    mCloudDomeEntity = 0;

    mSkyDomeEntity->setMaterialName("SkyMaterial");
    mFogRingEntity->setMaterialName("Sky/FogRing");
    mFogRingEntity->setNormaliseNormals(true);

    mSkyDomeEntity->setCastShadows(false);
    mNightDomeEntity->setCastShadows(false);
    //mCloudDomeEntity->setCastShadows(false);
    mFogRingEntity->setCastShadows(false);

    //mCloudDomeEntity->setRenderQueueGroup(Ogre::RENDER_QUEUE_1);
    mSkyDomeEntity->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_EARLY);
    mNightDomeEntity->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_EARLY);
    mFogRingEntity->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_EARLY);

    mSkyNode = scene_manager->getRootSceneNode()->createChildSceneNode("SkyNode");
    SceneNode * NightNode = mSkyNode->createChildSceneNode("NightNode");
    m_cloud_node = 0;

    //SceneNode * CloudNode = mSkyNode->createChildSceneNode("CloudNode");

    m_SunHolder = mSkyNode->createChildSceneNode("SunOrientation");
    m_SunNode = m_SunHolder->createChildSceneNode("SunNode",Vector3(0,0,0));

    m_MoonHolder = mSkyNode->createChildSceneNode("MoonOrientation");
    m_MoonNode = m_MoonHolder->createChildSceneNode("MoonNode",Vector3(0,0,0));

    SceneNode * fogRing = mSkyNode->createChildSceneNode("FogRing");

    theSun = new GameSun();
    EntityManager::getSingletonPtr()->addGlobalEntity(theSun);
    theSun->Init(Vector3(pX * (64 * terrain_tile_size) + center_offset,10000, pY * (64 * terrain_tile_size) + center_offset),Vector3(10,10,10),"WorkdDamnit");

    mSkyNode->attachObject(mSkyDomeEntity);
    NightNode->attachObject(mNightDomeEntity);
    fogRing->attachObject(mFogRingEntity);

    fogRing->setScale(Vector3(9,6,9) * 1.5);
    mSkyNode->scale(5000.0,5000.0,5000.0);

    skyMat = 0;
    skyMat = (Material*)MaterialManager::getSingletonPtr()->getByName(mSkyDomeEntity->getMesh()->getSubMesh(0)->getMaterialName()).getPointer();

    m_cloudMaterial = 0;
}

void Sky::SetCloudMaterialName(const String & name)
{
    if(name != "")
    {
        if(m_cloud_node == 0)
        {
            m_cloud_node = mSkyNode->createChildSceneNode("CloudNode");
            m_cloud_node->setScale(2.3 * cloud_scaler,0.5,2.3 * cloud_scaler);
        }

        if(mCloudDomeEntity == 0)
        {
            mCloudDomeEntity = m_scene_manager->createEntity("CloudDome","NightDome.mesh");
            mCloudDomeEntity->setCastShadows(false);
            mCloudDomeEntity->setRenderQueueGroup(Ogre::RENDER_QUEUE_1);
            m_cloud_node->attachObject(mCloudDomeEntity);
        }

        mCloudDomeEntity->setMaterialName(name);
        m_cloudMaterial = (Material*)MaterialManager::getSingletonPtr()->getByName(name).getPointer();
    }
    else
    {
        if(m_cloud_node != 0)
        {
            m_cloud_node->removeAndDestroyAllChildren();
            m_cloud_node->detachAllObjects();

            m_scene_manager->destroySceneNode(m_cloud_node->getName());
            m_scene_manager->destroyEntity(mCloudDomeEntity);

            m_cloud_node = 0;
            mCloudDomeEntity = 0;
        }

        m_cloudMaterial = 0;
    }
}

void Sky::Update(Vector3 camera_position, double time_of_day)
{
    mSkyNode->setPosition(camera_position);

    float theCos = Math::Cos(((2 * Math::PI) * (time_of_day / 86400.0)));
    float dayCos = 0.65 * theCos;

    float col = dayCos;
    float ambientCol = col / 1.8;

    if(ambientCol < 0)
        ambientCol = 0;

    col += 0.35;
    if(col <= 0)
        col = 0;
    
    ColourValue fogCol(col,col,col);
    ColourValue sunCol(col,col,col);

    if(fogColorImage != 0 && sunColorImage != 0)
    {
        float fogImageWidth = fogColorImage->getWidth();
        float sunImageWidth = sunColorImage->getWidth();

        int fogColPos = ((time_of_day / 86400.0) * (fogImageWidth - 1));
        int sunColPos = ((time_of_day / 86400.0) * (sunImageWidth - 1));

        if(fogColPos >= fogImageWidth)
        {
            fogColPos = 0;
        }
        if(sunColPos >= sunImageWidth)
        {
            sunColPos = 0;
        }

        fogCol = fogColorImage->getColourAt(fogColPos,1,0);
        sunCol = sunColorImage->getColourAt(sunColPos,1,0);
    }

    m_scene_manager->setFog(FOG_EXP2,fogCol,fog_thickness,8500.0,15000);

    this->theSun->m_Color = sunCol;
    m_sunLight->setDiffuseColour(sunCol);

    //Add some extra light to the ambient light colour to brighten nights
    float nightBrighter = 0.05;
    float ambR = ambientCol + nightBrighter;
    float ambG = ambientCol + nightBrighter + 0.102; //0.17;
    float amgB = ambientCol + nightBrighter + 0.198;

    m_scene_manager->setAmbientLight(ColourValue(ambR,ambG,amgB));
    m_ambientLightColour = ColourValue(ambR,ambG,amgB);

    //Scroll the sky dome material to the right time of day
    if(skyMat != 0)
    {
        skyMat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureScroll((time_of_day / 86400.0) + 0.13,0);
    }
    else
    {
        skyMat = (Material*)MaterialManager::getSingletonPtr()->getByName(mSkyDomeEntity->getMesh()->getSubMesh(0)->getMaterialName()).getPointer();
    }

    //Clouds take on the color of the sun
    if(m_cloudMaterial != 0)
    {
        m_cloudMaterial->setSelfIllumination(sunCol);
    }

    Quaternion sunOrient = m_SunHolder->getOrientation();
    Quaternion moonOrient = m_MoonHolder->getOrientation();

    Vector3 Direction = sunOrient * Vector3(0,0.9,0.1);
    Vector3 MoonDirection = moonOrient * Vector3(0.25,0.75,0.0);

    Ogre::Ray camray(m_SunHolder->_getDerivedPosition(),Direction);
    Real rayLength = 89000;

    Vector3 start = camray.getOrigin();
    Vector3 end = camray.getPoint(rayLength);

    Ogre::Ray moonray(camera_position,MoonDirection);
    Vector3 moon_start = moonray.getOrigin();
    Vector3 moonEnd = moonray.getPoint(rayLength - 10000);

    theSun->setPosition(end);

    Quaternion q(Degree(360) * (time_of_day / 86400.0),Vector3(0,0,1));

    m_SunHolder->setOrientation(q);

    m_sunLight->setDirection(-Direction);
}

void Sky::renderQueueStarted(Ogre::uint8 queue_id, const Ogre::String& invocation, bool& skipThisQueue)
{
    if(queue_id == RENDER_QUEUE_SKIES_EARLY && m_camera != 0 && m_clip_plane != 0)
    {
        if(m_camera->isReflected())
        {
            RenderSystem * rsys = Ogre::Root::getSingletonPtr()->getRenderSystem();
            m_camera->disableCustomNearClipPlane();
            rsys->_setProjectionMatrix(m_camera->getProjectionMatrixRS());
        }
    }
}

void Sky::renderQueueEnded(Ogre::uint8 queue_id, const Ogre::String &invocation, bool &repeatThisQueue)
{
    if(queue_id == RENDER_QUEUE_SKIES_EARLY && m_camera != 0 && m_clip_plane != 0)
    {
        if(m_camera->isReflected())
        {
            RenderSystem * rsys = Ogre::Root::getSingletonPtr()->getRenderSystem();
            m_camera->enableCustomNearClipPlane(m_clip_plane);
            rsys->_setProjectionMatrix(m_camera->getProjectionMatrixRS());
        }
    }
}
