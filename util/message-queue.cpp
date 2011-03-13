#include <queue>
#include <string>
#include "message-queue.h"
#include "thread.h"
    
Util::Thread::Lock Global::messageLock;

MessageQueue::MessageQueue(){
    Util::Thread::initializeLock(&lock);
}

void MessageQueue::add(const std::string & str){
    Util::Thread::acquireLock(&lock);
    messages.push(str);
    Util::Thread::releaseLock(&lock);
}

bool MessageQueue::hasAny(){
    bool any = false;
    Util::Thread::acquireLock(&lock);
    any = messages.size() > 0;
    Util::Thread::releaseLock(&lock);
    return any;
}

std::string MessageQueue::get(){
    std::string str;
    Util::Thread::acquireLock(&lock);
    if (messages.size() > 0){
        str = messages.front();
        messages.pop();
    }
    Util::Thread::releaseLock(&lock);
    return str;
}

MessageQueue::~MessageQueue(){
}
