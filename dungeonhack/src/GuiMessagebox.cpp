#include "DungeonHack.h"
#include "GuiMessagebox.h"
#include "UiManager.h"
#include "GameManager.h"

using namespace MyGUI;


GuiMessageBox::~GuiMessageBox()
{
    m_gui->hidePointer();
}


void GuiMessageBox::loadLayout()
{
    m_widgets = LayoutManager::getInstance().load("MessageBox.layout");
    update();
    m_gui->showPointer();
}


void GuiMessageBox::update()
{
    const int buttonSlotsReserve = 2;
    const float textPanelRatio = 1.1f;
    const float buttonShiftFactor = 1.3f;
    float guiScale = (float) ( GameManager::getSingletonPtr()->getMainWindow()->getWidth() ) / 640.0f;

    WidgetManager* wmgr = WidgetManager::getInstancePtr();

    // Scale font according to resolution
    EditPtr txt = wmgr->findWidget<Edit>("Text01");
    int fntHeight = txt->getFontHeight();
    txt->setFontHeight( (int) ((float)fntHeight * guiScale) );
    txt->setCaption(m_messageCaption);
    
    // Compute estimated text height
    int width = 30;
    int height = 1 + (m_messageCaption.length() / width);  // FIXME: MyGUI may have a proper method

    IntSize txtsz = txt->getSize();
    if (height > 8)
    {
        txtsz.width *= 2;
        height /= 2;
    }
    int oldHeight = txtsz.height;
    txtsz.height = (int) ( (float)txtsz.height * (float)height );
    txt->setSize(txtsz);

    ButtonPtr but0 = wmgr->findWidget<Button>("Button00");
    int butHeight = but0->getSize().height;

    // Adjust panel size according to length of text and number of buttons
    WidgetPtr pan = wmgr->findWidget<Widget>("Panel01");
    IntSize pansz = pan->getSize();
    pansz.width = (int) ((float)txtsz.width * textPanelRatio);
    pansz.height = butHeight * ( buttonSlotsReserve + height + (m_numOptions * guiScale / buttonShiftFactor) );
    if ((unsigned)pansz.height > GameManager::getSingletonPtr()->getMainWindow()->getHeight())
        pansz.height = GameManager::getSingletonPtr()->getMainWindow()->getHeight();
    pan->setSize(pansz);

    // Center vertically
    IntPoint panps = pan->getPosition();
    panps.top = ( GameManager::getSingletonPtr()->getMainWindow()->getHeight() / 2 ) - (pansz.height / 2);
    if (panps.top < 0) panps.top = 0;
    panps.left = ( GameManager::getSingletonPtr()->getMainWindow()->getWidth() / 2 ) - (pansz.width / 2);
    if (panps.left < 0) panps.left = 0;
    pan->setPosition(panps);

    // Align buttons with the text et hide if disabled
    char id[3] = "00";
    string buttonPrefix("Button");
    string buttonName;
    for (int i=0; i<MAX_OPTIONS; i++)
    {
        id[0] = '0' + i/10;
        id[1] = '0' + i%10;
        buttonName = buttonPrefix + id;
        ButtonPtr but = wmgr->findWidget<Button>(buttonName);
        IntPoint butps = but->getPosition();
        butps.left = (pansz.width / 2) - (but->getWidth() / 2);
        butps.top += (int) ( (float)(txtsz.height - oldHeight) / buttonShiftFactor );
        but->setPosition(butps);
        if (i >= m_numOptions)
        {
            but->setVisible(false);
        }
        else
        {
            but->setCaption(m_options[i]);
        }
    }
}


void GuiMessageBox::registerEvents()
{
    assignButton("Button00", this, &GuiMessageBox::hitButton);
    assignButton("Button01", this, &GuiMessageBox::hitButton);
    assignButton("Button02", this, &GuiMessageBox::hitButton);
    assignButton("Button03", this, &GuiMessageBox::hitButton);
    assignButton("Button04", this, &GuiMessageBox::hitButton);
    assignButton("Button05", this, &GuiMessageBox::hitButton);
    assignButton("Button06", this, &GuiMessageBox::hitButton);
    assignButton("Button07", this, &GuiMessageBox::hitButton);
    assignButton("Button08", this, &GuiMessageBox::hitButton);
    assignButton("Button09", this, &GuiMessageBox::hitButton);
    assignButton("Button10", this, &GuiMessageBox::hitButton);
    assignButton("Button11", this, &GuiMessageBox::hitButton);
}


void GuiMessageBox::setOptions(int numOptions, string theOptions[])
{
    m_numOptions = numOptions;
    if(m_numOptions > MAX_OPTIONS)
    {
        //Limit to 12 options for now
        m_numOptions = MAX_OPTIONS;
    }

    for(int i = 0; i < m_numOptions; i++)
    {
        m_options[i] = theOptions[i];
    }
}


void GuiMessageBox::hitButton(WidgetPtr _sender)
{
    string buttonName;
    string buttonPrefix("Button");
    int index = -1;

    buttonName = _sender->getName();
    if ( (buttonName.compare(0, 6, buttonPrefix) == 0) && (buttonName.length() == 8) )
    {
        index = (buttonName[6] - '0') * 10 + (buttonName[7] - '0');
        if (index < 0 || index >= m_numOptions)
            index = -1;
    }

    if (index != -1)
    {
        if (m_numOptions <= 2) index++; // FIXME: Have to check convention, cf old code
        
        m_retValue = index;
        m_hasReturned = true;
    }

#if 0
    if(testStr == "Resume")
    {
        this->m_returnedVal = 1;
    }
    else if(testStr == "OK" || testStr == "Yes")
    {
        this->m_returnedVal = 1;
    }
    else if(testStr == "Cancel" || testStr == "No" || testStr == "Quit")
    {
        this->m_returnedVal = 2;
    }
    else
    {
        //If the button is named something like "Button0" or "Button1" return the corresponding option
        if(this->m_numOptions > 0)
        {
            for(int i = 0; i < m_numOptions; i++)
            {
                char myNum[5];
                itoa(i,myNum,10);
                CEGUI::String myNumStr = myNum;

                if(testStr == "Button" + myNumStr)
                {
                    this->m_returnedVal = i;
                }
            }
        }
    }
#endif
}
