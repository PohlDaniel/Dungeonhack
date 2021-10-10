#include "DungeonHack.h"

#include "InventoryState.h"
#include "GameManager.h"

using namespace Ogre;

InventoryState InventoryState::mInventoryState;

void InventoryState::enter()
{
    GameManager* gmgr = GameManager::getSingletonPtr();
    //mInputDevice = InputManager::getSingletonPtr()->getInputDevice();

    m_root = Root::getSingletonPtr();
    m_sceneMgr = gmgr->getSceneManager();
    m_viewport = gmgr->getMainViewport();

    // setup GUI system
    mGUI = gmgr->getUI();
    //GuiListener = GameManager::getSingletonPtr()->GuiListener;

    /*if(mGUIRenderer == 0)
    {
        GameManager::getSingletonPtr()->mGUIRenderer = new CEGUI::OgreCEGUIRenderer(mRoot->getAutoCreatedWindow(), Ogre::RENDER_QUEUE_OVERLAY, false, 3000, mSceneMgr);
        mGUIRenderer = GameManager::getSingletonPtr()->mGUIRenderer;
    }
    else
    {
        //mGUIRenderer->setTargetSceneManager(mSceneMgr);
    }

    if(GuiListener == 0)
    {
        GameManager::getSingletonPtr()->GuiListener = new BasicGuiFrameListener(mRoot->getAutoCreatedWindow(),mViewport->getCamera(), mGUIRenderer);
        GuiListener = GameManager::getSingletonPtr()->GuiListener;
        mRoot->addFrameListener(GuiListener);
    }
    else
    {
        //mRoot->removeFrameListener(GuiListener);
        //delete GuiListener;

        //GameManager::getSingletonPtr()->GuiListener = new BasicGuiFrameListener(mRoot->getAutoCreatedWindow(),mViewport->getCamera(), mGUIRenderer);
        //GuiListener = GameManager::getSingletonPtr()->GuiListener;
        //mRoot->addFrameListener(GuiListener);
    }*/

    //mGUIRenderer = new CEGUI::OgreCEGUIRenderer(mRoot->getAutoCreatedWindow(), Ogre::RENDER_QUEUE_OVERLAY, false, 3000);
    //NOTE!! if you are using the "ST_EXTERIOR_CLOSE" sceneManager you will need to replace this line with the following:
    //mGUIRenderer = new CEGUI::OgreCEGUIRenderer(mWindow, Ogre::RENDER_QUEUE_OVERLAY, false, 3000, mSceneManager);
    //with mSceneManager being a pointer to your scene manager
    assert(mGUI);

/*
    if(mGUISystem == 0)
    {
        mGUISystem = new CEGUI::System(mGUIRenderer);
        CEGUI::SchemeManager::getSingleton().loadScheme((CEGUI::utf8*)"TaharezLook.scheme");
    }
*/

    //mGUISystem = new CEGUI::System(mGUIRenderer);

    //CEGUI::Logger::getSingleton().setLoggingLevel(CEGUI::Informative);

    //CEGUI::SchemeManager::getSingleton().loadScheme((CEGUI::utf8*)"TaharezLook.scheme");
/*
    mGUISystem->setDefaultMouseCursor((CEGUI::utf8*)"TaharezLook", (CEGUI::utf8*)"MouseArrow");
    mGUISystem->setDefaultFont((CEGUI::utf8*)"Tahoma-12");
    mEditorGuiSheet= CEGUI::WindowManager::getSingleton().createWindow((CEGUI::utf8*)"DefaultWindow", (CEGUI::utf8*)"PauseSheet");  
    mGUISystem->setGUISheet(mEditorGuiSheet);
*/

    //GuiListener = new BasicGuiFrameListener(mRoot->getAutoCreatedWindow(),mViewport->getCamera(), mGUIRenderer);
    //mRoot->addFrameListener(GuiListener);

/*  CEGUI::StaticImage* simg = (CEGUI::StaticImage*)CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/StaticImage", (CEGUI::utf8*)"MainMenuBackground");
    mEditorGuiSheet->addChildWindow(simg);
    simg->setMaximumSize(CEGUI::Size(1.0f, 0.7f));
    simg->setPosition(CEGUI::Point(0.32f, 0.19f));  //simg->setPosition(CEGUI::Point(0.29f, 0.19f));
    simg->setSize(CEGUI::Size(0.48f * menuScale, 0.7f * menuScale));
    simg->setFrameEnabled(false);
    simg->setBackgroundEnabled(false);
    simg->setImage((CEGUI::utf8*)"DHLook", (CEGUI::utf8*)"MainMenu");

    CEGUI::PushButton* resumeButton = (CEGUI::PushButton*)CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/Button", (CEGUI::utf8*)"Resume");
    simg->addChildWindow(resumeButton);
    resumeButton->setPosition(CEGUI::Point(0.19f, 0.3f));
    resumeButton->setSize(CEGUI::Size(0.45f, 0.065f));
    resumeButton->setText("Resume");

    CEGUI::PushButton* quitButton = (CEGUI::PushButton*)CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/Button", (CEGUI::utf8*)"Quit");
    simg->addChildWindow(quitButton);
    quitButton->setPosition(CEGUI::Point(0.19f, 0.38f));
    quitButton->setSize(CEGUI::Size(0.45f, 0.065f));
    quitButton->setText("Exit Game");

    requestUnPause = false;
    requestQuit = false;
    setupGuiEventHandlers();*/
}

