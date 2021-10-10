#include "DungeonHack.h"
#include "GameObject.h"
#include "GameManager.h"

unsigned int VisualRepresentation::nodeCount = 0;

VisualRepresentation::VisualRepresentation(Ogre::String name, Ogre::SceneNode* parentNode)
{
    name = name + StringConverter::toString(nodeCount++);

    if(parentNode == NULL)
    {
        Ogre::SceneManager* sceneMgr = GameManager::getSingletonPtr()
            ->getSceneManager();
        m_displaySceneNode = sceneMgr->getRootSceneNode()
            ->createChildSceneNode(name);
    }
    else
        m_displaySceneNode = parentNode->createChildSceneNode(name);

    m_displayMesh = NULL;
    m_displayEntity = NULL;
}

void VisualRepresentation::loadMesh(String mesh, Vector3 scale, bool generateLod)
{
    Ogre::SceneManager* mSceneMgr = GameManager::getSingletonPtr()->getSceneManager();
    m_displaySceneNode->setScale(scale);

    if(mesh == "")
        return;
    m_displayEntity = mSceneMgr->createEntity( m_displaySceneNode->getName() + "_entity", mesh );
    m_displayEntity->setNormaliseNormals(true);
    m_displaySceneNode->attachObject(m_displayEntity);
}

void VisualRepresentation::unloadMesh()
{
    if(m_displaySceneNode)
    {
        Ogre::SceneManager* mSceneMgr = GameManager::getSingletonPtr()->getSceneManager();
        m_displaySceneNode->detachAllObjects();
        m_displaySceneNode->removeAndDestroyAllChildren();
        mSceneMgr->destroySceneNode(m_displaySceneNode->getName());
    }

    if(m_displayEntity)
    {
        Ogre::SceneManager * mSceneMgr = GameManager::getSingletonPtr()->getSceneManager();
        mSceneMgr->destroyEntity(m_displayEntity);
    }
}

VisualRepresentation::~VisualRepresentation()
{
    unloadMesh();
}
