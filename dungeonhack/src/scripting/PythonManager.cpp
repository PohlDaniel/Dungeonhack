#include "DungeonHack.h"
#include "PythonManager.h"
#include "PythonThread.h"
#include <Python.h>
#include <stdio.h>

#include "PlayState.h"
#include "MessageState.h"
#include "SoundManager.h"

#include "GameObject.h"
#include "GameWorld.h"
#include "QuestManager.h"

extern "C" PyObject *PyInit__dungeonhack(); // FROM SWIG

#define SCRIPT_MAX_FILESIZE 16384

// Singleton
template<> PythonManager* Ogre::Singleton<PythonManager>::ms_Singleton = 0;

PythonManager* PythonManager::getSingletonPtr(void)
{
    return ms_Singleton;
}

PythonManager& PythonManager::getSingleton(void)
{
    assert(ms_Singleton);
    return *ms_Singleton;
}


// Deleter
void ScriptThreadDeleter(ScriptThread* th)
{
    delete th;
}

// Predicate
bool ScriptThreadIsTerminated(ScriptThread* th)
{
    bool result = th->isTerminated();
    if (result)
    {
        delete th;
    }
    return result;
};


/**
    Init Python engine & declare functions
*/
PythonManager::PythonManager()
{
    string dummy;

	Py_SetPythonHome(L"GameData/Scripts");
	PyImport_AppendInittab("_dungeonhack", PyInit__dungeonhack);
    Py_Initialize();
    PyEval_InitThreads();
    PyEval_ReleaseLock();

    m_mainThread = ScriptThread::createForeg();
    m_mainThread->init(dummy);

    linkFunctions();
}


/**
    Cleanup Python engine
*/
PythonManager::~PythonManager()
{
    for_each(m_threads.begin(), m_threads.end(), ScriptThreadDeleter);
    m_threads.clear();
    delete m_mainThread;

    Py_Finalize();
}


/**
    Suspend the calling thread - must be called by a Script thread
*/
void PythonManager::suspendThread()
{
    if (m_mainThread->isCaller())
    {
        m_mainThread->pause();
    }
    else
    {
        for (list<ScriptThread*>::reverse_iterator iter = m_threads.rbegin(); iter != m_threads.rend(); iter++)
        {
            if ((*iter)->isCaller())
            {
                (*iter)->pause();
                break;
            }
        }
    }
}


/**
    Check if any thread is waiting
*/
bool PythonManager::isWaiting()
{
    if (m_mainThread->isPaused())
    {
        return true;
    }
    else
    {
        for (list<ScriptThread*>::reverse_iterator iter = m_threads.rbegin(); iter != m_threads.rend(); iter++)
        {
            if ((*iter)->isPaused())
            {
                return true;
            }
        }
    }
    return false;
}


/**
    Resume latest blocked thread
*/
void PythonManager::resumeThread()
{
    if (m_mainThread->isPaused())
    {
        m_mainThread->resume();
    }
    else
    {
        for (list<ScriptThread*>::reverse_iterator iter = m_threads.rbegin(); iter != m_threads.rend(); iter++)
        {
            if ((*iter)->isPaused())
            {
                (*iter)->resume();
                break;
            }
        }
    }
}


/**
    Verify that any pending script is cleaned when appropriate
*/
void PythonManager::Update()
{
    m_threads.remove_if(ScriptThreadIsTerminated);
}


/**
    Execute a single command
    @param string
*/
void PythonManager::runCommand(string cmd)
{
    string cmdz(cmd);
    cmdz.append("\n");
    m_mainThread->enque(cmdz);
    m_lastRunScriptFile = "<console>";
}


/**
    Load and execute a script (internal)

    @param string filename
    @param ScriptThread*& thread : if NULL, create a One shot thread, else enque the request to the foreground thread
    @return bool true if loading was successful (regardless of the actual result of the script)
*/
bool PythonManager::loadScript(string filename, ScriptThread* thread)
{
    string fname;
    string contents;

	/*fname = GameWorld::getSingletonPtr()->m_gameDataPath + "Scripts/" + filename;
    contents = "execfile(\"";
    contents.append(fname);
    contents.append("\")\n");*/

	fname = GameWorld::getSingletonPtr()->m_gameDataPath + "Scripts/" + filename;
	contents = "exec(open(\"";
	contents.append(fname);
	contents.append("\").read())\n");

    if (!thread)
    {
        ScriptThread* th = ScriptThread::createOneShot();
        m_threads.push_back(th);
        th->init(contents);
    }
    else
    {
        thread->enque(contents);
    }
    return true;
}


/**
    Try to execute a script
    @param string filename
    @return bool true if loading was successful (regardless of the actual result of the script)
*/
bool PythonManager::runScript(string filename)
{
    bool retVal = loadScript(filename, NULL);
    m_lastRunScriptFile = filename;
    return retVal;
}


/**
    Call Python Functions from C++
*/
void PythonManager::callFunction(string filename, string function)
{
    string func_call(function);
    func_call.append("()\n");

    if (filename != m_lastRunScriptFile)
    {
        loadScript(filename, m_mainThread);
        m_lastRunScriptFile = filename;
    }

    m_mainThread->enque(func_call);
}


/**
    Call functions and pass in a quest
*/
void PythonManager::callQuestFunction(string filename, string function, Quest * theQuest)
{
    string func_call(function);
    func_call.append("()\n");

    // TODO: pass the quest argument directly to the Python objects
    string getQuestString = "this = getQuest(\"";
    getQuestString += theQuest->m_Name;
    getQuestString += "\")\n";

    // TODO: have a separate directory for quests ? (Quests/ is actually not used)
    if (filename != m_lastRunScriptFile)
    {
        m_mainThread->enque(getQuestString);
        loadScript(filename, m_mainThread);
        m_lastRunScriptFile = filename;
    }

    m_mainThread->enque(func_call);
}

void PythonManager::callQuestFunction(string filename, string function, Quest * theQuest, int value)
{
    // TODO: use VARARGS
    char theNum[15];
    string func_call(function);

    func_call.append("(");
    itoa(value, theNum, 10);
    func_call += theNum;
    func_call.append(")\n");

    // TODO: pass the quest argument directly to the Python objects
    string getQuestString = "this = getQuest(\"";
    getQuestString += theQuest->m_Name;
    getQuestString += "\")\n";

    // TODO: have a separate directory for quests ? (Quests/ is actually not used)
    if (filename != m_lastRunScriptFile)
    {
        m_mainThread->enque(getQuestString);
        loadScript(filename, m_mainThread);
        m_lastRunScriptFile = filename;
    }

    m_mainThread->enque(func_call);
}


void PythonManager::linkFunctions()
{
    loadScript("internals.py", m_mainThread);
    loadScript("dungeonhack.py", m_mainThread);
}