void InventoryState::exit()
{
    //ParticleSystemManager::getSingleton().setTimeFactor(1);

    //CEGUI::WindowManager::getSingleton().destroyWindow(mEditorGuiSheet);

    /*if(GuiListener)
    {
        mRoot->removeFrameListener(GuiListener);
    }*/

    //CEGUI::WindowManager& wmgr = CEGUI::WindowManager::getSingleton();

   //wmgr.getWindow((CEGUI::utf8*)"Paused")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&PauseState::unPause, this));

    //mGUISystem->setDefaultMouseCursor(NULL);

/*
    if(mEditorGuiSheet)
    {
        CEGUI::WindowManager::getSingleton().destroyWindow(mEditorGuiSheet);
        mEditorGuiSheet = 0;
    }

    if(mGUISystem)
    {
        //delete mGUISystem;
        //mGUISystem = 0;
    }
*/

    /*GuiListener = GameManager::getSingletonPtr()->GuiListener;
    if(GuiListener)
    {
        mRoot->removeFrameListener(GuiListener);
        GameManager::getSingletonPtr()->GuiListener = 0;
        GuiListener = 0;
    }

    mGUIRenderer = GameManager::getSingletonPtr()->mGUIRenderer;
    if(mGUIRenderer)
    {
        delete mGUIRenderer;
        GameManager::getSingletonPtr()->mGUIRenderer = 0;
        mGUIRenderer = 0;
    }*/
}

void InventoryState::pause()
{
}

void InventoryState::resume()
{
}

void InventoryState::keyPressed(const OIS::KeyEvent &e)
{
    if (e.key == OIS::KC_P)
    {
        requestUnPause = true;
    }
    if (e.key == OIS::KC_Q)
    {
        requestQuit = true;
    }
    else if(e.key == OIS::KC_ESCAPE)
    {
        requestUnPause = true;
    }
}

void InventoryState::keyReleased(const OIS::KeyEvent &e)
{
}


void InventoryState::mouseMoved(const OIS::MouseEvent &e)
{
    /*CEGUI::System::getSingleton().injectMouseMove(
               e->getRelX() * mGUIRenderer->getWidth(), 
               e->getRelY() * mGUIRenderer->getHeight());
    e->consume();*/
}

void InventoryState::mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id)
{
    /*CEGUI::System::getSingleton().injectMouseButtonDown(
         GuiListener->convertOgreButtonToCegui(e->getButtonID()));
       e->consume();*/
}

void InventoryState::mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id)
{
    SoundManager *sm = SoundManager::getSingletonPtr();
    sm->manageSound(sm->playSound("Click.wav",0.5,false));

    /*CEGUI::System::getSingleton().injectMouseButtonUp(
        GuiListener->convertOgreButtonToCegui(e->getButtonID()));
       e->consume();*/
}

//Gui Event Functions
void InventoryState::setupGuiEventHandlers(void)
{
// FIXME: menus
#if 0
    CEGUI::WindowManager& wmgr = CEGUI::WindowManager::getSingleton();
   //wmgr.getWindow((CEGUI::utf8*)"Paused")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&PauseState::unPause, this));

   wmgr.getWindow((CEGUI::utf8*)"Resume")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&InventoryState::resume, this));
   wmgr.getWindow((CEGUI::utf8*)"Quit")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&InventoryState::quitGame, this));
#endif
}

bool InventoryState::resume(MyGUI::WidgetPtr _sender)
{
    requestUnPause = true;
    return true;
}
bool InventoryState::quitGame(MyGUI::WidgetPtr _sender)
{
    requestQuit = true;
    return true;
}

//End Gui Event Functions

bool InventoryState::frameStarted(const FrameEvent& evt)
{
    if(requestUnPause)
    {
        popState();
    }
    if(requestQuit)
    {
        return false;
    }
    return true;
}

bool InventoryState::frameEnded(const FrameEvent& evt)
{
    return true;
}
