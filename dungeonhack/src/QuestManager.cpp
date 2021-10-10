#include "DungeonHack.h"
#include "QuestManager.h"
#include "PythonManager.h"

#include "Monster.h"
#include "Items.h"
#include "EntityManager.h"
#include "time.h"


template<> QuestManager* Ogre::Singleton<QuestManager>::ms_Singleton = 0;

//Quest manager singleton functions
QuestManager* QuestManager::getSingletonPtr(void)
{
    return ms_Singleton;
}

QuestManager& QuestManager::getSingleton(void)
{
    assert(ms_Singleton);
    return *ms_Singleton;
}

void QuestManager::Update(float MoveFactor)
{
    hash_map <String, Quest *>::iterator Quest_Iter;
    hash_map <String, Quest *>::iterator Remove_Iter;

    bool doRemove = false;

    int questNum = 0;
    for (Quest_Iter = m_activeQuests.begin(); Quest_Iter != m_activeQuests.end(); ++Quest_Iter)
    {
        if (!Quest_Iter->second)
            continue;

        //Don't remove the quest yet if we're still waiting for a function to return
        if(Quest_Iter->second->m_isEnded == true && PythonManager::getSingletonPtr()->isWaiting() == false)
        {
            doRemove = true;
            Remove_Iter = Quest_Iter;
        }
        else
        {
            Quest_Iter->second->Update(MoveFactor);
        }

        questNum++;
    }

    if(doRemove == true)
    {
        m_activeQuests.erase(Remove_Iter);
    }
}

//Quest class functions

Quest::Quest(String name, String script)
{ 
    m_Name = name;
    m_ScriptFile = script;

    m_updateFreq = 250;
    m_updatesSinceLastUpdate = 0;
    m_updateTotalMoveFactor = 0;

    m_doQuit = false;
    m_isEnded = false;
}

Quest::~Quest()
{
}

void Quest::Start()
{
    onStart();
    changeStage(1);
}

void Quest::End()
{
    m_doQuit = true;
}

void Quest::SetUpdateFrequency(int freq)
{
    m_updateFreq = freq;
}

void Quest::Update(float moveFactor)
{
    m_updateTotalMoveFactor += moveFactor;
    m_updatesSinceLastUpdate += 1;

    //char z[128];
    //itoa(m_updatesSinceLastUpdate,z,10);

    //MessageBox(0,z,"Hi",0);

    if(m_updatesSinceLastUpdate >= m_updateFreq)
    {
        onUpdate(m_updateTotalMoveFactor);

        m_updatesSinceLastUpdate = 0;
        m_updateTotalMoveFactor = 0;
    }

    if(m_doQuit == true && m_isEnded == false)
    {
        onEnd();
        m_isEnded = true;
    }
}

void Quest::setQuestVariable(String name, String value)
{
    m_QuestVars[name] = value;
}

String Quest::getQuestVariable(String name)
{
    return m_QuestVars[name];
}

void Quest::changeStage(int stage)
{
    m_currentStage = stage;
    onSwitchStage(stage);
}

QuestActor * Quest::AddQuestMonster(String type, String locID)
{
    QuestActor * newActor = new QuestActor;
    newActor->m_myLocID = locID;
    newActor->m_actorType = "Monster";
    newActor->m_actorSubtype = type;

    m_questActors.push_back(newActor);

    return newActor;
}

void Quest::onStart()
{
    PythonManager::getSingleton().callQuestFunction(m_ScriptFile,"OnQuestStart",this);
}

void Quest::onSwitchStage(int stage)
{
    PythonManager::getSingleton().callQuestFunction(m_ScriptFile,"OnQuestChangeStage",this,stage);
}

void Quest::onEnd()
{
    PythonManager::getSingleton().callQuestFunction(m_ScriptFile,"OnQuestEnd",this);
}

void Quest::onUpdate(float MoveFactor)
{
    PythonManager::getSingleton().callQuestFunction(m_ScriptFile,"OnUpdate",this,MoveFactor);
}

