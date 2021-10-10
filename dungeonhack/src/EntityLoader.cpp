
#include "DungeonHack.h"
#include "EntityLoader.h"

#include "PlayState.h"
#include "EntityManager.h"
#include "Player.h"


void EntityLoader::addRequest(EntityLoadingRequest* req)
{
    queue.push_back(*req);
}

void EntityLoader::update()
{
    GamePlayer* player = NULL;
    Weapon* newWeapon = NULL;
    EntityLoadingRequest req;

    if (!queue.empty())
    {
        player = ((PlayState*)(GameManager::getSingletonPtr()->getCurrentState()))->m_Player;

        req = queue.front();
        queue.pop_front();
        switch (req.type)
        {
        case ENTITY_LOAD_WEAPON:
            newWeapon = new Weapon();
            newWeapon->initialize(req.name, req.inventoryIcon, req.weight, req.worth, req.Mesh, req.collisionMesh, req.displayScale, req.collisionScale, req.mass, req.displayOffset, req.collisionOffset, req.centerOfGravity);
            EntityManager::getSingletonPtr()->addLocalEntity(newWeapon);
            newWeapon->initializeWeapon(req.weaponType, req.damage, req.reach, req.curCondition, req.maxCondition);
            player->m_InventoryItems.push_back(newWeapon);
            player->equipItem(newWeapon);
            break;

        default:
            // drop
            break;
        }
    }
}
