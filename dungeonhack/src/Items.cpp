#include "DungeonHack.h"
#include "Items.h"
#include "GameManager.h"
#include "Player.h"

Item::Item()
{
    m_Weight = 0;

    m_isLocal = false;
    requestDelete = false;

    m_ItemType = "baseItem";
}

Item::~Item()
{

}

void Item::initialize(string name, string inventoryIcon, float weight, float worth, string Mesh, string collisionMesh, Vector3 displayScale, Vector3 collisionScale, float mass, Vector3 displayOffset, Vector3 collisionOffset, Vector3 centerOfGravity)
{
    initPhysicsEntity(Mesh);
    m_ItemName = name;
    m_Weight = mass;
    m_displayMesh = Mesh;
    m_collisionMesh = collisionMesh;

    m_collisionScale = collisionScale;
    m_collisionOffset = collisionOffset;
    m_collisionCenterOfGravity = centerOfGravity;
    m_collisionMass = mass;

    m_displayScale = displayScale;
    m_displayOffset = displayOffset;

    m_displayIcon = inventoryIcon;
    m_Weight = weight;
    m_Worth = worth;
}

void Item::loadLocalItem(Vector3 worldPos)
{
    setPosition(worldPos);
    setMass(m_Weight);
    m_isLocal = true;
}

void Item::unloadLocalItem()
{
    SceneManager* theScene = GameManager::getSingleton().getSceneManager();

    /// get rid of the physics body first
    delete this->getPhysicsBody();

    m_displayRepresentation->m_displaySceneNode->detachAllObjects();
    theScene->destroySceneNode(m_displayRepresentation->m_displaySceneNode->getName());

    m_displayRepresentation->m_displaySceneNode = 0;

    delete m_displayRepresentation;
    m_displayRepresentation = 0;

    m_isLocal = false;
}

void Item::pickupItem(GameEntity * thefinder)
{
    if(thefinder != 0)
    {
        //If the item exists in the game world, destroy it's physical properties
        if(m_isLocal)
        {
            unloadLocalItem();
        }

        GameActor* finder = static_cast<GameActor*>(thefinder);

        finder->m_InventoryItems.push_back(this);
    }
}

void Item::onUse(GameEntity* theinstigator)
{
    //GameActor* instigator = (GameActor*)theinstigator;

    if(m_isLocal)
    {
        pickupItem(theinstigator);
    }
}

//Weapons
Weapon::Weapon()
{
    m_displayScale = Vector3(1,1,1);
    requestDelete = false;

    m_ItemType = "weapon";
}

Weapon::~Weapon()
{

}

void Weapon::initializeWeapon(int weaponType, float damage, float reach, float curCondition, float maxCondition)
{
    this->m_weaponType = weaponType;
    this->m_weaponDamage = damage;
    this->m_weaponReach = reach;

    this->m_currentCondition = curCondition;
    this->m_maxCondition = maxCondition;
}

void Weapon::onUse(GameEntity* theinstigator)
{
    //GameActor * instigator = (GameActor *)theinstigator;
    if(m_isLocal)
    {
        pickupItem(theinstigator);
    }
}

void Weapon::pickupItem(GameEntity* thefinder)
{
    if(thefinder != 0)
    {
        //If the item exists in the game world, destroy it's physical properties
        if(m_isLocal)
        {
            unloadLocalItem();
        }

        GameActor* finder = static_cast<GameActor*>(thefinder);

        finder->m_InventoryItems.push_back(this);

        //If the instigator is the Player, automatically equip this weapon for testing
        if(finder->m_EntityType == "GamePlayer")
        {
            GamePlayer* player = static_cast<GamePlayer*>(finder);
            player->equipItem(this);
        }
    }
}

//Clothing
Clothing::Clothing()
{
    m_displayScale = Vector3(1,1,1);

    m_ItemType = "clothing";
}

Clothing::~Clothing()
{

}

void Clothing::initializeClothing(int placementLocation)
{
    m_placementLoc = placementLocation;
}

void Clothing::onUse(GameEntity * theinstigator)
{
    GameActor * instigator = (GameActor *)theinstigator;
    if(m_isLocal)
    {
        pickupItem(instigator);
    }
}
