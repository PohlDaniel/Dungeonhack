#ifndef _QUEST_MANAGER_H_
#define _QUEST_MANAGER_H_

#include <stdlib.h>
#include <vector>
#include <Ogre.h>
#include "GamePrereqs.h"
#include "Location.h"

using namespace Ogre;
using namespace std;
using namespace stdext;

class QuestActor
{
public:
    QuestActor() { m_inUse = false; }

    String m_myLocID;
    int m_blockNum;
    int m_interiorNum;
    bool m_inUse;

    String m_actorType; //NPC, Monster
    String m_actorSubtype;  //Monster Type, or NPC Class

    //Quest functions
    String onDieQuestFunction;
    String onHurtQuestFunction;
    String onAggroQuestFunction;
};

class Quest
{
public:
    Quest(String name, String script);

    ~Quest();

    bool m_doQuit;
    bool m_isEnded;

    String m_Name;
    String m_ScriptFile;
    int m_currentStage;

    int m_updateFreq;
    int m_updatesSinceLastUpdate;
    float m_updateTotalMoveFactor;

    hash_map <String, String>::iterator Quest_Var_Iter;
    hash_map<String, String, DH_StringHash> m_QuestVars;

    vector<QuestActor*> m_questActors;

    void Start();
    void End();

    QuestActor * AddQuestMonster(String type, String theLocID);
    void placeQuestActor(QuestActor * theActor, LocationMarker * theMarker);

    void setQuestVariable(String name, String value);
    String getQuestVariable(String name);

    void changeStage(int stage);

    void Update(float MoveFactor);
    void SetUpdateFrequency(int freq);

    void onStart();
    void onSwitchStage(int stage);
    void onEnd();
    void onLocLoad(Location * theLoc);
    void onLocInteriorLoad(Interior * theLoc);
    void onUpdate(float MoveFactor);
};

class QuestManager : public Ogre::Singleton<QuestManager>
{
public:
    QuestManager()
    {

    }

    virtual ~QuestManager()
    {

    }

    void Update(float MoveFactor);

    void LocLoad(Location * theLoc)
    {
        hash_map <String, Quest *>::iterator Quest_Iter;

        for (Quest_Iter = m_activeQuests.begin(); Quest_Iter != m_activeQuests.end(); ++Quest_Iter)
        {
            Quest_Iter->second->onLocLoad(theLoc);
        }
    }

    void LocInteriorLoad(Interior * theLoc)
    {
        hash_map <String, Quest *>::iterator Quest_Iter;

        for (Quest_Iter = m_activeQuests.begin(); Quest_Iter != m_activeQuests.end(); ++Quest_Iter)
        {
            Quest_Iter->second->onLocInteriorLoad(theLoc);
        }
    }

    hash_map<String, Quest *, DH_StringHash> m_activeQuests;

    Quest * addQuest(String name, String scriptFile)
    {
        Quest * newQuest = new Quest(name, scriptFile);
        m_activeQuests[name] = newQuest;
        newQuest->Start();

        return newQuest;
    }

    static QuestManager& getSingleton(void);
    static QuestManager* getSingletonPtr(void);
};

#endif
