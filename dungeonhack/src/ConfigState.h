#ifndef ConfigState_H
#define ConfigState_H

#include <string>
#include <OgreMath.h>
#include "GameState.h"
#include "GameConfig.h"

using namespace std;
using namespace Ogre;


class UiManager;
class GuiConfig;

/**
    Game State class to handle options menu
*/
class ConfigState : public GameState
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
    static ConfigState* getInstance() { return &mConfigState; }

    /* UI notifications */
    void notify(string setting, Variant value);

protected:
    ConfigState() : m_GUI(NULL), m_config(NULL), m_prevTimeFactor(0) { }

    /** Game State instance */
    static ConfigState mConfigState;

    UiManager* m_GUI;           /// UI manager
    GuiConfig* m_config;        /// config menu

    Real m_prevTimeFactor;      /// time factor (used to pause & unpause the game)

    bool requestUnPause;        /// true if end of state
};

#endif
