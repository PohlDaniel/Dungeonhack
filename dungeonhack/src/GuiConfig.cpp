#include "DungeonHack.h"
#include "GuiConfig.h"
#include "ConfigState.h"
#include "UiManager.h"
#include "GameManager.h"


using namespace MyGUI;


/**
    Destructor
*/
GuiConfig::~GuiConfig()
{
}


/**
    Load the console elements
*/
void GuiConfig::loadLayout()
{
    m_widgets = LayoutManager::getInstance().load("Config.layout");
    m_gui->showPointer();
}


/**
    Cleanup console elements
*/
void GuiConfig::clearLayout()
{
    GuiView::clearLayout();
}


/**
    Register Config events
*/
void GuiConfig::registerEvents()
{
    MyGUI::WidgetManager* wmgr = MyGUI::WidgetManager::getInstancePtr();

    MyGUI::HScroll* ambvol = wmgr->findWidget<MyGUI::HScroll>("SliderAudAmbient");
    ambvol->eventScrollChangePosition = MyGUI::newDelegate(this, &GuiConfig::onChangeVolume);

    MyGUI::HScroll* musvol = wmgr->findWidget<MyGUI::HScroll>("SliderAudMusic");
    musvol->eventScrollChangePosition = MyGUI::newDelegate(this, &GuiConfig::onChangeVolume);

    MyGUI::HScroll* pspd = wmgr->findWidget<MyGUI::HScroll>("SliderPlaySpeed");
    pspd->eventScrollChangePosition = MyGUI::newDelegate(this, &GuiConfig::onChangeSpeed);

    MyGUI::HScroll* pgrv = wmgr->findWidget<MyGUI::HScroll>("SliderPlayGravity");
    pgrv->eventScrollChangePosition = MyGUI::newDelegate(this, &GuiConfig::onChangeGravity);
}


/**
    Update config UI with current settings - done once in this case
*/
void GuiConfig::update()
{
    GameConfiguration* config = GameManager::getSingletonPtr()->getConfig();
    MyGUI::WidgetManager* wmgr = MyGUI::WidgetManager::getInstancePtr();

    try {
        MyGUI::HScroll* speed = wmgr->findWidget<MyGUI::HScroll>("SliderPlaySpeed");
        speed->setScrollRange(21);
        //speed->setScrollPosition( static_cast<int>( config->getFloat("speed") ) );
        speed->setScrollPosition( static_cast<int>( config->getFloat("dayScale") ) );

        MyGUI::HScroll* gravity = wmgr->findWidget<MyGUI::HScroll>("SliderPlayGravity");
        gravity->setScrollRange(1001);
        gravity->setScrollPosition(config->getFloat("gravity"));

        MyGUI::Button* grass = wmgr->findWidget<MyGUI::Button>("CheckGraGrass");
        if (config->getInteger("doGrass") == 1)
            grass->setButtonPressed(true);

        MyGUI::Button* reflect = wmgr->findWidget<MyGUI::Button>("CheckGraReflect");
        if (config->getInteger("doWaterReflection") == 1)
            reflect->setButtonPressed(true);

        MyGUI::HScroll* ambvol = wmgr->findWidget<MyGUI::HScroll>("SliderAudAmbient");
        ambvol->setScrollRange(11);
        ambvol->setScrollPosition(config->getFloat("ambientVolume") * 10.0);

        MyGUI::HScroll* musvol = wmgr->findWidget<MyGUI::HScroll>("SliderAudMusic");
        musvol->setScrollRange(11);
        musvol->setScrollPosition(config->getFloat("musicVolume") * 10.0);

        /*
        if ( n.compare("EditKeyUp") == 0 )
        {
            std::cout << "Config item not yet implemented: " << n << std::endl;
        }
        else if ( n.compare("EditKeyDown") == 0 )
        {
            std::cout << "Config item not yet implemented: " << n << std::endl;
        }
        else if ( n.compare("EditKeyLeft") == 0 )
        {
            std::cout << "Config item not yet implemented: " << n << std::endl;
        }
        else if ( n.compare("EditKeyRight") == 0 )
        {
            std::cout << "Config item not yet implemented: " << n << std::endl;
        }
        else if ( n.compare("EditKeyAction") == 0 )
        {
            std::cout << "Config item not yet implemented: " << n << std::endl;
        }
        else if ( n.compare("EditKeyConsole") == 0 )
        {
            std::cout << "Config item not yet implemented: " << n << std::endl;
        }
        */

        MyGUI::HScroll* mssx = wmgr->findWidget<MyGUI::HScroll>("SliderMouSensX");
        mssx->setScrollRange(11);
        MyGUI::HScroll* mssy = wmgr->findWidget<MyGUI::HScroll>("SliderMouSensY");
        mssy->setScrollRange(11);

        MyGUI::Button* msinv = wmgr->findWidget<MyGUI::Button>("CheckMouInvert");
    }
    catch(...)
    {
    }
}


void GuiConfig::onChangeVolume(MyGUI::VScrollPtr _sender, size_t _position)
{
    float res = 1.0;
    res = _sender->getScrollPosition() / 10.0;

    if (!_sender->getName().compare("SliderAudAmbient"))
    {
        m_configState->notify("ambientVolume", Variant(res));
    }
    else if (!_sender->getName().compare("SliderAudMusic"))
    {
        m_configState->notify("musicVolume", Variant(res));
    }
}


void GuiConfig::onChangeSpeed(MyGUI::VScrollPtr _sender, size_t _position)
{
    float res = 1.0;
    res = _sender->getScrollPosition();
    //m_configState->notify("speed", Variant(res));
    m_configState->notify("dayScale", Variant(res));
}


void GuiConfig::onChangeGravity(MyGUI::VScrollPtr _sender, size_t _position)
{
    float res = 1.0;
    res = _sender->getScrollPosition();
    m_configState->notify("gravity", Variant(res));
}
