#ifndef _GUI_HUD_H
#define _GUI_HUD_H

#include <string>
#include "GuiView.h"

using namespace std;


class GuiHUD : public GuiView
{
public:
    GuiHUD(UiManager* gui) : GuiView(gui) {};
    virtual ~GuiHUD();

    void setHealth(float r)        { m_health = r; }
    void setMagica(float r)        { m_magica = r; }
    void setFatigue(float r)       { m_fatigue = r; }

    void switchWeapon(string w)    { m_weapon = w; } 
    void switchSpell(string w)     { m_spell = w; } 

    void setDebug(string d)        { m_debug = d; }
    void setInfo(string i)         { m_info = i; }
    void setStats(string s)        { m_stats = s; }
    void setStatsVisible(bool v)   { m_showStats = v; }

    virtual void update();

protected:
    virtual void loadLayout();
    virtual void registerEvents()   {};
    virtual void unregisterEvents() {};

    void clearWeapon();
    void clearSpell();

    bool m_showStats;
    float m_health, m_magica, m_fatigue;
    string m_oldweapon, m_weapon;
    string m_oldspell, m_spell;
    string m_debug;
    string m_info;
    string m_stats;
};

#endif // _GUI_HUD_H
