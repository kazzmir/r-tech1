#ifndef _util_network_irc_h
#define _util_network_irc_h

#include "network.h"
#include "chat.h"
#include "util/pointer.h"
#include "util/thread.h"

#include <string>
#include <vector>
#include <queue>

namespace Network{
namespace IRC{
    
    class Command{
    public:
        enum Type{
            Unknown,
            Pass,
            Nick,
            User,
            Server,
            Oper,
            Quit,
            Squit,
            Join,
            Part,
            Mode,
            Topic,
            Names,
            List,
            Invite,
            Kick,
            Version,
            Stats,
            Links,
            Time,
            Connect,
            Trace,
            Admin,
            Info,
            PrivateMessage,
            Notice,
            Who,
            Whois,
            Whowas,
            Kill,
            Ping,
            Pong,
            Error,
        };
        // Initializes it from an incoming message off of socket
        Command(const std::string &);
        // Create a message with owner and type
        Command(const std::string &, const Type &);
        Command(const Command &);
        virtual ~Command();
        
        virtual const Command & operator=(const Command &);
        
        virtual std::string getSendable() const;
        
        virtual inline const std::string & getOwner() const {
            return this->owner;
        }
        
        virtual inline const Type & getType() const {
            return this->type;
        }
        
        virtual inline void setParameters(const std::string & param1){
            this->parameters.clear();
            this->parameters.push_back(param1);
        }
        
        virtual inline void setParameters(const std::string & param1, const std::string & param2){
            this->parameters.clear();
            this->parameters.push_back(param1);
            this->parameters.push_back(param2);
        }
        
        virtual inline void setParameters(const std::string & param1, const std::string & param2, const std::string & param3){
            this->parameters.clear();
            this->parameters.push_back(param1);
            this->parameters.push_back(param2);
            this->parameters.push_back(param3);
        }
        
        virtual inline void setParameters(const std::string & param1, const std::string & param2, const std::string & param3, const std::string & param4){
            this->parameters.clear();
            this->parameters.push_back(param1);
            this->parameters.push_back(param2);
            this->parameters.push_back(param3);
            this->parameters.push_back(param4);
        }
        
        virtual inline void setParameters(const std::vector< std::string > & params){
            this->parameters = params;
        }
        
        virtual inline const std::vector< std::string > & getParameters() const {
            return this->parameters;
        }
        
    protected:
        std::string owner;
        Type type;
        std::vector< std::string > parameters;
    };

    class Client : public Chat::Threadable{
    public:
        Client(const std::string &, int port);
        virtual ~Client();
        
        virtual void connect();
        
        virtual void run();
        
        virtual bool hasCommands() const;

        virtual Command nextCommand() const;
        
        virtual void sendCommand(const Command &);
        
        virtual void setName(const std::string &);
        
        virtual void joinChannel(const std::string &);
        
        virtual void sendMessage(const std::string &);
        
        virtual void sendPong(const Command &);
        
    protected:
        std::string readMessage();
        
        Network::Socket socket;
        std::string username;
        std::string channel;
        std::string hostname;
        int port;
        bool end;
        mutable std::queue< Command > commands;
    };
    
}// end irc
}
#endif