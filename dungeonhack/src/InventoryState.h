#ifndef InventoryState_H
#define InventoryState_H

#include <Ogre.h>
#include "GameState.h"
#include <MyGUI.h>
#include "SoundManager.h"


class InventoryState : public GameState
{
public:
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

    void setupGuiEventHandlers();

    //Gui Event Functions
    bool resume(MyGUI::WidgetPtr _sender);
    bool quitGame(MyGUI::WidgetPtr _sender);
    //End Gui Event Functions

    //BasicGuiFrameListener * GuiListener;

    UiManager* mGUI;

    bool requestUnPause;
    bool requestQuit;

    static InventoryState* getInstance() { return &mInventoryState; }
protected:
    InventoryState() { }

private:
    static InventoryState mInventoryState;
};

#endif
