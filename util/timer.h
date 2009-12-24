#ifndef _paintown_util_timer_h
#define _paintown_util_timer_h

#include <pthread.h>

namespace Util{

/* calls a function after X seconds unless stop() is called first */
class Timer{
public:
    typedef void (*timeout)(void * arg);
    Timer(unsigned int seconds_to_wait, timeout func, void * arg);

    void stop();

    friend void * do_wait(void * timer_);

protected:
    unsigned int wait;
    timeout func;
    void * arg;
    bool stopped;
    pthread_mutex_t lock;
    pthread_t timer;
};

}

#endif
