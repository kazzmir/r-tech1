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
#include "load_exception.h"
#include "token_exception.h"
#include "mugen/exception.h"
#include "debug.h"

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

    extern Id uninitializedValue;
    bool isUninitialized(Id thread);
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

/* Computes stuff in a separate thread and gives it back when you ask for it.
 * As soon as the future is created a thread will start executing and compute
 * whatever it is that the class is supposed to do. You can then call `get'
 * on the future object to get the result. If the thread is still executing
 * then `get' will block until the future completes. If the future has already
 * completed then `get' will return immediately with the computed value.
 * The use case is computing something that has to be used later:
 *  Future future; // might take a while to compute
 *  do_stuff_that_takes_a_while(); // future might finish sometime in here
 *  Object o = future.get(); // future is already done
 *
 * TODO: handle exceptions
 */
template<class X>
class Future{
protected:
    /* WARNING: hack to find out the type of the exception */
    /*
    enum ExceptionType{
        None,
        Load,
        Token,
        Base,
        Mugen
    };
    */

public:
    Future():
    thing(0),
    thread(Thread::uninitializedValue),
    exception(NULL){
        /* future will increase the count */
        Thread::initializeSemaphore(&future, 0);
    }

    virtual ~Future(){
        if (Thread::isUninitialized(thread)){
            Thread::joinThread(thread);
        }
        Thread::destroySemaphore(&future);
        delete exception;
    }

    virtual X get(){
        X out;
        Thread::semaphoreDecrease(&future);
        if (exception != NULL){
            exception->throwSelf();
        }
        out = thing;
        Thread::semaphoreIncrease(&future);
        return out;
    }

    virtual void start(){
        if (!Thread::createThread(&thread, NULL, (Thread::ThreadFunction) runit, this)){
            Global::debug(0) << "Could not create future thread. Blocking until its done" << std::endl;
            runit(this);
            // throw Exception::Base(__FILE__, __LINE__);
        }
    }

protected:
    static void * runit(void * arg){
        Future<X> * me = (Future<X>*) arg;
        try{
            me->compute();
        } catch (const LoadException & load){
            me->exception = new LoadException(load);
        } catch (const TokenException & t){
            me->exception = new TokenException(t);
        } catch (const MugenException & m){
            me->exception = new MugenException(m);
        } catch (const Exception::Base & base){
            me->exception = new Exception::Base(base);
        }
        Thread::semaphoreIncrease(&me->future);
        return NULL;
    }

    virtual void set(X x){
        this->thing = x;
    }


    virtual void compute() = 0;

    X thing;
    Thread::Id thread;
    Thread::Semaphore future;
    /* if any exceptions occur, throw them from `get' */
    Exception::Base * exception;
};

}

#endif
