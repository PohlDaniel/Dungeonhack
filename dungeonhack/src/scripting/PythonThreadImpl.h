#ifndef _PYTHON_THREAD_IMPL_H
#define _PYTHON_THREAD_IMPL_H


/**
    Script thread implementation
*/
class ScriptThreadImpl : public ScriptThread
{
public:
    ScriptThreadImpl() : ScriptThread(), hThread(NULL), idThread(0), hSemaphorePause(NULL), hSemaphoreQuit(NULL) {}
    virtual ~ScriptThreadImpl() { quit(); };
    
    virtual void init(string& function) = 0;
    virtual void quit();
    virtual void pause();
    virtual void resume();
    virtual bool isCaller() const;

protected:
    void _init(THREAD_FUNC f, string& s);

    THREAD_HANDLE       hThread;
    THREAD_ID           idThread;
    PyThreadState*      pyState;
    SEMAPHORE_HANDLE    hSemaphorePause;
    SEMAPHORE_HANDLE    hSemaphoreQuit;
};


/**
    Background script implementation
*/
class ScriptThreadBackg : public ScriptThreadImpl
{
public:
    ScriptThreadBackg() : ScriptThreadImpl() {}
    virtual ~ScriptThreadBackg() { };
    
    virtual void init(string& function) { _init(threadFunction, function); }

protected:
    static DECLARE_THREAD_FUNC ( threadFunction, arg );
};


/**
    One shot script implementation
*/
class ScriptThreadOneshot : public ScriptThreadImpl
{
public:
    ScriptThreadOneshot() : ScriptThreadImpl() {}
    virtual ~ScriptThreadOneshot() {}
    
    virtual void init(string& function) { _init(oneShotFunction, function); }
    
protected:
    static DECLARE_THREAD_FUNC ( oneShotFunction, arg );
};


/**
    Foreground script implementation
*/
class ScriptThreadForeg : public ScriptThreadImpl
{
public:
    ScriptThreadForeg() : ScriptThreadImpl() {}
    virtual ~ScriptThreadForeg() {}
    
    virtual void init(string& ) { string dummy; _init(mainFunction, dummy); }

    virtual void enque(string func) { m_functions.push_front(func); }
    virtual bool isBusy() { return !m_functions.empty(); }
    
protected:
    static DECLARE_THREAD_FUNC ( mainFunction, arg );

    list<string> m_functions;
};


#endif
