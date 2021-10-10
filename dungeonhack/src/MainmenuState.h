/**
    Main menu Game State
*/

#ifndef MAINMENUSTATE_H_
#define MAINMENUSTATE_H_

#include <Ogre.h>
#include <OIS/OISEvents.h>
#include "GameState.h"
#include "UiManager.h"
#include <MyGUI_Button.h>


/**
    Game State class to handle Main Menu mode
*/
class MainmenuState : public GameState
{
public:
    /** Menu states */
    enum { MENU_MAIN=0, MENU_CREATE, MENU_LOAD, MENU_COUNT };

    /* ------------- Game State events --------------------- */
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
    /* ----------------------------------------------------- */

    /** Game State singleton accessor */
    static MainmenuState* getInstance() { return &m_mainmenuState; }

    /* End of state accessors */
    void setEnd(bool end) { m_end = end; }
    bool getEnd() { return m_end; }

    /* Menu state accessors */
    void setMenuState(int newState) { m_newMenuState = newState; };
    int getMenuState() { return m_curMenuState; };

protected:
    /**
        Constructor
    */
    MainmenuState() : m_end(false), m_GUI(NULL), m_curMenuState(0), m_newMenuState(0), m_menuData(NULL) { }

    /**
        Destructor
    */
    ~MainmenuState() { if (m_menuData) delete m_menuData; }

    /** Game State instance */
    static MainmenuState m_mainmenuState;

    /* Game State internals */
    bool m_end;                     /// true if end of program
    UiManager* m_GUI;               /// handle to GUI
    int m_curMenuState;             /// current submenu
    int m_newMenuState;             /// next submenu to activate

    class Submenu;
    Submenu* m_menuData;            /// submenu data used to manage layouts and buttons callbacks

    /**
        Initialize GUI elements according to current submenu activated
    */
    void setupMenu();

    /**
        Remove all GUI elements
    */
    void resetMenu();

    /**
        Detect if we need a change of submenu and apply switch if necessary
    */
    void updateMenuState();


    /* ------------------ Internal classes used to handle submenus ----------------- */
    
    /**
        Base class for submenus
    */
    class Submenu
    {
    public:
        Submenu(UiManager* gui) : m_GUI(gui) { };
        ~Submenu();

    protected:
        UiManager* m_GUI;
        MyGUI::VectorWidgetPtr m_widgets;
    };

    /** Main menu */
    class SubmenuMain : public Submenu
    {
    public:
        SubmenuMain(UiManager* gui);

        void buttonExitPressed(MyGUI::WidgetPtr _sender);
        void buttonNewPressed(MyGUI::WidgetPtr _sender);
        void buttonLoadPressed(MyGUI::WidgetPtr _sender);

    private:
        void assignButton(const char* name, void (SubmenuMain::*func)(MyGUI::WidgetPtr))
        {
            MyGUI::ButtonPtr button = m_GUI->findWidget<MyGUI::Button>(name);
            if (button)
                button->eventMouseButtonClick = MyGUI::newDelegate(this, func);
        }
    };

    /** Create menu */
    class SubmenuCreate : public Submenu
    {
    public:
        SubmenuCreate(UiManager* gui);

        void buttonExitPressed(MyGUI::WidgetPtr _sender);

    private:
        void assignButton(const char* name, void (SubmenuCreate::*func)(MyGUI::WidgetPtr))
        {
            MyGUI::ButtonPtr button = m_GUI->findWidget<MyGUI::Button>(name);
            if (button)
                button->eventMouseButtonClick = MyGUI::newDelegate(this, func);
        }
    };
};

#endif // MAINMENUSTATE_H_
