#include "thread.h"

namespace Util{

namespace Thread{

LockObject::LockObject(){
    initializeLock(&lock);
}

void LockObject::acquire() const {
    /* quick hack to get around annoying constness */
    acquireLock((Lock*) &lock);
}

void LockObject::release() const {
    releaseLock((Lock*) &lock);
}

LockObject::~LockObject(){
    destroyLock(&lock);
}

ScopedLock::ScopedLock(const LockObject & lock):
lock(lock){
    lock.acquire();
}

ScopedLock::~ScopedLock(){
    lock.release();
}
    
bool isUninitialized(Id thread){
    return thread == uninitializedValue;
}

#ifdef USE_SDL
Id uninitializedValue = NULL;
    
void initializeLock(Lock * lock){
    *lock = SDL_CreateMutex();
}

int acquireLock(Lock * lock){
    return SDL_LockMutex(*lock);
}

int releaseLock(Lock * lock){
    return SDL_UnlockMutex(*lock);
}

void destroyLock(Lock * lock){
    SDL_DestroyMutex(*lock);
}

void initializeSemaphore(Semaphore * semaphore, unsigned int value){
    *semaphore = SDL_CreateSemaphore(value);
}

void destroySemaphore(Semaphore * semaphore){
    SDL_DestroySemaphore(*semaphore);
}

void semaphoreDecrease(Semaphore * semaphore){
    SDL_SemWait(*semaphore);
}

void semaphoreIncrease(Semaphore * semaphore){
    SDL_SemPost(*semaphore);
}

bool createThread(Id * thread, void * attributes, ThreadFunction function, void * arg){
    *thread = SDL_CreateThread(function, arg);
    return *thread != NULL;
}

void joinThread(Id thread){
    SDL_WaitThread(thread, NULL);
}

void cancelThread(Id thread){
#ifndef PS3
    SDL_KillThread(thread);
#endif
}

#else
Id uninitializedValue = 0;

void initializeLock(Lock * lock){
    pthread_mutex_init(lock, NULL);
}

int acquireLock(Lock * lock){
    return pthread_mutex_lock(lock);
}

int releaseLock(Lock * lock){
    return pthread_mutex_unlock(lock);
}

void initializeSemaphore(Semaphore * semaphore, unsigned int value){
    sem_init(semaphore, 0, value);
}

void destroySemaphore(Semaphore * semaphore){
    /* nothing */
}

void semaphoreDecrease(Semaphore * semaphore){
    sem_wait(semaphore);
}

void semaphoreIncrease(Semaphore * semaphore){
    sem_post(semaphore);
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
    
void destroyLock(Lock * lock){
    /* nothing */
}

#endif

}

WaitThread::WaitThread():
done(false){
    Thread::initializeLock(&doneLock);
}

WaitThread::WaitThread(Thread::ThreadFunction thread, void * arg){
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
    Thread::createThread(&this->thread, NULL, (Thread::ThreadFunction) do_thread, this);
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
    Thread::joinThread(thread);
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
