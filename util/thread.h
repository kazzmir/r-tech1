#ifndef _paintown_thread_h
#define _paintown_thread_h

#ifdef USE_SDL
#include <SDL_thread.h>
#include <SDL_mutex.h>
#else
#include <pthread.h>
#include <semaphore.h>
#endif

#include "exceptions/exception.h"

namespace Util{

/* Either uses pthreads or SDL_thread */
namespace Thread{
#ifdef USE_SDL
    typedef SDL_mutex* Lock;
    typedef SDL_Thread* Id;
    typedef int (*ThreadFunction)(void*);
    typedef SDL_semaphore* Semaphore;
#else
    typedef pthread_mutex_t Lock;
    typedef pthread_t Id;
    typedef sem_t Semaphore;
    typedef void * (*ThreadFunction)(void*);
#endif

    void initializeLock(Lock * lock);

    void initializeSemaphore(Semaphore * semaphore, unsigned int value);
    void destroySemaphore(Semaphore * semaphore);
    void semaphoreDecrease(Semaphore * semaphore);
    void semaphoreIncrease(Semaphore * semaphore);

    int acquireLock(Lock * lock);
    int releaseLock(Lock * lock);
    void destroyLock(Lock * lock);
    bool createThread(Id * thread, void * attributes, ThreadFunction function, void * arg);
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

template<class X>
class Future{
public:
    Future():
    thing(0){
        /* future will increase the count */
        Thread::initializeSemaphore(&future, 0);
    }

    virtual ~Future(){
        Thread::joinThread(thread);
        Thread::destroySemaphore(&future);
    }

    virtual X get(){
        X out;
        Thread::semaphoreDecrease(&future);
        out = thing;
        Thread::semaphoreIncrease(&future);
        return out;
    }

protected:
    static void * runit(void * arg){
        Future<X> * me = (Future<X>*) arg;
        me->compute();
        Thread::semaphoreIncrease(&me->future);
        return NULL;
    }

    virtual void set(X x){
        this->thing = x;
    }

    virtual void start(){
        if (!Thread::createThread(&thread, NULL, (Thread::ThreadFunction) runit, this)){
            throw Exception::Base(__FILE__, __LINE__);
        }
    }

    virtual void compute() = 0;

    X thing;
    Thread::Id thread;
    Thread::Semaphore future;
};

}

#endif
