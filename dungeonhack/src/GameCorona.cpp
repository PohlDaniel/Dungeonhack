#include "DungeonHack.h"
#include "GameObject.h"
#include "GameManager.h"
#include "SoundManager.h"
#include "PhysicsManager.h"


//Sun for the sky
void GameSun::Init(Vector3 Position, Vector3 Scale, String name)
{
    Ogre::SceneManager * theScene = GameManager::getSingletonPtr()->getSceneManager();

    mCoronaBillboardSet = theScene->createBillboardSet(name);
    mCoronaBillboardSet->setMaterialName("Coronas/SunFlare2");
    mCoronaBillboardSet->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE);
    mCoronaBillboardSet->setBounds(AxisAlignedBox(-20000,-20000,-20000,20000,20000,20000),20000);

    mSunBillboardSet = theScene->createBillboardSet(name + "Sun");
    mSunBillboardSet->setMaterialName("Sun");

    m_displayRepresentation->m_displaySceneNode = theScene->getRootSceneNode()->createChildSceneNode();
    m_displayRepresentation->m_displaySceneNode->attachObject(mCoronaBillboardSet);
    m_displayRepresentation->m_displaySceneNode->attachObject(mSunBillboardSet);

    mCoronaBillboard = mCoronaBillboardSet->createBillboard(0,0,0);
    mCoronaBillboard->setColour(ColourValue(1,1,1,0.5));
    mCoronaBillboard->setDimensions(68000 * 2.0,68000 * 2.0);

    //mSunCoronaBillboard = mCoronaBillboardSet->createBillboard(0,0,0);
    //mSunCoronaBillboard->setColour(ColourValue(1,1,1,0.5));
    //mSunCoronaBillboard->setDimensions(68000 * 2.0,68000 * 2.0);

    mSunBillboard = mSunBillboardSet->createBillboard(0,0,0);
    mSunBillboard->setColour(ColourValue(1,1,1,0.6));
    //mSunBillboard->setDimensions(20000,20000);
    mSunBillboard->setDimensions(15000,15000);

    setPosition(Position);
    fadeFactor = 0.0;
    flickerAlpha = 1.0;
    flickerUpdateTime = 0;

    visible = true;
    hideSun = false;

    maxViewDistance = 0;
}

void GameSun::showSun(bool show)
{
    hideSun = !show;

    if(!show)
    {
        m_displayRepresentation->m_displaySceneNode->setVisible(false);
    }
    else
    {
        m_displayRepresentation->m_displaySceneNode->setVisible(true);
    }

}

void GameSun::Update(float MoveFactor)
{
    if(!hideSun)
    {
        Ogre::Camera * theCam = GameManager::getSingletonPtr()->getMainCamera();
        Vector3 Pos = theCam->getDerivedPosition();

        bool canSee;
        if(visible)
        {
            canSee = isVisible();
        }
        else
        {
            canSee = false;
        }

        if(canSee == true)
        {
            if(fadeFactor < 1)
            {
                fadeFactor += 0.05 * MoveFactor;

                if(fadeFactor >= 1)
                {
                    fadeFactor = 1;
                }
            }
        }
        else
        {
            if(fadeFactor > 0)
            {
                fadeFactor -= 0.05 * MoveFactor;

                if(fadeFactor <= 0)
                {
                    fadeFactor = 0;
                }
            }
        }

        Radian corona1Rot = mCoronaBillboard->getRotation();
        //Radian corona2Rot = mSunCoronaBillboard->getRotation();
        Radian newRot(0.0005 * MoveFactor);

        ColourValue sunColour = ColourValue(0.5,0.5,0.5,1.0);
        sunColour = (m_Color * 1.2);

        sunColour.r = (sunColour.r > 1) ? 1.0 : sunColour.r;
        sunColour.g = (sunColour.g > 1) ? 1.0 : sunColour.g;
        sunColour.b = (sunColour.b > 1) ? 1.0 : sunColour.b;

        mCoronaBillboard->setRotation(corona1Rot + newRot);
        //mCoronaBillboard->setColour(ColourValue(1.0,1.0,1.0) * ((fadeFactor)));
        mCoronaBillboard->setColour((sunColour) * ((fadeFactor)));

        //mSunCoronaBillboard->setColour((sunColour) * ((fadeFactor)));
        //mSunCoronaBillboard->setColour(ColourValue(1.0,1.0,1.0) * ((fadeFactor)));
        //mSunCoronaBillboard->setRotation(corona2Rot - newRot);
        //m_displayRepresentation->m_displaySceneNode->setVisible(fadeFactor > 0);

        this->mSunBillboard->setColour(sunColour);
    }
}

