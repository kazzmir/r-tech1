#ifndef _paintown_thread_h
#define _paintown_thread_h

#include <pthread.h>

namespace Util{

class WaitThread{
public:
    /* does not start a new thread yet */
    WaitThread();

    /* starts a thread */
    WaitThread(void * (*thread)(void*), void * arg);

    /* starts a thread */
    void start(void * (*thread)(void *), void * arg);

    bool isRunning();
    void kill();

    virtual ~WaitThread();

public:
    /* actually runs the thread */
    void doRun();

protected:
    pthread_mutex_t doneLock;
    pthread_t thread;
    volatile bool done;
    void * arg;
    void * (*function)(void *);
};

/* wraps a boolean with lock/unlock while checking/setting it */
class ThreadBoolean{
public:
    ThreadBoolean(volatile bool & what, pthread_mutex_t & lock);

    bool get();
    void set(bool value);

protected:
    volatile bool & what;
    pthread_mutex_t & lock;
};

}

#endif
