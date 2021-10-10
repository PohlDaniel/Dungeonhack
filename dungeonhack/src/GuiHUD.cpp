#include "DungeonHack.h"
#include "GuiHUD.h"
#include "UiManager.h"

using namespace MyGUI;


GuiHUD::~GuiHUD()
{
    clearWeapon();
    clearSpell();
}


void GuiHUD::clearWeapon()
{
    m_oldweapon.clear();
    m_weapon.clear();
}

void GuiHUD::clearSpell()
{
    m_oldspell.clear();
    m_spell.clear();
}


void GuiHUD::loadLayout()
{
    m_widgets = LayoutManager::getInstance().load("HUD.layout");
    WidgetManager::getInstance().findWidget<StaticText>("TextDebug")->setColour(Colour::Red);
    clearWeapon();
    m_gui->hidePointer();
    if (!m_showStats)
    {
        WidgetManager::getInstance().findWidget<StaticText>("TextStats")->setVisible(false);
    }
}


void GuiHUD::update()
{
    WidgetManager* wmgr = WidgetManager::getInstancePtr();
    StaticImagePtr healthBar =      wmgr->findWidget<StaticImage>("BarHealth");
    StaticImagePtr healthBarOpp =   wmgr->findWidget<StaticImage>("BarHealthOpp");
    StaticImagePtr magicBar =       wmgr->findWidget<StaticImage>("BarMagica");
    StaticImagePtr magicBarOpp =    wmgr->findWidget<StaticImage>("BarMagicaOpp");
    StaticImagePtr fatigueBar =     wmgr->findWidget<StaticImage>("BarStamina");
    StaticImagePtr fatigueBarOpp =  wmgr->findWidget<StaticImage>("BarStaminaOpp");
    StaticTextPtr debugText =       wmgr->findWidget<StaticText>("TextDebug");
    StaticTextPtr infoText =        wmgr->findWidget<StaticText>("TextInfo");
    StaticTextPtr statsText =       wmgr->findWidget<StaticText>("TextStats");

    debugText->setCaption(m_debug);
    infoText->setCaption(m_info);
    if (m_showStats)
    {
        statsText->setCaption(m_stats);
        statsText->setVisible(true);
    }
    else
    {
        statsText->setVisible(false);
    }

    IntCoord barsz;
    IntCoord barsz_;

    barsz = healthBar->getSize();
    barsz_ = barsz;
    barsz_.top = 0;
    barsz_.left = (int) ( (float)barsz.width * m_health );
    barsz_.width = (int) ( (float)barsz.width * (1.0 - m_health) ) + 2;
    healthBarOpp->setCoord(barsz_);

    barsz = magicBar->getSize();
    barsz_ = barsz;
    barsz_.top = 0;
    barsz_.left = (int) ( (float)barsz.width * m_magica );
    barsz_.width = (int) ( (float)barsz_.width * (1.0 - m_magica) ) + 2;
    magicBarOpp->setCoord(barsz_);

    barsz = fatigueBar->getSize();
    barsz_ = barsz;
    barsz_.top = 0;
    barsz_.left = (int) ( (float)barsz.width * m_fatigue );
    barsz_.width = (int) ( (float)barsz_.width * (1.0 - m_fatigue) ) + 2;
    fatigueBarOpp->setCoord(barsz_);

    if (m_oldweapon != m_weapon)
    {
        StaticImagePtr weapon = wmgr->findWidget<StaticImage>("ImageWeapon");
        if (m_weapon != "")
        {
            weapon->setImageTexture(m_weapon);
            weapon->setVisible(true);
        }
        else
        {
            weapon->setVisible(false);
        }
        m_oldweapon = m_weapon;
    }

    if (m_oldspell != m_spell)
    {
        StaticImagePtr spell = wmgr->findWidget<StaticImage>("ImageSpell");
        if (m_spell != "")
        {
            spell->setImageTexture(m_spell);
            spell->setVisible(true);
        }
        else
        {
            spell->setVisible(false);
        }
        m_oldspell = m_spell;
    }
}
