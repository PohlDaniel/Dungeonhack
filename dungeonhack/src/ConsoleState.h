#ifndef ConsoleState_H
#define ConsoleState_H

#include <string>
#include <OgreMath.h>
#include "GameState.h"

using namespace std;
using namespace Ogre;


class UiManager;
class GuiConsole;

/**
    Game State class to handle pause mode
*/
class ConsoleState : public GameState
{
public:
    /*
        Game State events
    */
    void enter();
    void exit();

    void pause();
    void resume();

    void keyPressed(const OIS::KeyEvent &e);
    void keyReleased(const OIS::KeyEvent &e);

    bool frameStarted(const Ogre::FrameEvent& evt);
    bool frameEnded(const Ogre::FrameEvent& evt);

    void mouseMoved(const OIS::MouseEvent &e);
    void mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id);
    void mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id);

    /** Game State singleton accessor */
    static ConsoleState* getInstance() { return &mConsoleState; }

    static bool isQuitRequested() { return requestQuit; }

protected:
    ConsoleState() : m_GUI(NULL), m_console(NULL), m_prevTimeFactor(0) { }

    /** Game State instance */
    static ConsoleState mConsoleState;

    UiManager* m_GUI;           /// UI manager
    GuiConsole* m_console;      /// pause menu

    Real m_prevTimeFactor;      /// time factor (used to pause & unpause the game)

    bool requestUnPause;        /// true if end of state
    static bool requestQuit;    /// true if end of program
    string m_script;
};

#endif
