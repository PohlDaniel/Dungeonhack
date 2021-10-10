#include "DungeonHack.h"
#include "PythonThread.h"

#include <Python.h>
#include "ThreadMacros.h"
#include "PythonThreadImpl.h"


/**
    Returns an instance of the background script
    @return ScriptThread*
*/
ScriptThread* ScriptThread::createBackg()
{
    return (ScriptThread*) new ScriptThreadBackg();
}

/**
    Returns an instance of the one shot script
    @return ScriptThread*
*/
ScriptThread* ScriptThread::createOneShot()
{
    return (ScriptThread*) new ScriptThreadOneshot();
}

/**
    Returns an instance of the one shot script
    @return ScriptThread*
*/
ScriptThread* ScriptThread::createForeg()
{
    return (ScriptThread*) new ScriptThreadForeg();
}


/**
    Python background thread function
    The scripts is executed in a loop until it terminates explicitely
    Note: m_scriptFunction is used as entry point and must be a simple function call
    
    @param void* arg 
    @return integer
*/
DECLARE_THREAD_FUNC( ScriptThreadBackg::threadFunction, arg )
{
    int ret = 0, retVal = 0;
    ScriptThreadBackg* pThis = (ScriptThreadBackg*) arg;

    if (arg)
    {
        while (!pThis->m_terminated)
        {
            TEST_CANCEL_THREAD
            
            PyGILState_STATE gstate;
            gstate = PyGILState_Ensure();
            ret = PyRun_SimpleString(pThis->m_scriptFunction.c_str());
            PyGILState_Release(gstate);
            
            if (ret)
            {
                // this allows scripts to quit themselves
                pThis->m_terminated = true;
            }
            else
            {
                // Block scripts from running recklessly
                SLEEP(SCRIPT_THREAD_INTERVAL);
            }
        }
    }
    
    INC_SEMAPHORE(pThis->hSemaphoreQuit);
    return CAST_THREAD_RETURN(retVal);
}


/**
    Python One Shot function
    The script executes only once
    This has been done in a thread in order to allow suspend
    Note: m_scriptFunction is used as entry point and must be a simple function call
    
    @param void* arg 
    @return integer
*/
DECLARE_THREAD_FUNC( ScriptThreadOneshot::oneShotFunction, arg )
{
    int retVal = 0;
    ScriptThreadOneshot* pThis = (ScriptThreadOneshot*) arg;

    TEST_CANCEL_THREAD
    
    if (arg)
    {
        PyGILState_STATE gstate;
        gstate = PyGILState_Ensure();
        PyRun_SimpleString(pThis->m_scriptFunction.c_str());
        PyGILState_Release(gstate);

        pThis->m_terminated = true;
        INC_SEMAPHORE(pThis->hSemaphoreQuit);
    }
    
    return CAST_THREAD_RETURN(retVal);
}


/**
    Python Foreground function
    Allow to execute a series of scripts each after
    This has been done to limit number of thread spawns
    and ensures that some function calls are done in correct order.
    Note: m_scriptFunction is used as entry point and must be a simple function call
    
    @param void* arg 
    @return integer
*/
DECLARE_THREAD_FUNC( ScriptThreadForeg::mainFunction, arg )
{
    int retVal = 0;
    ScriptThreadForeg* pThis = (ScriptThreadForeg*) arg;

    TEST_CANCEL_THREAD
    
    if (arg)
    {
        while (!pThis->m_terminated)
        {
            TEST_CANCEL_THREAD
            
            if (!pThis->m_functions.empty())
            {
                pThis->m_scriptFunction = pThis->m_functions.back();
                pThis->m_functions.pop_back();

                PyGILState_STATE gstate;
                gstate = PyGILState_Ensure();
                PyRun_SimpleString(pThis->m_scriptFunction.c_str());
                PyGILState_Release(gstate);

                // Wait for next command to enque
                pThis->m_scriptFunction.clear();
            }
            
            if (!pThis->m_terminated)
            {
                // Block scripts from running recklessly
                SLEEP(SCRIPT_THREAD_INTERVAL);
            }
        }
    }
    
    return CAST_THREAD_RETURN(retVal);
}