//Moon for the sky
void GameMoon::Init(Vector3 Position, Vector3 Scale, String name)
{
    Ogre::SceneManager * theScene = GameManager::getSingletonPtr()->getSceneManager();

    mCoronaBillboardSet = theScene->createBillboardSet(name);
    mCoronaBillboardSet->setMaterialName("Coronas/SunFlare2");
    mCoronaBillboardSet->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE);

    mMoonBillboardSet = theScene->createBillboardSet(name + "Moon");
    mMoonBillboardSet->setMaterialName("Moon");

    m_displayRepresentation->m_displaySceneNode = theScene->getRootSceneNode()->createChildSceneNode();
    m_displayRepresentation->m_displaySceneNode->attachObject(mCoronaBillboardSet);
    m_displayRepresentation->m_displaySceneNode->attachObject(mMoonBillboardSet);

    mCoronaBillboard = mCoronaBillboardSet->createBillboard(0,0,0);
    mCoronaBillboard->setColour(ColourValue(1,1,1,0.6));
    mCoronaBillboard->setDimensions(40000,40000);

    mMoonBillboard = mMoonBillboardSet->createBillboard(0,0,0);
    mMoonBillboard->setColour(ColourValue(1,1,1,0.6));
    mMoonBillboard->setDimensions(20000,20000);

    setPosition(Position);

    fadeFactor = 0.0;
    flickerAlpha = 1.0;
    flickerUpdateTime = 0;

    visible = true;
    hideMoon = false;
}

void GameMoon::showMoon(bool show)
{
    hideMoon = !show;

    if(!show)
    {
        m_displayRepresentation->m_displaySceneNode->setVisible(false);
    }
    else
    {
        m_displayRepresentation->m_displaySceneNode->setVisible(true);
    }

}

void GameMoon::Update(float MoveFactor)
{
    if(!hideMoon)
    {
        Ogre::Camera * theCam = GameManager::getSingletonPtr()->getMainCamera();
        Vector3 Pos = theCam->getDerivedPosition();

        bool canSee;
        if(visible)
        {
            //canSee = isVisible();
            canSee = false;
        }
        else
        {
            canSee = false;
        }

        if(canSee == true)
        {
            if(fadeFactor < 1)
            {
                fadeFactor += 0.05 * MoveFactor;

                if(fadeFactor >= 1)
                {
                    fadeFactor = 1;
                }
            }
        }
        else
        {
            if(fadeFactor > 0)
            {
                fadeFactor -= 0.05 * MoveFactor;

                if(fadeFactor <= 0)
                {
                    fadeFactor = 0;
                }
            }
        }

        Radian corona1Rot = mCoronaBillboard->getRotation();
        Radian newRot(0.0005 * MoveFactor);

        ColourValue moonColour = ColourValue(0.5,0.5,0.5,1.0);
        moonColour = (m_Color * 1.2);

        moonColour.r = (moonColour.r > 1) ? 1.0 : moonColour.r;
        moonColour.g = (moonColour.g > 1) ? 1.0 : moonColour.g;
        moonColour.b = (moonColour.b > 1) ? 1.0 : moonColour.b;

        //mCoronaBillboard->setRotation(corona1Rot + newRot);
        //mCoronaBillboard->setColour(ColourValue(1.0,1.0,1.0) * ((fadeFactor)));
        mCoronaBillboard->setColour((moonColour) * ((fadeFactor)));

        this->mMoonBillboard->setColour(moonColour);
    }
}

//Light flare corona

GameCorona::GameCorona()
{
    m_EntityType = "GameCorona";

    m_Color = ColourValue();
    mCoronaBillboardSet = 0;
    mCoronaBillboard = 0;
    m_displayRepresentation = new VisualRepresentation(m_EntityType);

    requestDelete = false;

    entityGroup = 5;
}

