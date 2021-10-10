#include "DungeonHack.h"
#include <iostream>

#include "GameManager.h"
#include "GameState.h"
#include "SoundManager.h"
#include "EntityManager.h"
#include "UiManager.h"
#include "PhysicsManager.h"
#include "GameConfig.h"


using namespace Ogre;

template<> GameManager* Singleton<GameManager>::ms_Singleton = 0;


GameManager::GameManager()
{
    debugMode = false;
    mRoot = 0;
    mSceneManager = 0;
    mCamera = 0;
    mViewport = 0;
    mInputManager = 0;
    mEntityManager = 0;
    mGUI = 0;
    mConfig = 0;
}

GameManager::~GameManager()
{
    // clean up all the states
    while (!mStates.empty()) {
        mStates.back()->exit();
        mStates.pop_back();
    }

    if (mConfig)
        delete mConfig;

    if (mGUI)
        delete mGUI;

    if (mEntityManager)
        delete mEntityManager;

    if (mInputManager)
        delete mInputManager;

    mSceneManager->clearScene();
    mSceneManager->destroyAllCameras();
    mRoot->getAutoCreatedWindow()->removeAllViewports();

    delete mRoot;
}

void GameManager::start(GameState* state)
{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#ifdef _DEBUG
    mRoot = new Root("win32/plugins_d.cfg");
#else
    mRoot = new Root("win32/plugins_r.cfg");
#endif
#else
    mRoot = new Root("linux/plugins.cfg");
#endif

    setupResources();

    if (!configure()) return;

    MaterialManager::getSingleton().setDefaultTextureFiltering(TFO_ANISOTROPIC);
    MaterialManager::getSingleton().setDefaultAnisotropy(6);
    TextureManager::getSingletonPtr()->setDefaultNumMipmaps(5);

    mRoot->addFrameListener(this);

    mInputManager = InputManager::getSingletonPtr();
    mInputManager->initialise(mRoot->getAutoCreatedWindow());
    mInputManager->addKeyListener( this, "KeyboardListener" );
    mInputManager->addMouseListener( this, "MouseListener" );

    mSceneManager = mRoot->createSceneManager(ST_GENERIC);

    mCamera = mSceneManager->createCamera("GameCamera");
    mCamera->setNearClipDistance(10.0);

    mViewport = mRoot->getAutoCreatedWindow()->addViewport(mCamera);

    mEntityManager = EntityManager::getSingletonPtr();

    PhysicsManager::getSingletonPtr()->setGravity(getConfig()->getFloat("gravity"));

    changeState(state);

    mRoot->startRendering();
}

void GameManager::changeState(GameState* state)
{
    // cleanup the current state
    if ( !mStates.empty() ) {
        mStates.back()->exit();
        mStates.pop_back();
    }

    // store and init the new state
    mStates.push_back(state);
    mStates.back()->enter();
}

void GameManager::pushState(GameState* state)
{
    // pause current state
    if ( !mStates.empty() ) {
        mStates.back()->pause();
    }

    // store and init the new state
    mStates.push_back(state);
    mStates.back()->enter();
}

void GameManager::popState()
{
    // cleanup the current state
    if ( !mStates.empty() ) {
        mStates.back()->exit();
        mStates.pop_back();
    }

    // resume previous state
    if ( !mStates.empty() ) {
        mStates.back()->resume();
    }
}

void GameManager::setupResources(void)
{
    // Load resource paths from config file
    ConfigFile cf;
    cf.load("resources.cfg");

    // Go through all sections & settings in the file
    ConfigFile::SectionIterator seci = cf.getSectionIterator();

    String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        ConfigFile::SettingsMultiMap *settings = seci.getNext();
        ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            try
            {
                typeName = i->first;
                archName = i->second;
                ResourceGroupManager::getSingleton().addResourceLocation(
                    archName, typeName, secName);
            }
            catch(Exception e)
            {
                //TODO: Log a message for failed loading of a resource here
            }
        }
    }
}

bool GameManager::configure(void)
{
    // load config settings from config.xml
    getConfig()->load();

    // load config settings from ogre.cfg
    if (!mRoot->restoreConfig())
    {
        // if there is no config file, show the configuration dialog
        if (!mRoot->showConfigDialog())
        {
            return false;
        }
    }

    // initialise and create a default rendering window
    mRenderWindow = mRoot->initialise(true);

    ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

    return true;
}

//Mouse input
bool GameManager::mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id)
{
    mStates.back()->mousePressed(e, id);
    return(true);
}

bool GameManager::mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id)
{
    mStates.back()->mouseReleased(e, id);
    return(true);
}

bool GameManager::mouseMoved(const OIS::MouseEvent &e)
{
    mStates.back()->mouseMoved(e);
    return(true);
}

//Keyboard Input
bool GameManager::keyPressed(const OIS::KeyEvent &e)
{
    mStates.back()->keyPressed(e);
    return(true);
}

bool GameManager::keyReleased(const OIS::KeyEvent &e)
{
    mStates.back()->keyReleased(e);
    return(true);
}

//Frame events
bool GameManager::frameStarted(const FrameEvent& evt)
{
    //Capture input
    InputManager::getSingletonPtr()->capture();

    // call frameStarted of current state
    SoundManager::getSingletonPtr()->Update(evt.timeSinceLastFrame);
    return mStates.back()->frameStarted(evt);
}

bool GameManager::frameEnded(const FrameEvent& evt)
{
    // call frameEnded of current state
    return mStates.back()->frameEnded(evt);
}

UiManager* GameManager::getUI()
{
    if (!mGUI)
    {
        mGUI = new UiManager(mRenderWindow);
        mGUI->initialize();
    }
    return mGUI;
}

GameConfiguration* GameManager::getConfig()
{
    if (!mConfig)
    {
        mConfig = new GameConfiguration();
        //mConfig->load();
    }
    return mConfig;
}

GameState * GameManager::getCurrentState()
{
    return (mStates.empty()) ? NULL : mStates.back();
}

GameManager* GameManager::getSingletonPtr(void)
{
    return ms_Singleton;
}

GameManager& GameManager::getSingleton(void)
{  
    assert(ms_Singleton);
    return *ms_Singleton;
}
