// #include <pthread.h>
#include "thread.h"

namespace Util{

namespace Thread{
    
void initializeLock(Lock * lock){
    pthread_mutex_init(lock, NULL);
}

void acquireLock(Lock * lock){
    pthread_mutex_lock(lock);
}

void releaseLock(Lock * lock){
    pthread_mutex_unlock(lock);
}

bool createThread(Id * thread, void * attributes, ThreadFunction function, void * arg){
    return pthread_create(thread, (pthread_attr_t*) attributes, function, arg) == 0;
}

void joinThread(Id thread){
    pthread_join(thread, NULL);
}
    
void cancelThread(Id thread){
    /* FIXME: cancel is not implemented for libogc, find another way.
     * thread suspend/resume is there, though.
     */
#if !defined(WII)
    pthread_cancel(thread);
#endif
}

}

WaitThread::WaitThread():
done(false){
    Thread::initializeLock(&doneLock);
}

WaitThread::WaitThread(void * (*thread)(void*), void * arg){
    Thread::initializeLock(&doneLock);
    start(thread, arg);
}

static void * do_thread(void * arg){
    WaitThread * thread = (WaitThread *) arg;
    thread->doRun();
    return NULL;
}

void WaitThread::doRun(){
    this->function(this->arg);

    Thread::acquireLock(&doneLock);
    this->done = true;
    Thread::releaseLock(&doneLock);
}

void WaitThread::start(Thread::ThreadFunction thread, void * arg){
    done = false;
    this->arg = arg;
    this->function = thread;
    Thread::createThread(&this->thread, NULL, do_thread, this);
}

bool WaitThread::isRunning(){
    Thread::acquireLock(&doneLock);
    bool what = done;
    Thread::releaseLock(&doneLock);
    return what;
}

void WaitThread::kill(){
    Thread::cancelThread(thread);
    Thread::joinThread(thread);
}

WaitThread::~WaitThread(){
    /* FIXME: Should we join the thread? */
    /* pthread_join(thread); */
}

ThreadBoolean::ThreadBoolean(volatile bool & what, Thread::Lock & lock):
what(what),
lock(lock){
}

bool ThreadBoolean::get(){
    Thread::acquireLock(&lock);
    bool b = what;
    Thread::releaseLock(&lock);
    return b;
}

void ThreadBoolean::set(bool value){
    Thread::acquireLock(&lock);
    what = value;
    Thread::releaseLock(&lock);
}

}
