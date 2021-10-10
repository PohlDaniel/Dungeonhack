#include "DungeonHack.h"
#include "EntityManager.h"
#include "GameManager.h"
#include "SoundManager.h"

#include "Player.h"
#include "Items.h"
#include "GameLight.h"
#include "Monster.h"
#include "NpcActor.h"
#include "Projectiles.h"
#include "GameFoliage.h"
#include "FireEntity.h"
#include "ScriptEntity.h"
#include "GameActor.h"

using namespace Ogre;


template<> EntityManager* Singleton<EntityManager>::ms_Singleton = 0;

EntityManager::EntityManager()
{
    m_Player = NULL;
    m_nextEntityID = 3; // Why 3?
}

EntityManager::~EntityManager()
{
    unloadLocalEntities();
    unloadGlobalEntities();
}

EntityManager* EntityManager::getSingletonPtr(void)
{
    if( !ms_Singleton ) {
        ms_Singleton = new EntityManager();
    }

    return ms_Singleton;
}

EntityManager& EntityManager::getSingleton(void)
{  
    assert(ms_Singleton);
    return *ms_Singleton;
}

void EntityManager::addGlobalEntity(GameEntity* newEntity)
{
    addEntity(newEntity, mGameEntities);
}

void EntityManager::addLocalEntity(GameEntity* newEntity)
{
    addEntity(newEntity, mLocalGameEntities);
}

void EntityManager::addEntity(GameEntity* newEntity, std::vector<GameEntity*> &theList)
{
    newEntity->m_EntityID = m_nextEntityID++;
    theList.push_back(newEntity);
}

void EntityManager::enqueueRequest(EntityLoadingRequest* req)
{
    loader.addRequest(req);
}

void EntityManager::DeleteEntity(GameEntity * theEntity)
{
    if(theEntity != 0)
    {
        String entityType = theEntity->m_EntityType;

        if(entityType == "GamePlayerActor")
        {
            GamePlayer * t_Entity = (GamePlayer *)theEntity;
            delete t_Entity;
            t_Entity = 0;
        }
        else if(entityType == "GameActor")
        {
            GameActor * t_Entity = (GameActor *)theEntity;
            delete t_Entity;
            t_Entity = 0;
        }
        else if(entityType == "GameItem")
        {
            Item * t_Entity = (Item *)theEntity;

            /*if(t_Entity->m_isLocal)
            {
                t_Entity->unloadLocalItem();
            }*/

            if(t_Entity->m_isLocal)
            {
                delete t_Entity;
                t_Entity = 0;
            }
        }
        else if(entityType == "GameWeapon")
        {
            Weapon * t_Entity = (Weapon *)theEntity;

            if(t_Entity->m_isLocal)
            {
                delete t_Entity;
                t_Entity = 0;
            }
        }
        else if(entityType == "GameLight")
        {
            GameLight * t_Entity = (GameLight *)theEntity;
            delete t_Entity;
            t_Entity = 0;
        }
        else if(entityType == "GameProp")
        {
            GameProp * t_Entity = (GameProp *)theEntity;

            delete t_Entity;
            t_Entity = 0;
        }
        else if(entityType == "DoorProp")
        {
            DoorProp * t_Entity = (DoorProp *)theEntity;

            delete t_Entity;
            t_Entity = 0;
        }
        else if(entityType == "GameCorona")
        {
            GameCorona * t_Entity = (GameCorona*)theEntity;
            delete t_Entity;
            t_Entity = 0;
        }
        else if(entityType == "GameSun")
        {
            GameSun * t_Entity = (GameSun*)theEntity;
            delete t_Entity;
            t_Entity = 0;
        }
        else if(entityType == "MonsterZombie")
        {
            Monster* t_Entity= (Monster*)theEntity;
            delete t_Entity;
            t_Entity = 0;
        }
        else if(entityType == "Monster")
        {
            Monster * t_Entity= (Monster*)theEntity;
            delete t_Entity;
            t_Entity = 0;
        }
        else if(entityType == "NpcActor")
        {
            NpcActor * t_Entity= (NpcActor*)theEntity;
            delete t_Entity;
            t_Entity = 0;
        }
        else if(entityType == "TeleportDoorProp")
        {
            TeleportDoorProp * t_Entity = (TeleportDoorProp*)theEntity;
            delete t_Entity;
            t_Entity = 0;
        }
        else if(entityType == "Foliage")
        {
            Foliage * t_Entity = (Foliage*)theEntity;
            t_Entity->destroy();

            delete t_Entity;
            t_Entity = 0;
        }
        else if(entityType == "GameEmitter")
        {
            GameEmitter * t_Entity= (GameEmitter*)theEntity;
            delete t_Entity;
            t_Entity = 0;
        }
        else if(entityType == "ScriptEntity")
        {
            ScriptEntity * t_Entity = (ScriptEntity*)theEntity;
            delete t_Entity;
            t_Entity = 0;
        }
        else if(entityType == "FireEntity")
        {
            FireEntity * t_Entity= (FireEntity*)theEntity;
            delete t_Entity;
            t_Entity = 0;
        }
        else if(entityType == "GameEmitterProjectile")
        {
            GameEmitterProjectile * t_Entity= (GameEmitterProjectile*)theEntity;
            delete t_Entity;
            t_Entity = 0;
        }
    }
    theEntity = 0;
}

