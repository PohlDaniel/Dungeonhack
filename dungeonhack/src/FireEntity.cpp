#include "DungeonHack.h"
#include "FireEntity.h"
#include "GameManager.h"
#include "EntityManager.h"


FireEntity::FireEntity()
{
    m_isBurning = false;
    fireEmitter = 0;
    m_yOffset = 0;
    this->m_EntityType = "FireSpot";
    requestDelete = false;
    fireLight = 0;
    m_corona = 0;
}

FireEntity::~FireEntity()
{
    douse();
}

void FireEntity::init(Vector3 pos, string flameEmitter, bool isBurning, float fuel, float theSize, Vector3 lightOffset, bool makeLight)
{
    m_fireEmitterFile = flameEmitter;
    setPosition(pos);
    m_fuel = fuel;
    m_fireSize = theSize;
    m_lightOffset = lightOffset;
    m_makeLight = makeLight;

    if(isBurning)
    {
        ignite();
    }
}

void FireEntity::ignite()
{
    m_isBurning = true;

    Ogre::SceneManager * theScene = GameManager::getSingletonPtr()->getSceneManager();

    fireEmitter = new GameEmitter();

    fireEmitter->m_displayRepresentation->m_displaySceneNode = theScene->getRootSceneNode()->createChildSceneNode();

    fireEmitter->m_Emitter = theScene->createParticleSystem(fireEmitter->m_displayRepresentation->m_displaySceneNode->getName() + "fire", m_fireEmitterFile);
    fireEmitter->m_Emitter->setDefaultNonVisibleUpdateTimeout(1000);

    Ogre::MaterialPtr matPtr = Ogre::MaterialManager::getSingleton().getByName(fireEmitter->m_Emitter->getMaterialName());

    if(matPtr->isLoaded() == false)
    {
        matPtr->load();
    }

    m_fireMaterial = matPtr.get()->clone(fireEmitter->m_displayRepresentation->m_displaySceneNode->getName() + "_fire_cloned");

    if(m_fireMaterial.get()->getBestTechnique() != 0)
    {
        int numFrames = m_fireMaterial.get()->getBestTechnique()->getPass(0)->getTextureUnitState(0)->getNumFrames();
        m_fireMaterial.get()->getBestTechnique()->getPass(0)->getTextureUnitState(0)->setCurrentFrame(rand() % numFrames);

        fireEmitter->m_Emitter->setMaterialName(m_fireMaterial.get()->getName());

        fireEmitter->m_displayRepresentation->m_displaySceneNode->attachObject(fireEmitter->m_Emitter);

        //Set size and fire offset
        fireEmitter->m_Emitter->setDefaultDimensions(fireEmitter->m_Emitter->getDefaultWidth() * m_fireSize,fireEmitter->m_Emitter->getDefaultHeight() * m_fireSize);

        m_yOffset = fireEmitter->m_Emitter->getDefaultHeight() / 3.0f;
    }
    else
    {
        fireEmitter->m_Emitter->setMaterialName(m_fireEmitterFile);
    }

    if(m_makeLight)
    {
        fireLight = theScene->createLight(fireEmitter->m_displayRepresentation->m_displaySceneNode->getName() + "light");

        //the lower the linear attenuate, the larger the radius of the light
        float attenuate = 0.16; //0.4
        fireLight->setAttenuation(1000,1.3,attenuate * 0.000005,0.0002273547 * (attenuate * 1.5));

        fireLight->setDiffuseColour(1.0,0.534611,0.090050);
        fireLight->setDiffuseColour(fireLight->getDiffuseColour() * 0.95);
        fireLight->setSpecularColour(fireLight->getDiffuseColour());

        Vector3 theLightPos = m_lightOffset + getPosition();

        m_fireGameLight = new GameLight();
        EntityManager::getSingletonPtr()->addLocalEntity(m_fireGameLight);
        m_fireGameLight->InitializeLight(fireLight, theLightPos, theScene->getRootSceneNode());
        m_fireGameLight->setDoFlicker(true);

        Vector3 firePos = getPosition();
        firePos.y += m_yOffset / 2.0;

        /*m_corona = static_cast<GameCorona *>( EntityManager::getSingletonPtr()->SpawnLocalEntity("GameCorona",0) );
        m_corona->Init(firePos,Vector3(1.0,1.0,1.0),fireEmitter->m_displayRepresentation->m_displaySceneNode->getName() + "lightcorona");*/
    }
}

void FireEntity::douse()
{
    Ogre::SceneManager * theScene = GameManager::getSingletonPtr()->getSceneManager();

    if(m_isBurning)
    {
        try
        {
            fireEmitter->m_displayRepresentation->m_displaySceneNode->detachAllObjects();

            if(m_makeLight)
            {
                theScene->destroyLight(fireLight->getName());
            }

            theScene->destroyParticleSystem(fireEmitter->m_displayRepresentation->m_displaySceneNode->getName() + "fire");
        }
        catch(Exception e)
        {
            //Error destroying fire info
            std::cout << "Error destroying fire emitter and light!" << std::endl;
        }

        fireEmitter->m_displayRepresentation->m_displaySceneNode->removeAndDestroyAllChildren();

        if(m_corona)
        {
            m_corona->Destroy();
            m_corona = 0;
        }

        if(m_makeLight)
        {
            if(m_fireGameLight)
            {
                m_fireGameLight->Destroy();
                m_fireGameLight = 0;
            }
        }
    }
    m_isBurning = false;
}

void FireEntity::Update(float MoveFactor)
{

    if(fireEmitter != 0)
    {
        Vector3 newPos = getPosition();
        newPos.y += m_yOffset;
        fireEmitter->setPosition(newPos);
    }

    if(m_isBurning)
    {
        if(m_fuel != -1.0)
        {
            m_fuel -= 0.1 * MoveFactor;

            if(m_fuel <= 0)
            {
                this->Destroy();
            }
        }
    }
}