void Quest::onLocLoad(Location * theLoc)
{
    srand ( time(NULL) );

    //If we have any quest actors, so if they go to this location
    if(!m_questActors.empty())
    {
        std::vector<QuestActor*>::iterator qIt;

        vector<LocationMarker *> questMarkers;
        int numQuestMarkers = 0;

        std::vector<LocationMarker*>::iterator it;
        for (it = theLoc->m_Markers.begin(); it != theLoc->m_Markers.end(); ++it)
        {
            if(*it)
            {
                LocationMarker * thisLoc = *it;
                if(thisLoc != 0)
                {
                    if(thisLoc->type == QUEST_MARKER)
                    {
                        numQuestMarkers++;
                        questMarkers.push_back(thisLoc);
                    }
                }
            }
        }

        if(numQuestMarkers > 0)
        {
            for(qIt = m_questActors.begin(); qIt != m_questActors.end(); ++qIt)
            {
                if(*qIt)
                {
                    QuestActor * thisActor = *qIt;

                    if(thisActor != 0)
                    {
                        if(thisActor->m_myLocID == theLoc->getLocID())
                        {
                            //Pick quest marker for quest entity
                            int chosenQuestMarker = rand() % questMarkers.size();
                            LocationMarker * thisMarker = questMarkers[chosenQuestMarker];

                            placeQuestActor(thisActor, thisMarker);

                            it = questMarkers.begin() + chosenQuestMarker;
                            questMarkers.erase(it);
                            --it;
                        }
                    }
                }
            }
        }
    }

    PythonManager::getSingleton().callQuestFunction(m_ScriptFile,"OnLocLoad",this);
}

void Quest::placeQuestActor(QuestActor * theActor, LocationMarker * theMarker)
{
    if(theActor->m_inUse == false)
    {
        //TODO: When loading is in place, the quest monster will be saved so we won't have to generate it again a second time.
        //theActor->m_inUse = true;

        GameActor * spawnedActor;

        if(theActor->m_actorType == "Monster")
        {
            if(theActor->m_actorSubtype == "Zombie")
            {
                Monster* theMonster = new Monster(theActor->m_actorSubtype);
                EntityManager::getSingletonPtr()->addLocalEntity(theMonster);
                theMonster->Init(theMarker->m_Position, Vector3::UNIT_SCALE, "Zombie.mesh",
                    "Attack1", "Walk", "Idle", "zombie_3.wav", "zombie_4.wav",
                    "zombie_1.wav");
                spawnedActor = theMonster;
            }
            else if(theActor->m_actorSubtype == "Spider")
            {
                Monster* theMonster = new Monster(theActor->m_actorSubtype);
                EntityManager::getSingletonPtr()->addLocalEntity(theMonster);
                theMonster->Init(theMarker->m_Position, Vector3::UNIT_SCALE, "Spider.mesh",
                    "Attack1", "Walk", "Idle", "spider_cry.wav", "spider_cry.wav",
                    "spider_idle.wav");
                spawnedActor = theMonster;
            }
        }

        //Setup actor quest properties
        spawnedActor->m_isQuestActor = true;
        spawnedActor->m_quest = this;
        spawnedActor->setQuestAggroFunction(theActor->onAggroQuestFunction);
        spawnedActor->setQuestDieFunction(theActor->onDieQuestFunction);
        spawnedActor->setQuestHurtFunction(theActor->onHurtQuestFunction);
    }
}

void Quest::onLocInteriorLoad(Interior * theLoc)
{
    srand ( time(NULL) );

    //If we have any quest actors, so if they go to this location
    if(!m_questActors.empty())
    {
        std::vector<QuestActor*>::iterator qIt;

        vector<LocationMarker *> questMarkers;
        int numQuestMarkers = 0;

        std::vector<LocationMarker*>::iterator it;
        for (it = theLoc->m_Markers.begin(); it != theLoc->m_Markers.end(); ++it)
        {
            if(*it)
            {
                LocationMarker * thisLoc = *it;
                if(thisLoc != 0)
                {
                    if(thisLoc->type == QUEST_MARKER)
                    {
                        //MessageBox(0,"Got Quest Marker","Marker",0);
                        numQuestMarkers++;
                        questMarkers.push_back(thisLoc);
                    }
                }
            }
        }

        if(numQuestMarkers > 0)
        {
            for(qIt = m_questActors.begin(); qIt != m_questActors.end(); ++qIt)
            {
                if(*qIt)
                {
                    QuestActor * thisActor = *qIt;

                    if(thisActor != 0)
                    {
                        if(thisActor->m_myLocID == theLoc->m_ExteriorLoc->getLocID())
                        {
                            //Pick quest marker for quest entity
                            //int chosenQuestMarker = rand() % numQuestMarkers-1;
                            int chosenQuestMarker = rand() % questMarkers.size();
                            LocationMarker * thisMarker = questMarkers[chosenQuestMarker];

                            placeQuestActor(thisActor, thisMarker);

                            it = questMarkers.begin() + chosenQuestMarker;
                            questMarkers.erase(it);

                            //numQuestMarkers--;
                        }
                    }
                }
            }
        }
    }

    PythonManager::getSingleton().callQuestFunction(m_ScriptFile,"OnLocInteriorLoad",this);
}