void EntityManager::updateEntities(std::vector<GameEntity*> &theList, float timeScaler, bool local)
{
    std::vector<GameEntity*>::iterator it;

    // Add any pending new entity before updating the whole
    loader.update();

    for (it = theList.begin(); it != theList.end();)
    {
        GameEntity * thisEntity = *it;
        if(thisEntity != 0)
        {
            thisEntity->Update(timeScaler);

            if(local && thisEntity->requestDelete)
            {
                DeleteEntity(thisEntity);
                it = theList.erase(it);
                continue;
            }
        }
        ++it;
    }
}

void EntityManager::updateGlobalEntities(float timeScaler)
{
    updateEntities(this->mGameEntities, timeScaler);
}

void EntityManager::updateLocalEntities(float timeScaler)
{
    updateEntities(this->mLocalGameEntities, timeScaler, true);
}

void EntityManager::unloadLocalEntities()
{
    std::vector<GameEntity*>::iterator it;
    std::vector<Ogre::Light*>::iterator it2;
    std::vector<GeometryNode*>::iterator it3;

    //GameManager::getSingletonPtr()->getSceneManager()->removeAllLights();

    GameEntity * thisEntity;

    for (it = mLocalGameEntities.begin(); it != mLocalGameEntities.end(); ++it)
    {
        thisEntity = *it;
        DeleteEntity(thisEntity);
        thisEntity = 0;
    }
    mLocalGameEntities.clear();

    Ogre::Light * thisLight;
    for (it2 = this->mLocalLightEntities.begin(); it2 != mLocalLightEntities.end(); ++it2)
    {
        thisLight = *it2;
        GameManager::getSingletonPtr()->getSceneManager()->destroyLight(thisLight);
        thisLight = 0;
    }
    mLocalLightEntities.clear();

    GeometryNode* geom;
    for(it3 = mLocalGeometry.begin(); it3 != mLocalGeometry.end(); ++it3)
    {
        geom = *it3;
        delete geom;
    }
    mLocalGeometry.clear();
}

void EntityManager::unloadGlobalEntities()
{
    std::vector<GameEntity*>::iterator it;
    GameEntity * thisEntity;

    for (it = mGameEntities.begin(); it != mGameEntities.end(); ++it)
    {
        thisEntity = *it;
        DeleteEntity(thisEntity);
        thisEntity = 0;
    }
    mGameEntities.clear();
}
