#include "DungeonHack.h"
#include "MessageState.h"
#include "GuiMessagebox.h"
#include "GameManager.h"
#include "TimeManager.h"
#include "UiManager.h"
#include "SoundManager.h"

using namespace Ogre;

MessageState MessageState::mMessageState;


MessageState::MessageState()
{
    m_pushMessage = false;
    m_messageType = "";
    m_requestUnPause = false;
    m_didJustReturn = false;
    m_returnedVal = -1;
}


void MessageState::enter()
{
    m_pushMessage = false;

    m_prevTimeFactor = TimeManager::getSingleton().getTimeFactor();
    TimeManager::getSingleton().setTimeFactor(0.0);

    m_root = Root::getSingletonPtr();

    GameManager* gmgr = GameManager::getSingletonPtr();
    m_sceneMgr = gmgr->getSceneManager();
    m_camera = gmgr->getMainCamera();
    m_viewport = gmgr->getMainViewport();

    if (!m_message)
    {
        m_GUI = GameManager::getSingletonPtr()->getUI();
        m_message = new GuiMessageBox(m_GUI);
    }
    m_message->setup();

    m_returnedVal = -1;
    m_requestUnPause = false;
}


void MessageState::exit()
{
    if (m_message)
    {
        delete m_message;
        m_message = NULL;
        m_GUI = NULL;
    }

    m_didJustReturn = true;
    TimeManager::getSingleton().setTimeFactor(m_prevTimeFactor);
}


void MessageState::pause()
{
}


void MessageState::resume()
{
}


void MessageState::keyPressed(const OIS::KeyEvent &e)
{
    m_GUI->injectKeyPress(e);

    if (e.key == OIS::KC_P)
    {
        m_requestUnPause = true;
    }
    else if(e.key == OIS::KC_ESCAPE)
    {
        m_requestUnPause = true;
    }
}


void MessageState::keyReleased(const OIS::KeyEvent &e)
{
    m_GUI->injectKeyRelease(e);
}


void MessageState::mouseMoved(const OIS::MouseEvent &e)
{
    m_GUI->injectMouseMove(e);
}


void MessageState::mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id)
{
    m_GUI->injectMousePress(e, id);
}


void MessageState::mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id)
{
    SoundManager *sm = SoundManager::getSingletonPtr();
    sm->manageSound(sm->playSound("Click.wav",0.5,false));

    m_GUI->injectMouseRelease(e, id);
}


bool MessageState::frameStarted(const FrameEvent& evt)
{
    m_GUI->injectFrameStarted(evt);

    if (m_message->hasReturned())
    {
        m_returnedVal = m_message->getReturnValue();
        m_requestUnPause = true;
    }
    if(m_requestUnPause)
    {
        popState();
    }

    return true;
}


bool MessageState::frameEnded(const FrameEvent& evt)
{
    return true;
}


void MessageState::setMessage(string theMessage)
{
    if (!m_message)
    {
        m_GUI = GameManager::getSingletonPtr()->getUI();
        m_message = new GuiMessageBox(m_GUI);
    }
    m_message->setMessage(theMessage);
}

void MessageState::setOptions(int numOptions, string theOptions[],
                                  String scriptFile, String scriptFunction)
{
    if (!m_message)
    {
        m_GUI = GameManager::getSingletonPtr()->getUI();
        m_message = new GuiMessageBox(m_GUI);
    }
    m_message->setOptions(numOptions, theOptions);

    m_scriptFile = scriptFile;
    m_scriptFunction = scriptFunction;
}
