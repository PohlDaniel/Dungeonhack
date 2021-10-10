#ifndef _GUI_MESSAGEBOX_H
#define _GUI_MESSAGEBOX_H

#include <string>
#include "GuiView.h"

using namespace std;


class GuiMessageBox : public GuiView
{
public:
    static const int MAX_OPTIONS = 12;

    GuiMessageBox(UiManager* gui) : GuiView(gui) {
        m_numOptions = 0;
        m_hasReturned = false;
        m_retValue = -1;
    }

    virtual ~GuiMessageBox();

    virtual void update();

    void setMessage(string msg)     { m_messageCaption = msg; }

    void setOptions(int numOptions, string theOptions[]);

    bool hasReturned() { return m_hasReturned; }
    int getReturnValue() { return m_retValue; }

protected:
    virtual void loadLayout();
    virtual void registerEvents();
    virtual void unregisterEvents() {};

    void hitButton(MyGUI::WidgetPtr _sender);

    string m_messageCaption;
    string m_options[MAX_OPTIONS];
    int m_values[MAX_OPTIONS];
    int m_numOptions;

    bool m_hasReturned;
    int m_retValue;
};

#endif  // _GUI_MESSAGEBOX_H
