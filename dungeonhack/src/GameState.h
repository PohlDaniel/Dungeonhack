// reference info from :  http://www.ogre3d.org/wiki/index.php/Managing_Game_States_with_OGRE


#ifndef GameState_H
#define GameState_H

#include <OISEvents.h>
#include "GameManager.h"


class GameState
{
public:
    virtual void enter() = 0;
    virtual void exit() = 0;

    virtual void pause() = 0;
    virtual void resume() = 0;

    virtual void mouseMoved(const OIS::MouseEvent &e) = 0;
    virtual void mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id) = 0;
    virtual void mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id) = 0;

    virtual void keyPressed(const OIS::KeyEvent &e) = 0;
    virtual void keyReleased(const OIS::KeyEvent &e) = 0;

    virtual bool frameStarted(const Ogre::FrameEvent& evt) = 0;
    virtual bool frameEnded(const Ogre::FrameEvent& evt) = 0;

    void changeState(GameState* state) { GameManager::getSingletonPtr()->changeState(state); }
    void pushState(GameState* state) { GameManager::getSingletonPtr()->pushState(state); }
    void popState() { GameManager::getSingletonPtr()->popState(); }

    Ogre::Root* getRoot() { return m_root; }
    Ogre::SceneManager* getSceneManager() { return m_sceneMgr; }
    Ogre::Viewport* getViewport() { return m_viewport; }
    Ogre::Camera* getCamera() { return m_camera; }

protected:
    GameState() { m_pauseOthers = false; }

    Ogre::Root *m_root;
    Ogre::SceneManager* m_sceneMgr;
    Ogre::Viewport* m_viewport;
    Ogre::Camera* m_camera;

    bool m_pauseOthers;
};

#endif // GameState_H
