#include "DungeonHack.h"
#include "ConsoleState.h"
#include "GuiConsole.h"
#include "GameManager.h"
#include "TimeManager.h"
#include "UiManager.h"
#include "SoundManager.h"
#include "GameWorld.h"
#include "PythonManager.h"

using namespace Ogre;


/** Game State instance */
ConsoleState ConsoleState::mConsoleState;

bool ConsoleState::requestQuit = false;


/**
    Game State startup
*/
void ConsoleState::enter()
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
    m_console = new GuiConsole(m_GUI);
    m_console->setup();

    requestUnPause = false;
    requestQuit = false;
    m_script = "";
}

/**
    Game State cleanup
*/
void ConsoleState::exit()
{
    if (m_console)
    {
        delete m_console;
        m_console = NULL;
        m_GUI = NULL;
    }

    // Resume time
    TimeManager::getSingleton().setTimeFactor(m_prevTimeFactor);
}

void ConsoleState::pause()
{
    // Not applicable
}

void ConsoleState::resume()
{
    // Not applicable
}


/** Keyboard listener */
void ConsoleState::keyPressed(const OIS::KeyEvent &e)
{
    m_GUI->injectKeyPress(e);
}

/** Keyboard listener */
void ConsoleState::keyReleased(const OIS::KeyEvent &e)
{
    m_GUI->injectKeyRelease(e);

    if (e.key == OIS::KC_GRAVE || e.key == OIS::KC_ESCAPE)
    {
        requestUnPause = true;
    }
}

/** Mouse listener */
void ConsoleState::mouseMoved(const OIS::MouseEvent &e)
{
    m_GUI->injectMouseMove(e);
}

/** Mouse listener */
void ConsoleState::mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id)
{
    m_GUI->injectMousePress(e, id);
}

/** Mouse listener */
void ConsoleState::mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id)
{
    SoundManager *sm = SoundManager::getSingletonPtr();
    sm->manageSound(sm->playSound("Click.wav",0.5,false));
    m_GUI->injectMouseRelease(e, id);
}

/** Frame listener */
bool ConsoleState::frameStarted(const FrameEvent& evt)
{
    m_GUI->injectFrameStarted(evt);
    SoundManager::getSingleton().Update(0);

    if (m_console->getIsResumeRequested()) requestUnPause = true;
    if (m_console->getIsQuitRequested()) requestQuit = true;
    if(requestQuit || requestUnPause)
    {
        popState();
        return true;
    }

    if (m_console->getScript() != "")
    {
        m_script = m_console->getScript();
        popState();

        PythonManager::getSingletonPtr()->runScript(m_script);
        return true;
    }

    if (m_console->getCommand() != "")
    {
        PythonManager::getSingletonPtr()->runCommand(m_console->getCommand());
        m_console->clearCommand();
    }
    return true;
}

/** Frame listener */
bool ConsoleState::frameEnded(const FrameEvent& evt)
{
    return true;
}
