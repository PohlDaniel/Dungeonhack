#ifndef _PYTHON_THREAD_MACROS_H
#define _PYTHON_THREAD_MACROS_H

#define SEMAPHORE_MAX_VALUE     1


#ifdef WIN32

#include <windows.h>

#define THREAD_FUNC                 LPTHREAD_START_ROUTINE
#define THREAD_HANDLE               HANDLE
#define THREAD_ID                   DWORD
#define DECLARE_THREAD_FUNC(f,a)    DWORD WINAPI f ( LPVOID a )
#define CAST_THREAD_RETURN(x)       (DWORD)(x)
#define TEST_CANCEL_THREAD
#define SEMAPHORE_HANDLE            HANDLE
#define SLEEP(x)                    Sleep(x)

inline static THREAD_HANDLE CREATE_THREAD(THREAD_FUNC th_func, void* arg, THREAD_ID& tid)
{
    HANDLE hThread = CreateThread(
        NULL,               // default security attributes
        0,                  // use default stack size
        th_func,            // thread function name
        arg,                // argument to thread function
        0,                  // use default creation flags
        &tid                // returns the thread identifier
    );
    assert(hThread != NULL);
    return hThread;
}

inline static void DESTROY_THREAD(THREAD_HANDLE& th)
{
    CloseHandle(th);
    th = NULL;
}

inline static void TERMINATE_THREAD(THREAD_HANDLE th)
{
    TerminateThread(th, 0);
}

inline static THREAD_ID GET_THREAD_ID()
{
    return GetCurrentThreadId();
}

inline static bool COMPARE_THREAD_ID(THREAD_ID id1, THREAD_ID id2)
{
    return (id1 == id2);
}

inline static SEMAPHORE_HANDLE CREATE_SEMAPHORE(int init_value)
{
    HANDLE hSemaphore = CreateSemaphore(
        NULL,                       // default security attributes
        init_value,                 // initial value
        SEMAPHORE_MAX_VALUE,        // maximum value
        NULL                        // name
    );
    assert(hSemaphore != NULL);
    return hSemaphore;
}

inline static void DESTROY_SEMAPHORE(SEMAPHORE_HANDLE& sem)
{
    CloseHandle(sem);
    sem = NULL;
}

inline static void INC_SEMAPHORE(SEMAPHORE_HANDLE sem)
{
    LONG lOldValue;
    ReleaseSemaphore(sem, 1, &lOldValue);
}

inline static bool DEC_SEMAPHORE(SEMAPHORE_HANDLE sem, int msDelay = -1)
{
    DWORD retVal = WaitForSingleObject(sem, (msDelay <= 0) ? INFINITE : msDelay);
    return (retVal == 0);
}

#else

#include <pthread.h>

typedef struct
{
    pthread_mutex_t     mutex;
    int                 count;
}
pthread_s;

typedef void* (*THREAD_FUNC)(void*);
#define THREAD_HANDLE               pthread_t
#define THREAD_ID                   pthread_t
#define DECLARE_THREAD_FUNC(f,a)    void* f ( void* a )
#define CAST_THREAD_RETURN(x)       (void*)(x)
#define TEST_CANCEL_THREAD          pthread_testcancel();
#define SEMAPHORE_HANDLE            pthread_s*
#define SLEEP(x)                    usleep(1000 * (x))

inline static THREAD_HANDLE CREATE_THREAD(THREAD_FUNC th_func, void* arg, THREAD_ID& tid)
{
    pthread_t hThread;
    int result = pthread_create(
        &hThread,               // pointer to pthread structure
        NULL,                   // use default thread attributes
        th_func,                // thread function name
        arg                     // argument to thread function
    );
    assert(result == 0);
    tid = hThread;
    return hThread;
}

inline static void DESTROY_THREAD(THREAD_HANDLE& th)
{
    pthread_join(th, NULL);
    th = NULL;
}

inline static void TERMINATE_THREAD(THREAD_HANDLE th)
{
    pthread_cancel(th);
}

inline static THREAD_ID GET_THREAD_ID()
{
    return pthread_self();
}

inline static bool COMPARE_THREAD_ID(THREAD_ID id1, THREAD_ID id2)
{
    return (pthread_equal(id1, id2) != 0);
}

inline static SEMAPHORE_HANDLE CREATE_SEMAPHORE(int init_value)
{
    pthread_s* sem = (pthread_s*) malloc(sizeof(pthread_s));
    assert(sem);
    pthread_mutex_init(&(sem->mutex), NULL);
    sem->count = init_value;
    return sem;
}

inline static void DESTROY_SEMAPHORE(SEMAPHORE_HANDLE& sem)
{
    if (sem)
    {
        pthread_mutex_destroy(&(sem->mutex));
        free(sem);
        sem = NULL;
    }
}

inline static void INC_SEMAPHORE(SEMAPHORE_HANDLE sem)
{
    pthread_mutex_lock(&(sem->mutex));
    assert(sem->count < SEMAPHORE_MAX_VALUE);
    sem->count++;
    pthread_mutex_unlock(&(sem->mutex));
}

inline static bool DEC_SEMAPHORE(SEMAPHORE_HANDLE sem, int msDelay = -1)
{
    int timeout = -1, released = 0;
    struct timespec tm;

    tm.tv_sec = 0;
    tm.tv_nsec = 10000000; // 10ms
    if (msDelay > 0)
    {
        timeout = msDelay / 10;
    }

    do {
        pthread_mutex_lock(&(sem->mutex));
        if (sem->count > 0)
        {
            sem->count--;
            released = 1;
        }
        pthread_mutex_unlock(&(sem->mutex));
        
        if (!released)
        {
            nanosleep(&tm, NULL);
        }
        
        if (timeout != -1)
        {
            timeout--;
        }
    }
    while ( !released && ((timeout == -1) || (timeout > 0)) );
    
    return (released == 1);
}

#endif


#endif
