#include "DungeonHack.h"
#include "PauseState.h"
#include "GuiPaused.h"
#include "GameManager.h"
#include "TimeManager.h"
#include "UiManager.h"
#include "SoundManager.h"
#include "ConfigState.h"

using namespace Ogre;


/** Game State instance */
PauseState PauseState::mPauseState;

bool PauseState::requestQuit = false;


/**
    Game State startup
*/
void PauseState::enter()
{
    GameManager* gmgr = GameManager::getSingletonPtr();

    // Time freeze
    m_prevTimeFactor = TimeManager::getSingleton().getTimeFactor();
    TimeManager::getSingleton().setTimeFactor(0.0);
    
    m_root = Root::getSingletonPtr ();

    m_sceneMgr = gmgr->getSceneManager();
    m_camera = gmgr->getMainCamera();
    m_viewport = gmgr->getMainViewport();

    // Setup menu elements
    m_GUI = GameManager::getSingletonPtr()->getUI();
    m_menu = new GuiPaused(m_GUI);
    m_menu->setup();

    requestUnPause = false;
    requestQuit = false;
}

/**
    Game State cleanup
*/
void PauseState::exit()
{
    if (m_menu)
    {
        delete m_menu;
        m_menu = NULL;
        m_GUI = NULL;
    }

    // Resume time
    TimeManager::getSingleton().setTimeFactor(m_prevTimeFactor);
}

void PauseState::pause()
{
    m_menu->suspend();
}

void PauseState::resume()
{
    m_menu->resume();
}


/** Keyboard listener */
void PauseState::keyPressed(const OIS::KeyEvent &e)
{
    m_GUI->injectKeyPress(e);
}

/** Keyboard listener */
void PauseState::keyReleased(const OIS::KeyEvent &e)
{
    m_GUI->injectKeyRelease(e);

    if (e.key == OIS::KC_P)
    {
        requestUnPause = true;
    }
    if (e.key == OIS::KC_Q)
    {
        requestQuit = true;
    }
    else if(e.key == OIS::KC_ESCAPE)
    {
        requestUnPause = true;
    }
}

/** Mouse listener */
void PauseState::mouseMoved(const OIS::MouseEvent &e)
{
    m_GUI->injectMouseMove(e);
}

/** Mouse listener */
void PauseState::mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id)
{
    SoundManager *sm = SoundManager::getSingletonPtr();
    sm->manageSound(sm->playSound("Click.wav",0.5,false));
    m_GUI->injectMousePress(e, id);
}

/** Mouse listener */
void PauseState::mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id)
{
    m_GUI->injectMouseRelease(e, id);
}

/** Frame listener */
bool PauseState::frameStarted(const FrameEvent& evt)
{
    m_GUI->injectFrameStarted(evt);

    if (m_menu->getIsQuitRequested())   requestQuit = true;
    if (m_menu->getIsResumeRequested()) requestUnPause = true;
    if(requestQuit || requestUnPause)
    {
        popState();
        return true;
    }

    if (m_menu->getIsConfigRequested())
    {
        m_menu->clearConfigRequested();
        pushState(ConfigState::getInstance());
    }
    return true;
}

/** Frame listener */
bool PauseState::frameEnded(const FrameEvent& evt)
{
    return true;
}
