#ifndef _paintown_thread_h
#define _paintown_thread_h

#include <pthread.h>

namespace Util{

/* Either uses pthreads or SDL_thread */
namespace Thread{
    typedef pthread_mutex_t Lock;
    typedef pthread_t Id;
    typedef void * (*ThreadFunction)(void*);

    void initializeLock(Lock * lock);
    void acquireLock(Lock * lock);
    void releaseLock(Lock * lock);
    void createThread(Id * thread, void * attributes, ThreadFunction function, void * arg);
    void joinThread(Id thread);
    void cancelThread(Id thread);
}

class WaitThread{
public:
    /* does not start a new thread yet */
    WaitThread();

    /* starts a thread */
    WaitThread(Thread::ThreadFunction thread, void * arg);

    /* starts a thread */
    void start(Thread::ThreadFunction thread, void * arg);

    bool isRunning();
    void kill();

    virtual ~WaitThread();

public:
    /* actually runs the thread */
    void doRun();

protected:
    Thread::Lock doneLock;
    Thread::Id thread;
    volatile bool done;
    void * arg;
    Thread::ThreadFunction function;
};

/* wraps a boolean with lock/unlock while checking/setting it */
class ThreadBoolean{
public:
    ThreadBoolean(volatile bool & what, Thread::Lock & lock);

    bool get();
    void set(bool value);

protected:
    volatile bool & what;
    Thread::Lock & lock;
};

}

#endif