GameCorona::~GameCorona()
{
    Ogre::SceneManager * theScene = GameManager::getSingletonPtr()->getSceneManager();

    m_displayRepresentation->m_displaySceneNode->detachAllObjects();

    theScene->destroyBillboardSet(mCoronaBillboardSet->getName());
    mCoronaBillboardSet = 0;
}

void GameCorona::Init(Vector3 Position, Vector3 Scale, String name)
{
    Ogre::SceneManager * theScene = GameManager::getSingletonPtr()->getSceneManager();
    mCoronaBillboardSet = theScene->createBillboardSet(name);
    mCoronaBillboardSet->setMaterialName("Coronas/Corona1");
    mCoronaBillboardSet->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE);

    m_displayRepresentation->m_displaySceneNode = theScene->getRootSceneNode()->createChildSceneNode();
    m_displayRepresentation->m_displaySceneNode->attachObject(mCoronaBillboardSet);

    mCoronaBillboard = mCoronaBillboardSet->createBillboard(0,0,0);
    mCoronaBillboard->setColour(ColourValue(1,0.5,0.09,1.0));

    //GameAmbientSoundObject * fireSound = static_cast<GameAmbientSoundObject *>( EntityManager::getSingletonPtr()->SpawnLocalEntity("GameAmbientSoundObject",0) );
    //fireSound->Init("Fire.wav",Position,0.28,0.08);
    //SoundManager *sm = SoundManager::getSingletonPtr();
    //sm->manageSound(sm->playPositionedSound("Fire.wav",Position,0.4,400,true));

    setPosition(Position);
    fadeFactor = 0.0;
    flickerAlpha = 1.0;
    flickerUpdateTime = 0;

    maxViewDistance = 1200;
}

void GameCorona::Update(float MoveFactor)
{
    Ogre::Camera * theCam = GameManager::getSingletonPtr()->getMainCamera();
    Vector3 Pos = theCam->getDerivedPosition();

    //Vector3 Dir = Pos - getPosition();
    //float Distance = Dir.normalise();
    //Vector3 theScale(1,1,1);
    //theScale *= (200.0 / (Distance + (Distance /4)));
    //m_displayRepresentation->m_displaySceneNode->setScale(theScale * fadeFactor);

    bool canSee = isVisible();

    if(canSee == true)
    {
        if(fadeFactor < 1)
        {
            fadeFactor += 0.05 * MoveFactor;

            if(fadeFactor >= 1)
            {
                fadeFactor = 1;
            }
        }
    }
    else
    {
        if(fadeFactor > 0)
        {
            fadeFactor -= 0.05 * MoveFactor;

            if(fadeFactor <= 0)
            {
                fadeFactor = 0;
            }
        }
    }

    flickerUpdateTime += 0.3 * MoveFactor;

    if(flickerUpdateTime >= 1.0)
    {
        flickerUpdateTime = 0;
        flickerAlpha = ((rand()% 100) / 200.0) + 0.5;
    }

    mCoronaBillboard->setColour(ColourValue(1,0.5,0.09) * ((fadeFactor * 0.8) * flickerAlpha));
    m_displayRepresentation->m_displaySceneNode->setVisible(fadeFactor > 0);
}

bool GameCorona::isVisible()
{
    Ogre::Camera * theCam = GameManager::getSingletonPtr()->getMainCamera();
    if(theCam == 0)
        return false;

    Vector3 Pos = theCam->getDerivedPosition();

    if(theCam->isVisible(m_displayRepresentation->m_displaySceneNode->_getWorldAABB()))
    {
        Vector3 Dir = Pos - getPosition();
        Real rayLength = Dir.normalise() - 5; //to avoid having the ray hit the light source

        Ogre::Ray camray(Pos,-Dir);

        if(rayLength <= maxViewDistance || maxViewDistance == 0)
        {
            Vector3 start = camray.getOrigin();
            Vector3 end = camray.getPoint(rayLength);

            btCollisionWorld::ClosestRayResultCallback ray(BtOgre::Convert::toBullet(start),BtOgre::Convert::toBullet(end));
            PhysicsManager::getSingletonPtr()->getWorld()->rayTest(BtOgre::Convert::toBullet(start),BtOgre::Convert::toBullet(end),ray);

            if (ray.hasHit())
            {
                return false;
            }
            else
            {
                return true;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}
