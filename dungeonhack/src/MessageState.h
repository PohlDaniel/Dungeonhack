#ifndef MessageState_H
#define MessageState_H

#include <string>
#include <OgreString.h>
#include <OIS/OISEvents.h>
#include "GameState.h"

using namespace std;
using namespace Ogre;


class GuiMessageBox;

class MessageState : public GameState
{
public:
    /*
        Game State events
    */
    void enter();
    void exit();

    void pause();
    void resume();

    void keyClicked(const OIS::KeyEvent &e);
    void keyPressed(const OIS::KeyEvent &e);
    void keyReleased(const OIS::KeyEvent &e);
    bool frameStarted(const Ogre::FrameEvent& evt);
    bool frameEnded(const Ogre::FrameEvent& evt);

    void mouseMoved(const OIS::MouseEvent &e);
    void mouseDragged(const OIS::MouseEvent &e);
    void mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id);
    void mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id);
    
    /*
        Message box contents setup
    */
    void setMessageType(string theMessageType)  { m_messageType = theMessageType; }
    string getMessageType()                     { return m_messageType; }

    void setMessage(string theMessage);
    void setOptions(int numOptions, string theOptions[], String scriptFile = "", String scriptFunction = "");
    
    void setReady()     { m_pushMessage = true; }
    bool hasMessage()   { return m_pushMessage; }

    String getScriptFile()      { return m_scriptFile; }
    String getScriptFunction()  { return m_scriptFunction; }
    bool getDoScriptFile()      { return (m_scriptFile != "") ? true : false; }

    void clearDidReturn()           { m_didJustReturn = false; }
    bool didJustReturn()            { return m_didJustReturn; }

    int getReturnedValue()          { return m_returnedVal; }

    static MessageState* getInstance()  { return &mMessageState; }

protected:
    MessageState();

    static MessageState mMessageState;

    bool m_pushMessage;
    bool m_requestUnPause;
    bool m_didJustReturn;
    int m_returnedVal;

    UiManager* m_GUI;
    GuiMessageBox* m_message;

    Real m_prevTimeFactor;

    string m_messageType;
    String m_scriptFile;
    String m_scriptFunction;
};

#endif
