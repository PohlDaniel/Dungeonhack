#ifndef scriptEntity_H
#define scriptEntity_H

#include "GameObject.h"
#include "PythonManager.h"

class ScriptEntity : public PointEntity
{
public:
    ScriptEntity();
    ~ScriptEntity();

    void Init(Vector3 pos, string triggeredOn, string scriptFile = "", string triggerOther = "", float touchTriggerDistance = 100, bool triggerOnce = true);
    virtual void Update(float MoveFactor);
    virtual void onLoad();
    void trigger();

private:
    string m_scriptFile;
    string m_triggeredOn;
    string m_triggerOther;
    float m_triggerDistance;
    bool m_triggerOnce;
    bool m_canTrigger;
    bool m_isFirstUpdate;
};

#endif
