#include <queue>
#include <pthread.h>
#include <string>
#include "message-queue.h"

MessageQueue::MessageQueue(){
    pthread_mutex_init(&lock, NULL);
}

void MessageQueue::add(const std::string & str){
    pthread_mutex_lock(&lock);
    messages.push(str);
    pthread_mutex_unlock(&lock);
}

bool MessageQueue::hasAny(){
    bool any = false;
    pthread_mutex_lock(&lock);
    any = messages.size() > 0;
    pthread_mutex_unlock(&lock);
    return any;
}

std::string MessageQueue::get(){
    std::string str;
    pthread_mutex_lock(&lock);
    if (messages.size() > 0){
        str = messages.front();
        messages.pop();
    }
    pthread_mutex_unlock(&lock);
    return str;
}

MessageQueue::~MessageQueue(){
}
