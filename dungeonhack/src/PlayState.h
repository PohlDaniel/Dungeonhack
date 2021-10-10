#ifndef PlayState_H
#define PlayState_H

#include <string>
#include <OgreSceneManager.h>
#include "GameState.h"

using namespace Ogre;
using namespace std;


class HDRListener;
class Sky;
class GameTerrain;
class WaterPlane;
class GuiHUD;
class GamePlayer;
class QuestManager;
class PythonManager;

class PlayState : public GameState
{
public:
    GamePlayer * m_Player;
    Light * m_playerLight;
    Camera * m_PlayerCam;

    HDRListener *hdrListener;

    void enter();
    void exit();

    void showInventory();
    void closeInventory();

    void pause();
    void resume();

    void keyPressed(const OIS::KeyEvent &e);
    void keyReleased(const OIS::KeyEvent &e);

    bool frameStarted(const Ogre::FrameEvent& evt);
    bool frameEnded(const Ogre::FrameEvent& evt);

    void mouseMoved(const OIS::MouseEvent &e);
    void mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id);
    void mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id);

    void showHUD();
    void hideHUD();
    void switchHUDEquippedWeaponIcon(string newIcon);
    void updateHUD();

    int entNum;
    float timeScaler;

    ColourValue m_ambientLightColour;

    SceneNode * playerNode;
    SceneNode * leftHandNode;
    SceneNode * rightHandNode;

    SceneNode * hitTestNode;
    SceneNode * weaponRayNode;

    AnimationState* mWeaponAnimState;
    Entity *m_SwordEntity;

    GameTerrain* m_WorldTerrain;
    Sky * m_sky;
    WaterPlane * m_water;

    ParticleSystem* torchEmitter;

    Real grassWaveSin;
    bool isOutdoor;

    void startMorning();
    void startNoon();
    void startEvening();
    void startMidnight();

    void playOutdoorMusic();

    Ogre::Vector2 m_cloudScrollVelocity;

    //GUI Overlay setup
    void addStringToHUDInfo(string newString);

    float ambientNoiseRandom;
    float ambientNoiseWait;
    float curAmbientNoiseWait;
    string worldInfo;

    //config variables
    float cfg_fogThickness;

    HDRListener* mHdrListener;

    void returnedFromMessageBox(int returnedVal, string type);

    static PlayState* getInstance() { return &mPlayState; }

protected:
    PlayState() { }

    UiManager* m_GUI;
    GuiHUD* m_hud;

private:
    bool m_showHUD;
    static PlayState mPlayState;

    //Time since we last checked what entities should be updating
    float lastCheckUpdatingTime;
    float timeToCheckUpdating;

    // Hold these pointers just to shut up some warnings on init
    QuestManager* questManager;
    PythonManager* pythonManager;

    void InitOgre();
    void InitLandscape(int x_tile, int y_tile, int exit_marker_num);
    void InitPlayer();
    void clearLandscape();
};

#endif
