#include <pthread.h>
#include "thread.h"

namespace Util{

Thread::Thread():
done(false){
    pthread_mutex_init(&doneLock, NULL);
}

Thread::Thread(void * (*thread)(void*), void * arg){
    pthread_mutex_init(&doneLock, NULL);
    start(thread, arg);
}

static void * do_thread(void * arg){
    Thread * thread = (Thread *) arg;
    thread->doRun();
    return NULL;
}

void Thread::doRun(){
    this->function(this->arg);

    pthread_mutex_lock(&doneLock);
    this->done = true;
    pthread_mutex_unlock(&doneLock);
}

void Thread::start(void * (*thread)(void *), void * arg){
    done = false;
    this->arg = arg;
    this->function = thread;
    pthread_create(&this->thread, NULL, do_thread, this);
}

bool Thread::isRunning(){
    pthread_mutex_lock(&doneLock);
    bool what = done;
    pthread_mutex_unlock(&doneLock);
    return what;
}

void Thread::kill(){
    /* FIXME: cancel is not implemented for libogc, find another way.
     * thread suspend/resume is there, though.
     */
#if !defined(WII)
    pthread_cancel(thread);
#endif
    pthread_join(thread, NULL);
}

Thread::~Thread(){
    /* FIXME: Should we join the thread? */
    /* pthread_join(thread); */
}

ThreadBoolean::ThreadBoolean(volatile bool & what, pthread_mutex_t & lock):
what(what),
lock(lock){
}

bool ThreadBoolean::get(){
    pthread_mutex_lock(&lock);
    bool b = what;
    pthread_mutex_unlock(&lock);
    return b;
}

void ThreadBoolean::set(bool value){
    pthread_mutex_lock(&lock);
    what = value;
    pthread_mutex_unlock(&lock);
}

}
