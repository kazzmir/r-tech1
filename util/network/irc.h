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
            ErrorNickInUse,
            ErrorNoSuchNick,
            ErrorNoSuchChannel,
            ErrorNeedMoreParams,
            ErrorBannedFromChannel,
            ErrorInviteOnlyChannel,
            ErrorBadChannelKey,
            ErrorChannelIsFull,
            ReplyNames,
            ReplyNamesEndOf,
            ReplyNoTopic,
            ReplyTopic,
            ReplyTopicAuthor,
            ReplyMOTD,
            ReplyMOTDStart,
            ReplyMOTDEndOf,
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
        
        virtual inline bool hasCtcp() {
            return !this->ctcp.empty();
        }
        
        virtual inline const std::vector< std::string > & getCtcp() const {
            return this->ctcp;
        }
        
    protected:
        std::string owner;
        Type type;
        std::vector< std::string > parameters;
        std::vector< std::string > ctcp;
    };
    
    class Channel{
    public:
        Channel();
        Channel(const std::string &);
        Channel(const Channel &);
        ~Channel();
        
        const Channel & operator=(const Channel &);
        
        void addUser(const std::string &);
        void removeUser(const std::string &);
        void addUsers(const std::vector<std::string> &);
        
        inline const std::vector<std::string> & getUsers() const{
            return this->users;
        }
        
        inline const std::string & getName() const {
            return this->name;
        }
        
        inline void setTopic(const std::string & topic){
            this->topic = topic;
        }
        
        inline void setTopicAuthor(const std::string & topicAuthor, uint64_t topicDate){
            this->topicAuthor = topicAuthor;
            this->topicDate = topicDate;
        }
        
    protected:
        std::string name;
        std::string topic;
        std::string topicAuthor;
        uint64_t topicDate;
        std::vector<std::string> users;
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
        
        virtual void sendCommand(const Command::Type &);
        virtual void sendCommand(const Command::Type &, const std::string &);
        virtual void sendCommand(const Command::Type &, const std::string &, const std::string &);
        virtual void sendCommand(const Command::Type &, const std::string &, const std::string &, const std::string &);
        virtual void sendCommand(const Command::Type &, const std::string &, const std::string &, const std::string &, const std::string &);
        
        virtual void setName(const std::string &);
        
        virtual inline const std::string & getName() const {
            return this->username;
        }
        
        virtual void joinChannel(const std::string &);
        
        virtual inline const Channel & getChannel() const {
            return this->channel;
        }
        
        virtual void sendMessage(const std::string &);
        
        virtual void sendPong(const Command &);
        
    protected:
        std::string readMessage();
        
        //! Doesn't do anything to the command just handle some internal changes like username and channel stuff
        void checkResponseAndHandle(const Command &);
        
        Network::Socket socket;
        std::string previousUsername;
        std::string username;
        Channel previousChannel;
        Channel channel;
        std::string hostname;
        int port;
        bool end;
        mutable std::queue< Command > commands;
    };
    
}// end irc
}
#endif