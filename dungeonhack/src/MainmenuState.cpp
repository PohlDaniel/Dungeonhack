#include "DungeonHack.h"
#include "MainmenuState.h"
#include "PlayState.h"
#include "PauseState.h"
#include "SoundManager.h"

using namespace Ogre;


/** Game State instance */
MainmenuState MainmenuState::m_mainmenuState;


/**
    Game State startup
*/
void MainmenuState::enter()
{
    GameManager* gmgr = GameManager::getSingletonPtr();

    // Basic Ogre scene init
    m_root = Root::getSingletonPtr ();
    m_sceneMgr = gmgr->getSceneManager();
    m_camera = gmgr->getMainCamera();
    m_viewport = gmgr->getMainViewport();

    m_GUI = gmgr->getUI();
    setupMenu();
}

/**
    Game State cleanup
*/
void MainmenuState::exit()
{
    // GUI cleanup
    resetMenu();

    // Scene cleanup
    m_sceneMgr->clearScene();
}

// Not applicable
void MainmenuState::pause()
{
}

// Not applicable
void MainmenuState::resume()
{
    /* For test only: normally we never resume to this state */
    MyGUI::LayerManager::getInstance().setSceneManager(m_sceneMgr);
    updateMenuState();
}


/** Keyboard listener */
void MainmenuState::keyPressed(const OIS::KeyEvent &e)
{
    m_GUI->injectKeyPress(e);
}

/** Keyboard listener */
void MainmenuState::keyReleased(const OIS::KeyEvent &e)
{
    m_GUI->injectKeyRelease(e);

    if(e.key == OIS::KC_ESCAPE) {
        m_end = true;
    }
}

/** Mouse listener */
void MainmenuState::mouseMoved(const OIS::MouseEvent &e)
{
    m_GUI->injectMouseMove(e);
}

/** Mouse listener */
void MainmenuState::mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id)
{
    SoundManager *sm = SoundManager::getSingletonPtr();
    sm->manageSound(sm->playSound("Click.wav",0.5,false));
    m_GUI->injectMousePress(e, id);
}

/** Mouse listener */
void MainmenuState::mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id)
{
    m_GUI->injectMouseRelease(e, id);
}

/** Frame listener */
bool MainmenuState::frameStarted(const Ogre::FrameEvent& evt)
{
    m_GUI->injectFrameStarted(evt);

    if(m_end) {
        return false;
    }
    return true;
}

/** Frame listener */
bool MainmenuState::frameEnded(const Ogre::FrameEvent& evt)
{
    updateMenuState();

    if(m_end) {
        return false;
    }
    return true;
}


/**
    Initialize GUI elements according to current submenu activated
*/
void MainmenuState::setupMenu()
{
    assert(m_menuData == NULL);
    switch (m_curMenuState)
    {
    case MENU_MAIN:
        m_menuData = new SubmenuMain(m_GUI);
        break;

    case MENU_CREATE:
        m_menuData = new SubmenuCreate(m_GUI);
        break;

    case MENU_LOAD:
        //GameManager::getSingleton().changeState(IntroState::getInstance());
        GameManager::getSingleton().changeState(PlayState::getInstance());
#if 0
        /* For debug only */
        pushState(PauseState::getInstance());
        m_newMenuState = MENU_MAIN;
#endif
        break;

    default:
        assert(0);
    }
}

/**
    Remove all GUI elements
*/
void MainmenuState::resetMenu()
{
    if (m_menuData)
    {
        delete m_menuData;
        m_menuData = NULL;
    }
}

/**
    Detect if we need a change of submenu and apply switch if necessary
*/
void MainmenuState::updateMenuState()
{
    if (m_newMenuState != m_curMenuState)
    {
        resetMenu();
        m_curMenuState = m_newMenuState;
        setupMenu();
    }
}


/**
    Submenu common destructor
*/
MainmenuState::Submenu::~Submenu()
{
    // Unload layout
    MyGUI::LayoutManager::getInstance().unloadLayout(m_widgets);
    m_widgets.clear();
}

/**
    Main submenu constructor
*/
MainmenuState::SubmenuMain::SubmenuMain(UiManager* gui) : MainmenuState::Submenu(gui)
{
    // Load layout
    m_widgets = MyGUI::LayoutManager::getInstance().load("MainMenu.layout");

    // Set callbacks
    assignButton("ButtonExit", &MainmenuState::SubmenuMain::buttonExitPressed);
    assignButton("ButtonNew", &MainmenuState::SubmenuMain::buttonNewPressed);
    assignButton("ButtonLoad", &MainmenuState::SubmenuMain::buttonLoadPressed);
}

/**
    Create submenu constructor
*/
MainmenuState::SubmenuCreate::SubmenuCreate(UiManager* gui) : MainmenuState::Submenu(gui)
{
    // Load layout
    if (Ogre::Root::getSingletonPtr()->getAutoCreatedWindow()->getWidth() < 1024)
        m_widgets = MyGUI::LayoutManager::getInstance().load("CharOverview02.layout");
    else
        m_widgets = MyGUI::LayoutManager::getInstance().load("CharOverview01.layout");

    // set callbacks
    assignButton("ButtonExit", &MainmenuState::SubmenuCreate::buttonExitPressed);
}


/** Button listener */
void MainmenuState::SubmenuMain::buttonExitPressed(MyGUI::WidgetPtr _sender)
{
    MainmenuState::getInstance()->setEnd(true);
}

/** Button listener */
void MainmenuState::SubmenuMain::buttonNewPressed(MyGUI::WidgetPtr _sender)
{
    MainmenuState::getInstance()->setMenuState(MENU_CREATE);
}

/** Button listener */
void MainmenuState::SubmenuMain::buttonLoadPressed(MyGUI::WidgetPtr _sender)
{
    MainmenuState::getInstance()->setMenuState(MENU_LOAD);
}

/** Button listener */
void MainmenuState::SubmenuCreate::buttonExitPressed(MyGUI::WidgetPtr _sender)
{
    MainmenuState::getInstance()->setMenuState(MENU_MAIN);
}
