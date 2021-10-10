#include "DungeonHack.h"
#include "GuiConsole.h"
#include "UiManager.h"

using namespace MyGUI;


// We keep the list of typed commands between 2 console calls
vector<Ogre::UTFString> GuiConsole::m_commands;
string GuiConsole::m_history;


/**
    Destructor
*/
GuiConsole::~GuiConsole()
{
    m_gui->hidePointer();
}


/**
    Load the console elements
*/
void GuiConsole::loadLayout()
{
    m_widgets = LayoutManager::getInstance().load("Console.layout");
    m_listHistory   = WidgetManager::getInstancePtr()->findWidget<Edit>("ListHistory");
    m_comboCommand  = WidgetManager::getInstancePtr()->findWidget<ComboBox>("ComboCommand");
    loadHistory();
    m_gui->showPointer();
}


/**
    Cleanup console elements
*/
void GuiConsole::clearLayout()
{
    m_listHistory = NULL;
    m_comboCommand = NULL;
    GuiView::clearLayout();
}


/**
    Register Console events
*/
void GuiConsole::registerEvents()
{
    WindowPtr windowConsole = WidgetManager::getInstancePtr()->findWidget<Window>("WindowConsole");
    windowConsole->eventWindowButtonPressed = newDelegate(this, &GuiConsole::close);

    assignButton("ButtonSubmit", this, &GuiConsole::submit);
    m_comboCommand->eventKeyButtonPressed = newDelegate(this, &GuiConsole::notifyComboKeyPress);
}


/**
    Load history from cache
*/
void GuiConsole::loadHistory()
{
    for (vector<Ogre::UTFString>::iterator iter = m_commands.begin(); iter != m_commands.end(); iter++)
    {
        m_comboCommand->addItem(*iter);
    }
    m_listHistory->addText(m_history);
}


/**
    Add a new command to history
*/
void GuiConsole::addHistory(const Ogre::UTFString& cmd)
{
    m_commands.push_back(cmd);

    if (m_listHistory->getCaption().empty())
    {
        m_listHistory->addText(cmd);
        m_history.append(cmd);
    }
    else
    {
        m_listHistory->addText("\n" + cmd);
        m_history.append("\n" + cmd);
    }

    m_comboCommand->addItem(cmd);
}


/**
    Clear the history
*/
void GuiConsole::clearHistory()
{
    m_comboCommand->deleteAllItems();
    m_listHistory->setCaption("");
    m_commands.clear();
    m_history.clear();
}


/**
    Try to submit a new command
*/
void GuiConsole::submit(WidgetPtr _sender)
{
    if (!m_comboCommand->getCaption().empty())
    {
        addHistory(m_comboCommand->getCaption());
        processCommand(m_comboCommand->getCaption());
        m_comboCommand->setCaption("");
    }
}


/**
    Handle special key presses in Combo box
*/
void GuiConsole::notifyComboKeyPress(WidgetPtr _sender, KeyCode _key, Char _char)
{
    if (_key == KeyCode::Return || _key == KeyCode::NumpadEnter)
    {
        submit(_sender);
    }
}


/**
    Execute a command
*/
void GuiConsole::processCommand(const Ogre::UTFString& cmd)
{
    bool ok = false;

    if (cmd == "clear")
    {
        clearHistory();
        ok = true;
    }
    else if (cmd == "quit")
    {
        m_quitRequested = true;
        ok = true;
    }
    else if (cmd.substr(0, 10) == "runScript ")
    {
        m_listHistory->addText("\n#00FF00running script#FFFFFF ");
        m_listHistory->addText(cmd.substr(10));
        m_script = cmd.substr(10);
        ok = true;
    }
    if (!ok)
    {
        //m_listHistory->addText("\n#FF0000unknown command#FFFFFF");
        m_command = cmd;
    }
}
