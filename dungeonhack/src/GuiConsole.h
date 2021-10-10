#ifndef _GUI_CONSOLE_H
#define _GUI_CONSOLE_H

#include "GuiView.h"
#include <string>
#include <OgreUTFString.h>
#include <MyGUI_Window.h>
#include <MyGUI_Edit.h>
#include <MyGUI_ComboBox.h>

using namespace std;
using namespace Ogre;


class GuiConsole : public GuiView
{
public:
    GuiConsole(UiManager* gui) : GuiView(gui), m_resumeRequested(false), m_quitRequested(false), m_script("") { };
    virtual ~GuiConsole();

    virtual void update() {};

    bool getIsResumeRequested() { return m_resumeRequested; }
    bool getIsQuitRequested() { return m_quitRequested; }
    string getScript() { return m_script; }

    string getCommand() { return m_command; }
    void clearCommand() { m_command.clear(); }

    void close(MyGUI::WidgetPtr _sender, const string& _button) { if (_button=="close") m_resumeRequested = true; };
    void submit(MyGUI::WidgetPtr _sender);
    void notifyComboKeyPress(MyGUI::WidgetPtr _sender, MyGUI::KeyCode _key, MyGUI::Char _char);

protected:
    virtual void loadLayout();
    virtual void clearLayout();
    virtual void registerEvents();
    virtual void unregisterEvents() {};

    void loadHistory();
    void addHistory(const Ogre::UTFString& cmd);
    void clearHistory();

    void processCommand(const Ogre::UTFString& cmd);

    bool m_resumeRequested, m_quitRequested;
    string m_script;
    string m_command;

    static vector<Ogre::UTFString> m_commands;
    static string m_history;

    MyGUI::EditPtr     m_listHistory;
    MyGUI::ComboBoxPtr m_comboCommand;
};

#endif // _GUI_CONSOLE_H
