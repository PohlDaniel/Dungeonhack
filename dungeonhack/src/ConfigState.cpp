#include "DungeonHack.h"
#include "ConfigState.h"
#include "GuiConfig.h"
#include "GameManager.h"
#include "TimeManager.h"
#include "UiManager.h"
#include "SoundManager.h"
#include "GameWorld.h"
#include "PythonManager.h"
#include "PhysicsManager.h"

using namespace Ogre;


/** Game State instance */
ConfigState ConfigState::mConfigState;


/**
    Game State startup
*/
void ConfigState::enter()
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
    m_config = new GuiConfig(m_GUI);
    m_config->setup();
    m_config->update();
    m_config->setCallback(this);

    requestUnPause = false;
}

/**
    Game State cleanup
*/
void ConfigState::exit()
{
    if (m_config)
    {
        delete m_config;
        m_config = NULL;
        m_GUI = NULL;
    }

    // Resume time
    TimeManager::getSingleton().setTimeFactor(m_prevTimeFactor);
}

void ConfigState::pause()
{
    // Not applicable
}

void ConfigState::resume()
{
    // Not applicable
}


/** Keyboard listener */
void ConfigState::keyPressed(const OIS::KeyEvent &e)
{
    m_GUI->injectKeyPress(e);
}

/** Keyboard listener */
void ConfigState::keyReleased(const OIS::KeyEvent &e)
{
    m_GUI->injectKeyRelease(e);

    if (e.key == OIS::KC_ESCAPE)
    {
        requestUnPause = true;
    }
}

/** Mouse listener */
void ConfigState::mouseMoved(const OIS::MouseEvent &e)
{
    m_GUI->injectMouseMove(e);
}

/** Mouse listener */
void ConfigState::mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id)
{
    m_GUI->injectMousePress(e, id);
}

/** Mouse listener */
void ConfigState::mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id)
{
    SoundManager *sm = SoundManager::getSingletonPtr();
    sm->manageSound(sm->playSound("Click.wav",0.5,false));
    m_GUI->injectMouseRelease(e, id);
}

/** Frame listener */
bool ConfigState::frameStarted(const FrameEvent& evt)
{
    m_GUI->injectFrameStarted(evt);
    SoundManager::getSingleton().Update(0);

    if (m_config->getIsResumeRequested()) requestUnPause = true;
    if(requestUnPause)
    {
        popState();
        return true;
    }
    return true;
}

/** Frame listener */
bool ConfigState::frameEnded(const FrameEvent& evt)
{
    return true;
}

/** Config callback */
void ConfigState::notify(string setting, Variant value)
{
    /*
     *  Planned: hold pending changes for validation / cancellation
     */
    GameSoundObject* obj = 0;
    if (!setting.compare("musicVolume"))
    {
        obj = SoundManager::getSingletonPtr()->getMusic();
        if (obj)
        {
            obj->setGain(value.getFloat());
            obj->UpdateVolume();
        }
        GameManager::getSingletonPtr()->getConfig()->setFloat(setting, value.getFloat());
    }
    else if (!setting.compare("ambientVolume"))
    {
        obj = SoundManager::getSingletonPtr()->getOutdoorAmbient();
        if (obj)
        {
            obj->setGain(value.getFloat());
            obj->UpdateVolume();
        }
        GameManager::getSingletonPtr()->getConfig()->setFloat(setting, value.getFloat());
    }
    else if (!setting.compare("dayScale"))
    {
        /* New value will be applied by PlayState on resume */
        GameManager::getSingletonPtr()->getConfig()->setFloat(setting, value.getFloat());
        TimeManager::getSingleton().setDayScale(value.getFloat());
    }
    else if (!setting.compare("gravity"))
    {
        GameManager::getSingletonPtr()->getConfig()->setFloat(setting, value.getFloat());
        PhysicsManager::getSingletonPtr()->setGravity(value.getFloat());
    }
}
