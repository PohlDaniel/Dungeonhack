#include "DungeonHack.h"
#include "GuiPaused.h"
#include "UiManager.h"
#include "GameManager.h"
#include "GameConfig.h"

using namespace MyGUI;


GuiPaused::~GuiPaused()
{
    m_gui->hidePointer();
}


void GuiPaused::loadLayout()
{
    m_widgets = LayoutManager::getInstance().load("Paused.layout");
    m_gui->showPointer();

    GameConfiguration* config = GameManager::getSingletonPtr()->getConfig();
    if (!config->getInteger("enableGuiConfig"))
    {
        MyGUI::WidgetManager* wmgr = MyGUI::WidgetManager::getInstancePtr();
        MyGUI::WidgetPtr wdg = wmgr->findWidgetT("ButtonConfig");
        wdg->setVisible(false);
    }
}


void GuiPaused::registerEvents()
{
    assignButton("ButtonExit", this, &GuiPaused::doQuit);
    assignButton("ButtonResume", this, &GuiPaused::doResume);
    assignButton("ButtonConfig", this, &GuiPaused::doConfigure);
}


void GuiPaused::unregisterEvents()
{
    // TODO
}
