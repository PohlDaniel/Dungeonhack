#ifndef GameManager_H
#define GameManager_H

#include <string>
#include <vector>
#include <OgreSingleton.h>
#include <OgreSceneManager.h>
#include "InputManager.h"


class GameState;
class EntityManager;
class UiManager;
class GameConfiguration;

class GameManager : public Ogre::FrameListener, public OIS::KeyListener, public OIS::MouseListener, /*public Ogre::KeyListener, public Ogre::MouseListener, public Ogre::MouseMotionListener,*/
    public Ogre::Singleton<GameManager>
{
public:
    GameManager();
    ~GameManager();
    void start(GameState* state);
    void changeState(GameState* state);
    void pushState(GameState* state);
    void popState();

    bool isDebugMode() {return debugMode;}
    void setDebugMode(bool enabled) {debugMode = enabled;}

    Ogre::SceneManager* getSceneManager() { return mSceneManager; }
    Ogre::Camera* getMainCamera() { return mCamera; }
    Ogre::RenderWindow* getMainWindow() { return mRenderWindow; }
    Ogre::Viewport* getMainViewport() { return mViewport; }
    UiManager* getUI();
    GameConfiguration* getConfig();

    GameState* getCurrentState();

    static GameManager& getSingleton(void);
    static GameManager* getSingletonPtr(void);

protected:
    Ogre::Root* mRoot;
    Ogre::SceneManager* mSceneManager;
    Ogre::Camera* mCamera;
    Ogre::RenderWindow* mRenderWindow;
    Ogre::Viewport* mViewport;
    InputManager* mInputManager;
    EntityManager* mEntityManager;
    UiManager* mGUI;
    GameConfiguration* mConfig;

    void setupResources(void);
    bool configure(void);

    bool mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id);
    bool mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id);
    bool mouseMoved(const OIS::MouseEvent &e);

    bool keyPressed(const OIS::KeyEvent &e);
    bool keyReleased(const OIS::KeyEvent &e);

    bool frameStarted(const Ogre::FrameEvent& evt);
    bool frameEnded(const Ogre::FrameEvent& evt);

    bool debugMode;

private:
    std::vector<GameState*> mStates;
};

#endif
