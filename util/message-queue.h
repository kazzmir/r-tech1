#ifndef _paintown_message_queue_h
#define _paintown_message_queue_h

#include <queue>
#include <string>
#include "thread.h"

namespace Global{
    extern Util::Thread::Lock messageLock;
}

/* multithreaded message queue.
 * someone puts messages in, someone takes them out.
 * FIFO order
 */
class MessageQueue{
public:
    MessageQueue();

    /* push on */
    void add(const std::string & str);
    /* true if any messages are left */
    bool hasAny();
    /* get the first message */
    std::string get();

    virtual ~MessageQueue();
private:
    std::queue<std::string> messages;
    Util::Thread::Lock lock;
};

#endif
