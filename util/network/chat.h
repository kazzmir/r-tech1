#ifndef _util_network_chat_h
#define _util_network_chat_h

#include "network.h"
#include "util/pointer.h"
#include "util/thread.h"

#include <string>
#include <vector>
#include <queue>

namespace Network{
namespace Chat{

class Message{
public:
    enum Type{
        Ping,
        Chat,
        Command,
        Unknown,
    };
    Message();
    Message(Socket socket);
    Message(const Type &, const std::string &, const std::string &);
    Message(const Message &);
    virtual ~Message();
    
    const Message & operator=(const Message &);
    
    virtual void read(Socket socket);
    
    virtual void send(Socket socket);
    
    virtual const Type & getType() const;
    
    virtual const std::string & getName() const;
    
    virtual const std::string & getMessage() const;
    
    virtual void setParameters(const std::vector<std::string> &);
    
    virtual const std::vector<std::string> & getParameters() const;
    
protected:
    Type type;
    std::string sender;
    std::string message;
    std::vector<std::string> parameters;
};
   
class Client{
public:
    Client(int id, Network::Socket socket);
    
    virtual ~Client();
    
    virtual void run();
    
    virtual int getId() const;
    
    virtual void sendMessage(const Message &);
    
    virtual bool hasMessages() const;
    
    virtual Message nextMessage() const;
    
    virtual void shutdown();
    
    virtual bool isValid() const;
    
private:
    int id;
    Network::Socket socket;
    ::Util::Thread::Id thread;
    ::Util::Thread::LockObject lock;
    bool end;
    mutable std::queue<Message> messages;
    bool valid;
};

}
}
#endif