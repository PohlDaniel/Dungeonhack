#ifndef _GUI_CONFIG_H
#define _GUI_CONFIG_H

#include "GuiView.h"
#include <string>
#include <OgreUTFString.h>
#include <MyGUI_Window.h>
#include <MyGUI_Edit.h>
#include <MyGUI_ComboBox.h>

using namespace std;
using namespace Ogre;


class ConfigState;

class GuiConfig : public GuiView
{
public:
    GuiConfig(UiManager* gui) : GuiView(gui), m_resumeRequested(false), m_configState(0) { };
    virtual ~GuiConfig();

    virtual void update();

    void setCallback(ConfigState* cs) { m_configState = cs; }

    bool getIsResumeRequested() { return m_resumeRequested; }

    void onChangeVolume(MyGUI::VScrollPtr _sender, size_t _position);
    void onChangeSpeed(MyGUI::VScrollPtr _sender, size_t _position);
    void onChangeGravity(MyGUI::VScrollPtr _sender, size_t _position);

protected:
    virtual void loadLayout();
    virtual void clearLayout();
    virtual void registerEvents();
    virtual void unregisterEvents() {};

    bool m_resumeRequested;
	ConfigState* m_configState;
};

#endif // _GUI_CONFIG_H
