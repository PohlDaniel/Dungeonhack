#ifndef _GUI_VIEW_H
#define _GUI_VIEW_H

#include <MyGUI_Button.h>
#include "UiManager.h"


/**
    Base class for all GUI views
*/
class GuiView
{
public:
    GuiView(UiManager* gui);
    virtual ~GuiView();

    virtual void setup();
    virtual void cleanup();

    virtual void setVisible(bool);
    virtual bool getVisible() { return m_visible; }
    virtual void show() { setVisible(true); }
    virtual void hide() { setVisible(false); }

    virtual void suspend() { unregisterEvents(); hide(); }
    virtual void resume() { show(); registerEvents(); }

    virtual void update() = 0;

protected:
    virtual void loadLayout() = 0;
    virtual void clearLayout();

    virtual void registerEvents() = 0;
    virtual void unregisterEvents() = 0;

    UiManager* m_gui;
    MyGUI::VectorWidgetPtr m_widgets;
    bool m_visible;

    /**
        Common function used to assign button to an handler
    */
    template <class T>
    inline void assignButton(const char* name, T* obj, void (T::*func)(MyGUI::WidgetPtr))
    {
        MyGUI::ButtonPtr button = m_gui->findWidget<MyGUI::Button>(name);
        if (button)
            button->eventMouseButtonClick = newDelegate(obj, func);
    }
};


#endif
