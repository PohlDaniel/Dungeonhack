#include "DungeonHack.h"
#include "ScriptEntity.h"
#include "EntityManager.h"

ScriptEntity::ScriptEntity()
{
    m_canTrigger = true;
    m_isFirstUpdate = true;
}

ScriptEntity::~ScriptEntity()
{

}

void ScriptEntity::Init(Vector3 pos, string triggeredOn, string scriptFile, string triggerOther, float touchTriggerDistance, bool triggerOnce)
{
    setPosition(pos);
    m_triggerDistance = touchTriggerDistance;
    m_triggeredOn = triggeredOn;
    m_scriptFile = scriptFile;
    m_triggerOther = triggerOther;
    m_triggerOnce = triggerOnce;
}

void ScriptEntity::Update(float MoveFactor)
{
    //Check distance between us and the player to see if we should trigger, if we should
    if(m_triggeredOn == "OnTouch" || m_triggeredOn == "onTouch")
    {
        float distance = (getPosition()
            - EntityManager::getSingletonPtr()->getPlayer()->getPosition()).normalise();

        if(distance <= m_triggerDistance)
        {
            trigger();
        }
    }

    //If this is the first update, call onLoad
    if(m_isFirstUpdate == true)
    {
        m_isFirstUpdate = false;
        onLoad();
    }
}

void ScriptEntity::trigger()
{
    if(m_canTrigger)
    {
        //Run script, if any
        if(m_scriptFile != "")
        {
            string Message = "Triggered Script: " + m_scriptFile;

            bool scriptOk = PythonManager::getSingletonPtr()->runScript(m_scriptFile);

            if(!scriptOk)
            {
                //uhoh, script error
            }
        }

        //trigger other trigger :)
        if(m_triggerOther != "")
        {
            //TODO: Get other entity, call it's trigger function
        }

        //If we're only supposed to trigger once, disable the trigger
        if(m_triggerOnce == true)
        {
            m_canTrigger = false;
        }
    }
}

void ScriptEntity::onLoad()
{
    //If we need to be triggered on a new area load, do it
    if(m_triggeredOn == "OnLoad" || m_triggeredOn == "onLoad")
    {
        trigger();
    }
}
