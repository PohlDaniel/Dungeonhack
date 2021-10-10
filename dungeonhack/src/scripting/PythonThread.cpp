#include "DungeonHack.h"
#include "PythonThread.h"

#include <Python.h>
#include "ThreadMacros.h"
#include "PythonThreadImpl.h"


/**
    Thread creation (internal)
*/
void ScriptThreadImpl::_init(THREAD_FUNC th_func, string& sc_func)
{
    if (hThread)
        return;

    m_scriptFunction.assign(sc_func);

    hSemaphoreQuit = CREATE_SEMAPHORE(0);
    hSemaphorePause = CREATE_SEMAPHORE(0);
    m_paused = false;
    
    pyState = NULL;
    hThread = CREATE_THREAD(th_func, this, idThread);
}


/**
    Cleanup the script thread
*/
void ScriptThreadImpl::quit()
{
    m_terminated = true;
    resume();
    
    if (hThread)
    {
        if ( ! DEC_SEMAPHORE(hSemaphoreQuit, SCRIPT_THREAD_TIMEOUT * SCRIPT_THREAD_INTERVAL) )
        {
            // FIXME: not Python-safe
            TERMINATE_THREAD(hThread);
        }
            
        DESTROY_THREAD(hThread);
        DESTROY_SEMAPHORE(hSemaphoreQuit);
        DESTROY_SEMAPHORE(hSemaphorePause);
    }
    
    m_scriptFunction.clear();
}


/**
    Suspend the thread
    <!> To ensure consistency this function must be called from inside the thread
*/
void ScriptThreadImpl::pause()
{
    assert(!m_paused);
    assert(isCaller());

    m_paused = true;
	pyState = PyEval_SaveThread();
    DEC_SEMAPHORE(hSemaphorePause);

    // We just resumed
    PyEval_RestoreThread(pyState);
    m_paused = false;
}


/**
    Allow the designated thread to resume
*/
void ScriptThreadImpl::resume()
{
    if (m_paused)
    {
        INC_SEMAPHORE(hSemaphorePause);
    }
}


/**
    Check if we are the current thread
*/
bool ScriptThreadImpl::isCaller() const
{
    return COMPARE_THREAD_ID(idThread, GET_THREAD_ID());
}
