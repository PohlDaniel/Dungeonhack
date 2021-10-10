#include "DungeonHack.h"
#include "GuiView.h"
#include "UiManager.h"

using namespace MyGUI;


/**
    Constructor
*/
GuiView::GuiView(UiManager* gui) : m_gui(gui)
{
    m_visible = true;
    m_widgets.clear();
}


/**
    Destructor
*/
GuiView::~GuiView()
{
    cleanup();
}


/**
    Initialize the view
*/
void GuiView::setup()
{
    loadLayout();
    registerEvents();
}


/**
    Cleanup the view
*/
void GuiView::cleanup()
{
    clearLayout();
}


/**
    Show/hide all elements of the view
*/
void GuiView::setVisible(bool vis)
{
    EnumeratorWidgetPtr it = EnumeratorWidgetPtr(m_widgets);
    while (it.next())
    {
        WidgetPtr widget = it.current();
        widget->setVisible(vis);
    }
    m_visible = vis;
    if (m_visible)
    {
        update();
    }
}


/**
    Remove all widgets
*/
void GuiView::clearLayout()
{
    LayoutManager::getInstance().unloadLayout(m_widgets);
    m_widgets.clear();
}
