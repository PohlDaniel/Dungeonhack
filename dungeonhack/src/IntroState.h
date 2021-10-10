#ifndef IntroState_H
#define IntroState_H

#include <Ogre.h>
#include "GameState.h"
#include "Sky.h"
#include "GameTerrain.h"

class IntroState : public GameState
{
public:
    void enter();
    void exit();

    void pause();
    void resume();

    void keyPressed(const OIS::KeyEvent &e);
    void keyReleased(const OIS::KeyEvent &e);
    void mouseMoved(const OIS::MouseEvent &e);
    void mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id);
    void mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id);

    bool frameStarted(const Ogre::FrameEvent& evt);
    bool frameEnded(const Ogre::FrameEvent& evt);

    static IntroState* getInstance() { return &mIntroState; }
protected:
    IntroState() { }
    void InitLandscape();

    GameTerrain * m_intro_terrain;
    Sky * m_sky;
    bool mExitGame;
    float fog_thickness;
    double time;
private:
    static IntroState mIntroState;
};

#endif
