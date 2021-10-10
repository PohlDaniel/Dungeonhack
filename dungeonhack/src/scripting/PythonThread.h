#ifndef _PYTHON_THREAD_H
#define _PYTHON_THREAD_H

#include <string>
using namespace std;


// Interval between 2 runs of a background script, in ms
#define SCRIPT_THREAD_INTERVAL  100

// Number of tries to terminate gracefully a script
#define SCRIPT_THREAD_TIMEOUT   500


/**
    Thread Script definition
    Implementation details are hidden in order to not expose low level stuff to other files
*/
class ScriptThread
{
public:
    ScriptThread() : m_paused(false), m_terminated(false)  { }
    virtual ~ScriptThread() { }

    // Factory
    static ScriptThread* createBackg();
    static ScriptThread* createOneShot();
    static ScriptThread* createForeg();
    
    // Thread API
    virtual void init(string& function) = 0;
    virtual void quit() = 0;
    virtual void pause()  = 0;
    virtual void resume()  = 0;
    virtual bool isCaller() const = 0;

    // Accessors
    bool isPaused() const { return m_paused; }
    bool isTerminated() const { return m_terminated; }

    // Foreground thread specific
    virtual void enque(string) { };
    virtual bool isBusy() const { return true; }

protected:
    bool    m_paused;
    bool    m_terminated;
    string  m_scriptFunction;
};


#endif // _PYTHON_THREAD_H
