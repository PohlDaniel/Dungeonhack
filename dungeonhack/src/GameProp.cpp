#include "DungeonHack.h"
#include "GameObject.h"
#include "Items.h"
#include "GameManager.h"

void GameProp::onUse(GameEntity * instigator)
{
    /*if(m_collisionObject)
    {
        if(m_collisionObject->m_PhysicsBody)
        {
            Real objectMass;
            Vector3 inertia;
            m_collisionObject->m_PhysicsBody->getMassMatrix(objectMass, inertia);

            if(objectMass < 300)
            {
                Item * thisItemCopy = new Item();
                thisItemCopy->m_collisionMesh = this->m_CollisionMesh;
                thisItemCopy->m_displayMesh = this->m_DisplayMesh;
                thisItemCopy->m_Weight = objectMass / 150.0;
                thisItemCopy->m_Worth = 0;

                GameManager::getSingletonPtr()->m_Player->m_InventoryItems.push_back(thisItemCopy);
                SceneManager * m_SceneMgr = GameManager::getSingletonPtr()->getSceneManager();
                m_SceneMgr->destroyEntity(m_DisplayMesh);

                //thisItemCopy->m_collisionMesh = this->
                //OgreNewt::BasicJoints::Universal* u = new OgreNewt::BasicJoints::Universal(PhysicsManager::getSingletonPtr()->getWorld(),GameManager::getSingletonPtr()->m_Player->m_collisionObject->m_PhysicsBody,m_collisionObject->m_PhysicsBody,Vector3(0,1.0,0),Vector3(10,10,10),Vector3(10,10,10));
                requestDelete = true;
            }
        }
    }*/
}
