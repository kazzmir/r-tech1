#include <pthread.h>
#include "thread.h"

namespace Util{

Thread::Thread():
done(false){
    pthread_mutex_init(&doneLock, NULL);
}

void Thread::start(void * (*thread)(void *), void * arg){
    done = false;
    pthread_create(&this->thread, NULL, thread, arg);
}

bool Thread::isRunning(){
    pthread_mutex_lock(&doneLock);
    bool what = done;
    pthread_mutex_unlock(&doneLock);
    return what;
}

void Thread::kill(){
    pthread_cancel(thread);
}

Thread::~Thread(){
    /* FIXME: Should we join the thread? */
    /* pthread_join(thread); */
}

}
