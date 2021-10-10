#ifndef _PYTHON_MANAGER_H_
#define _PYTHON_MANAGER_H_

#include <string>
using namespace Ogre;
using namespace std;


class ScriptThread;
class Quest;

class PythonManager : public Ogre::Singleton<PythonManager>
{
public:
    PythonManager();
    virtual ~PythonManager();

    void suspendThread();
    bool isWaiting();
    void resumeThread();
    void Update();

    void runCommand(string cmd);
    bool runScript(string filename);
    void callFunction(string filename, string function);

    //Quest functions pass in a quest
    void callQuestFunction(string filename, string function, Quest * theQuest);
    void callQuestFunction(string filename, string function, Quest * theQuest, int value);

    string getLastScript() const { return m_lastRunScriptFile; }

    static PythonManager& getSingleton(void);
    static PythonManager* getSingletonPtr(void);
    
protected:
    void linkFunctions();
    bool loadScript(String filename, ScriptThread* thread);

    list<ScriptThread*> m_threads;
    ScriptThread* m_mainThread;
    string m_lastRunScriptFile;
};

#endif
