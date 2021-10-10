#ifndef Items_H
#define Items_H

#include <OgreVector3.h>
using namespace Ogre;

#include "GameObject.h"


class Item : public PhysicsEntity
{
public:
    //std::vector<Item*> m_ContainedItems;

    //Display
    String m_displayIcon;
    String m_displayMesh;
    Vector3 m_displayScale;
    Vector3 m_displayOffset;

    float m_collisionMass;
    String m_collisionMesh;
    Vector3 m_collisionScale;
    Vector3 m_collisionOffset;
    Vector3 m_collisionCenterOfGravity;
    
    String m_ItemName;
    String m_ItemType;

    String m_ItemID;

    float m_Worth;
    float m_Weight;

    float m_maxCondition;
    float m_currentCondition;

    bool m_isLocal;

    //When the player trys to pick up this item
    virtual void onUse(GameEntity * instigator);

    virtual void loadLocalItem(Vector3 worldPos);   //Place item in world
    virtual void unloadLocalItem(); //Unload local item, and stop updating
    virtual void pickupItem(GameEntity * instigator);

    virtual void initialize(string name, string displayIcon, float weight, float worth, string Mesh, string CollisionMesh, Vector3 displayScale = Vector3(1,1,1), Vector3 collisionScale = Vector3(1,1,1), float mass = 10.0, Vector3 displayOffset = Vector3(0,0,0), Vector3 collisionOffset = Vector3(0,0,0), Vector3 centerOfGravity = Vector3(0,0,0));

    Item();
    ~Item();

    //virtual void Delete() {delete this;};
};

class Clothing : public Item
{
public:
    int m_placementLoc; //0 head, 1 torso, 2 legs, 3 feet, 4 gloves
    String m_clothingMesh;

    virtual void initializeClothing(int placementLocation);
    virtual void onUse(GameEntity * instigator);

    Clothing();
    ~Clothing();
};

class Weapon : public Item
{
public:
    String m_weaponDisplayMesh;

    Vector3 m_weaponDisplayScale;
    Vector3 m_weaponDisplayOffset;
    Vector3 m_weaponDisplayRotation;

    int m_weaponType;
    float m_weaponDamage;
    float m_weaponReach; //How far to check hit

    virtual void initializeWeapon(int weaponType, float damage, float reach, float curCondition, float maxCondition);
    virtual void pickupItem(GameEntity * instigator);
    virtual void onUse(GameEntity * instigator);

    //virtual void Delete() {delete this;};

    Weapon();
    ~Weapon();
};

#endif
