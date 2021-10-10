#ifndef PauseState_H
#define PauseState_H

#include <OgreMath.h>
#include "GameState.h"

using namespace Ogre;


class UiManager;
class GuiPaused;

/**
    Game State class to handle pause mode
*/
class PauseState : public GameState
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
    static PauseState* getInstance() { return &mPauseState; }

    static bool isQuitRequested() { return requestQuit; }

protected:
    PauseState() : m_GUI(NULL), m_menu(NULL), m_prevTimeFactor(0) { }

    /** Game State instance */
    static PauseState mPauseState;

    /* Game State internals */
    UiManager* m_GUI;           /// UI manager
    GuiPaused* m_menu;          /// pause menu
    Real m_prevTimeFactor;      /// time factor (used to pause & unpause the game)
    bool requestUnPause;        /// true if end of state
    static bool requestQuit;    /// true if end of program
};

#endif
