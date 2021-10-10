#ifndef _GUI_PAUSED_H
#define _GUI_PAUSED_H

#include "GuiView.h"


class GuiPaused : public GuiView
{
public:
    GuiPaused(UiManager* gui) : GuiView(gui), m_resumeRequested(false), m_quitRequested(false), m_configRequested(false) {};
    virtual ~GuiPaused();

    virtual void update() {};

    bool getIsResumeRequested() { return m_resumeRequested; }
    bool getIsQuitRequested() { return m_quitRequested; }
    bool getIsConfigRequested() { return m_configRequested; }
    void clearConfigRequested() { m_configRequested = false; }

    void doResume(MyGUI::WidgetPtr _sender) { m_resumeRequested = true; }
    void doQuit(MyGUI::WidgetPtr _sender) { m_quitRequested = true; }
    void doConfigure(MyGUI::WidgetPtr _sender) { m_configRequested = true; }

protected:
    virtual void loadLayout();
    virtual void registerEvents();
    virtual void unregisterEvents();

    bool m_resumeRequested, m_quitRequested, m_configRequested;
};

#endif // _GUI_PAUSED_H
