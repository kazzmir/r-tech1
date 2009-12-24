#include "timer.h"
#include "init.h"
#include "funcs.h"

namespace Util{

void * do_wait(void * timer_){
    Timer * timer = (Timer *) timer_;
    unsigned int now = Global::second_counter;
    /* add 1 because the current time is in between X and X+1 */
    while (Global::second_counter - now < timer->wait+1){
        Util::rest(50);
        bool do_stop = false;
        pthread_mutex_lock(&timer->lock);
        do_stop = timer->stopped;
        pthread_mutex_unlock(&timer->lock);
        if (do_stop){
            return NULL;
        }
    }
    timer->func(timer->arg);
    return NULL;
}

Timer::Timer(unsigned int seconds_to_wait, timeout func, void * arg):
wait(seconds_to_wait),
func(func),
arg(arg),
stopped(false){
    pthread_mutex_init(&lock, NULL);
    pthread_create(&timer, NULL, do_wait, this);
}

void Timer::stop(){
    pthread_mutex_lock(&lock);
    stopped = true;
    pthread_mutex_unlock(&lock);
    pthread_join(timer, NULL);
}

}
