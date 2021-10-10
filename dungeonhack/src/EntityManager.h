#ifndef _ENTITY_MANAGER_H
#define _ENTITY_MANAGER_H

#include "GameObject.h"
#include "Player.h"
#include "GeometryNode.h"
#include "EntityLoader.h"

class EntityManager : public Ogre::Singleton<EntityManager>
{
public:
    EntityManager();
    ~EntityManager();

    void unloadLocalEntities();
    void unloadGlobalEntities();

    /// Add new entity to given list and give it an ID
    void addEntity(GameEntity* newEntity, std::vector<GameEntity*> &theList);
    /// Add new entity which could be anywhere
    void addGlobalEntity(GameEntity* newEntity);
    /// Add new entity near the player
    void addLocalEntity(GameEntity* newEntity);

    /// Add new entity, differed to next frame start
    void enqueueRequest(EntityLoadingRequest* req);

    void updateEntities(std::vector<GameEntity*>& theList, float timeScaler, bool local=false);
    void updateGlobalEntities(float timeScaler);
    void updateLocalEntities(float timeScaler);

    void DeleteEntity(GameEntity* theEntity);

    int getNextEntityID() { return this->m_nextEntityID; }

    void setPlayer(GamePlayer* player) { this->m_Player = player; }
    GamePlayer* getPlayer() { return this->m_Player; }

    void addLight(Light* newLight) { mLocalLightEntities.push_back(newLight); }
    /// Add new scene geometry to the global list
    void addGeometry(GeometryNode* geom) { mLocalGeometry.push_back(geom); }

    static EntityManager* getSingletonPtr(void);
    static EntityManager& getSingleton(void);

    std::vector<GameEntity*>& getLocalEntities() { return mLocalGameEntities; };

protected:
    int m_nextEntityID;

    // Global game entities like the player, sun, etc...
    std::vector<GameEntity*> mGameEntities;

    // Game entities in the current area. They get deleted/unloaded and have
    // their info saved after leaving the current area.
    std::vector<GameEntity*> mLocalGameEntities;
    std::vector<Ogre::Light*> mLocalLightEntities;
    std::vector<GeometryNode*> mLocalGeometry;
    GamePlayer* m_Player;

    // Differed loading support for entities
    EntityLoader loader;
};

#endif // _ENTITY_MANAGER_H
