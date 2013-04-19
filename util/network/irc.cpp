#include "irc.h"

#include "util/font.h"
#include "util/regex.h"
#include "util/graphics/bitmap.h"
#include "util/configuration.h"

#include <stdexcept>

namespace Network{
namespace IRC{

static Command::Type convertCommand(const std::string & cmd){
    Command::Type command = Command::Unknown;
    if (cmd == "PASS"){
        command = Command::Pass;
    } else if (cmd == "NICK"){
        command = Command::Nick;
    } else if (cmd == "USER"){
        command = Command::User;
    } else if (cmd == "SERVER"){
        command = Command::Server;
    } else if (cmd == "OPER"){
        command = Command::Oper;
    } else if (cmd == "QUIT"){
        command = Command::Quit;
    } else if (cmd == "SQUIT"){
        command = Command::Squit;
    } else if (cmd == "JOIN"){
        command = Command::Join;
    } else if (cmd == "PART"){
        command = Command::Part;
    } else if (cmd == "MODE"){
        command = Command::Mode;
    } else if (cmd == "TOPIC"){
        command = Command::Topic;
    } else if (cmd == "NAMES"){
        command = Command::Names;
    } else if (cmd == "LIST"){
        command = Command::List;
    } else if (cmd == "INVITE"){
        command = Command::Invite;
    } else if (cmd == "KICK"){
        command = Command::Kick;
    } else if (cmd == "VERSION"){
        command = Command::Version;
    } else if (cmd == "STATS"){
        command = Command::Stats;
    } else if (cmd == "LINKS"){
        command = Command::Links;
    } else if (cmd == "TIME"){
        command = Command::Time;
    } else if (cmd == "CONNECT"){
        command = Command::Connect;
    } else if (cmd == "TRACE"){
        command = Command::Trace;
    } else if (cmd == "ADMIN"){
        command = Command::Admin;
    } else if (cmd == "INFO"){
        command = Command::Info;
    } else if (cmd == "PRIVMSG"){
        command = Command::PrivateMessage;
    } else if (cmd == "NOTICE"){
        command = Command::Notice;
    } else if (cmd == "WHO"){
        command = Command::Who;
    } else if (cmd == "WHOIS"){
        command = Command::Whois;
    } else if (cmd == "WHOWAS"){
        command = Command::Whowas;
    } else if (cmd == "KILL"){
        command = Command::Kill;
    } else if (cmd == "PING"){
        command = Command::Ping;
    } else if (cmd == "PONG"){
        command = Command::Pong;
    } else if (cmd == "ERROR"){
        command = Command::Error;
    } else if (cmd == "433"){
        command = Command::ErrorNickInUse;
    } else if (cmd == "401"){
        command = Command::ErrorNoSuchNick;
    } else if (cmd == "403"){
        command = Command::ErrorNoSuchChannel;
    } else if (cmd == "461"){
        command = Command::ErrorNeedMoreParams;
    } else if (cmd == "473"){
        command = Command::ErrorInviteOnlyChannel;
    } else if (cmd == "474"){
        command = Command::ErrorBannedFromChannel;
    } else if (cmd == "475"){
        command = Command::ErrorBadChannelKey;
    } else if (cmd == "471"){
        command = Command::ErrorChannelIsFull;
    } else if (cmd == "331"){
        command = Command::ReplyNoTopic;
    } else if (cmd == "332"){
        command = Command::ReplyTopic;
    } else if (cmd == "333"){
        command = Command::ReplyTopicAuthor;
    } else if (cmd == "353"){
        command = Command::ReplyNames;
    } else if (cmd == "366"){
        command = Command::ReplyNamesEndOf;
    } else if (cmd == "372"){
        command = Command::ReplyMOTD;
    } else if (cmd == "375"){
        command = Command::ReplyMOTDStart;
    } else if (cmd == "376"){
        command = Command::ReplyMOTDEndOf;
    }
    return command;
    
}

static std::string convertCommand(const Command::Type & cmd){
    switch (cmd){
        case Command::Pass: return "PASS";
        case Command::Nick: return "NICK";
        case Command::User: return "USER";
        case Command::Server: return "SERVER";
        case Command::Oper: return "OPER";
        case Command::Quit: return "QUIT";
        case Command::Squit: return "SQUIT";
        case Command::Join: return "JOIN";
        case Command::Part: return "PART";
        case Command::Mode: return "MODE";
        case Command::Topic: return "TOPIC";
        case Command::Names: return "NAMES";
        case Command::List: return "LIST";
        case Command::Invite: return "INVITE";
        case Command::Kick: return "KICK";
        case Command::Version: return "VERSION";
        case Command::Stats: return "STATS";
        case Command::Links: return "LINKS";
        case Command::Time: return "TIME";
        case Command::Connect: return "CONNECT";
        case Command::Trace: return "TRACE";
        case Command::Admin: return "ADMIN";
        case Command::Info: return "INFO";
        case Command::PrivateMessage: return "PRIVMSG";
        case Command::Notice: return "NOTICE";
        case Command::Who: return "WHO";
        case Command::Whois: return "WHOIS";
        case Command::Whowas: return "WHOAS";
        case Command::Kill: return "KILL";
        case Command::Ping: return "PING";
        case Command::Pong: return "PONG";
        case Command::Error: return "ERROR";
        case Command::Unknown:
        default:
            break;
    }

    return "";
}

static std::vector<std::string> split(std::string str, char splitter){
    std::vector<std::string> strings;
    size_t next = str.find(splitter);
    while (next != std::string::npos){
        strings.push_back(str.substr(0, next));
        str = str.substr(next+1);
        next = str.find(splitter);
    }
    if (str != ""){
        strings.push_back(str);
    }

    return strings;
}
    
Command::Command(const std::string & message){
    std::vector< std::string > messageSplit = split(message, ' ');
    std::vector< std::string >::iterator current = messageSplit.begin();
    if (Util::matchRegex(*current, Util::Regex("^:.*"))){
        // Found owner (":") indicates the user, otherwise it's going to be the command
        // Grab just the username, ignore everything else
        try{
            owner = split(*current, '!').at(0).substr(1);
        } catch (const std::out_of_range & ex){
        }
        current++;
    }
    // Next is the actual command
    type = convertCommand(*current);
    if (type == Unknown){
        Global::debug(0) << "Got unhandled response: " << message << std::endl;
    }
    current++;
    // Parameters
    bool foundCtcp = false;
    bool concactenate = false;
    std::string concactenated;
    for (std::vector< std::string >::iterator i = current; i != messageSplit.end(); ++i){
        const std::string & parameter = *i;
        // If there is a colon in the parameter the rest of split string is the whole parameter rejoin
        if (Util::matchRegex(parameter, Util::Regex("^:\001.*")) && !foundCtcp){
            foundCtcp = true;
            // Drop the ':\001'
            ctcp.push_back(parameter.substr(2));
            continue;
        } else if (Util::matchRegex(parameter, Util::Regex(".*\001")) && foundCtcp){
            foundCtcp = false;
            // Drop the '\001'
            ctcp.push_back(parameter.substr(0, parameter.size()-1));
            continue;
        } else if (Util::matchRegex(parameter, Util::Regex("^:.*")) && !concactenate){
            concactenate = true;
            // Drop the ':'
            concactenated += parameter.substr(1) + " ";
            continue;
        } else if (Util::matchRegex(parameter, Util::Regex("=")) ||
                   Util::matchRegex(parameter, Util::Regex("@"))){
            // Ignore
            continue;
        }
        if (concactenate){
            concactenated += parameter + " ";
        } else {
            if (!foundCtcp){
                parameters.push_back(parameter);
            } else {
                ctcp.push_back(parameter);
            }
        }
    }
    if (concactenate){
        parameters.push_back(concactenated);
    }
}

Command::Command(const std::string & owner, const Type & type):
owner(owner),
type(type){
}

Command::Command(const Command & copy):
owner(copy.owner),
type(copy.type),
parameters(copy.parameters),
ctcp(copy.ctcp){
}

Command::~Command(){
}

const Command & Command::operator=(const Command & copy){
    owner = copy.owner;
    type = copy.type;
    parameters = copy.parameters;
    ctcp = copy.ctcp;
    return *this;
}

std::string Command::getSendable() const {
    std::string sendable;
    // Name
    if (!owner.empty()){
        sendable += ":" + owner + " ";
    }
    // Command
    sendable += convertCommand(type) + " ";
    // Params
    /*for (std::vector<std::string>::const_iterator i = parameters.begin(); i != parameters.end(); ++i){
        sendable+= *i + " ";
    }*/
    for (unsigned int i = 0; i < parameters.size(); ++i){
        sendable += parameters[i] + (i < parameters.size()-1 ? " " : "");
    }
    // End
    sendable += "\r\n";
    return sendable;
}

Channel::Channel(){
}

Channel::Channel(const std::string & name):
name(name){
}

Channel::Channel(const Channel & copy):
name(copy.name),
topic(copy.topic),
topicAuthor(copy.topicAuthor),
topicDate(copy.topicDate),
users(copy.users){
}

Channel::~Channel(){
}

const Channel & Channel::operator=(const Channel & copy){
    name = copy.name;
    topic = copy.topic;
    topicAuthor = copy.topicAuthor;
    topicDate = copy.topicDate;
    users = copy.users;
    return *this;
}

void Channel::addUser(const std::string & user){
    // Can't add same user twice
    for (std::vector<std::string>::iterator i = users.begin(); i != users.end(); ++i){
        const std::string & name = *i;
        if (name == user){
            return;
        }
    }
    users.push_back(user);
}

void Channel::removeUser(const std::string & user){
    for (std::vector<std::string>::iterator i = users.begin(); i != users.end(); ++i){
        const std::string & name = *i;
        if (name == user){
            users.erase(i);
            break;
        }
    }
}

void Channel::addUsers(const std::vector<std::string> & list){
    for (std::vector<std::string>::const_iterator i = list.begin(); i != list.end(); ++i){
        const std::string & name = *i;
        addUser(name);
    }
}

Client::Client(const std::string & hostname, int port):
previousUsername("AUTH"),
username("AUTH"),
previousActiveChannel(0),
currentChannel(0),
hostname(hostname),
port(port),
end(false){
}

Client::~Client(){
}

void Client::connect(){
    if (username.empty()){
        throw NetworkException("Set username first.");
    }
    Global::debug(0) << "Connecting to " << hostname << " on port " << port << std::endl;
    socket = Network::connectReliable(hostname, port);
    start();
    setName("paintown-test");
    Command user("AUTH", Command::User);
    user.setParameters(username, "*", "0", ":auth");
    sendCommand(user);
    //  ^^^^^^^^ Should get a response from this crap!
    joinChannel("#paintown");
    
    Global::debug(0) << "Connected" << std::endl;
}



bool Client::hasCommands() const{
    ::Util::Thread::ScopedLock scope(lock);
    return !commands.empty();
}

Command Client::nextCommand() const {
    ::Util::Thread::ScopedLock scope(lock);
    Command command = commands.front();
    commands.pop();
    return command;
}

void Client::sendCommand(const Command & command){
    const std::string & sendable = command.getSendable();
    Network::sendBytes(socket, (uint8_t *) sendable.c_str(), sendable.size());
}

void Client::sendCommand(const Command::Type & type){
    Command command(username, type);
    sendCommand(command);
}

void Client::sendCommand(const Command::Type & type, const std::string & param1){
    Command command(username, type);
    command.setParameters(param1);
    sendCommand(command);
}

void Client::sendCommand(const Command::Type & type, const std::string & param1, const std::string & param2){
    Command command(username, type);
    command.setParameters(param1, param2);
    sendCommand(command);
}

void Client::sendCommand(const Command::Type & type, const std::string & param1, const std::string & param2, const std::string & param3){
    Command command(username, type);
    command.setParameters(param1, param2, param3);
    sendCommand(command);
}

void Client::sendCommand(const Command::Type & type, const std::string & param1, const std::string & param2, const std::string & param3, const std::string & param4){
    Command command(username, type);
    command.setParameters(param1, param2, param3, param4);
    sendCommand(command);
}

void Client::setName(const std::string & name){
    previousUsername = username;
    // Update channel list
    //channel.removeUser(username);
    //channel.addUser(name);
    for (std::vector< ChannelPointer >::iterator i = activeChannels.begin(); i != activeChannels.end(); ++i){
        ChannelPointer activeChannel = *i;
        activeChannel->removeUser(username);
        activeChannel->addUser(name);
    }
    username = name;
    sendCommand(Command::Nick, name);
}

void Client::joinChannel(const std::string & chan){
    for (std::vector< ChannelPointer >::iterator i = activeChannels.begin(); i != activeChannels.end(); ++i){
        ChannelPointer activeChannel = *i;
        if (activeChannel->getName() == chan){
            // Already belonging to this channel
            return;
        }
    }
    previousActiveChannel = currentChannel;
    ChannelPointer newChannel = ChannelPointer(new Channel(chan));
    /*if (!currentChannel.getName().empty()){
        sendCommand(Command::Part, currentChannel.getName());
    }*/
    activeChannels.push_back(newChannel);
    currentChannel = activeChannels.size()-1;
    sendCommand(Command::Join, getChannel()->getName());
}

std::string Client::channelListAsString(){
    std::string list;
    for (std::vector<ChannelPointer>::iterator i = activeChannels.begin(); i != activeChannels.end(); ++i){
        ChannelPointer activeChannel = *i;
        list += activeChannel->getName() + ", ";
    }
    return list.substr(0, list.size()-2);
}

unsigned int Client::getChannelIndex(const std::string & channel){
    for (unsigned int i = 0; i < activeChannels.size(); ++i){
        if (activeChannels[i]->getName() == channel){
            return i;
        }
    }
    return 0;
}

bool Client::isCurrentChannel(const std::string & channel){
    return (activeChannels[currentChannel]->getName() == channel);
}

void Client::setChannel(unsigned int channel){
    if (channel >= activeChannels.size()){
        return;
    }
    currentChannel = channel;
}

void Client::nextChannel(){
    currentChannel = (currentChannel + 1) % activeChannels.size();
}

void Client::previousChannel(){
    if (currentChannel == 0){
        currentChannel = activeChannels.size()-1;
    } else {
        currentChannel--;
    }
}

void Client::removeChannel(const std::string & name){
    for (std::vector< ChannelPointer >::iterator i = activeChannels.begin(); i != activeChannels.end(); ++i){
        ChannelPointer activeChannel = *i;
        if (activeChannel->getName() == name){
            activeChannels.erase(i);
            return;
        }
    }
}

ChannelPointer Client::findChannel(const std::string & name){
    for (std::vector< ChannelPointer >::iterator i = activeChannels.begin(); i != activeChannels.end(); ++i){
        ChannelPointer activeChannel = *i;
        if (activeChannel->getName() == name){
            return activeChannel;
        }
    }
    return ChannelPointer(NULL);
}

void Client::sendMessage(const std::string & msg){
    sendCommand(Command::PrivateMessage, getChannel()->getName(), ":" + msg);
}

void Client::sendPong(const Command & ping){
    Command pong(username, Command::Pong);
    pong.setParameters(ping.getParameters());
    sendCommand(pong);
}

std::string Client::readMessage(){
    std::string received;
    bool foundReturn = false;
    while (true){
        try {
            char nextCharacter = Network::read8(socket);
            /* NOTE the latest RFC says that either \r or \n is the end of the message
             * http://www.irchelp.org/irchelp/rfc/chapter8.html
             */
            if (nextCharacter == '\r'){
                // Found return
                foundReturn = true;
                continue;
            } else if ((nextCharacter == '\n') && foundReturn){
                // Should be the end of the message assuming \r is before it
                break;
            }
            received += nextCharacter;
        } catch (const Network::MessageEnd & ex){
            // end of message get out
            throw ex;
        }
    }
    //Global::debug(0) << "Read next string: " << received << std::endl;
    return received;
}

void Client::checkResponseAndHandle(const Command & command){
    // Checks for username or channel errors
    if (command.getType() == Command::ErrorNickInUse){
        ::Util::Thread::ScopedLock scope(lock);
        // Change the username back to what it was
        getChannel()->removeUser(username);
        username = previousUsername;
        getChannel()->addUser(username);
    } else if (command.getType() == Command::ErrorBannedFromChannel ||
               command.getType() == Command::ErrorInviteOnlyChannel ||
               command.getType() == Command::ErrorBadChannelKey ||
               command.getType() == Command::ErrorChannelIsFull ||
               command.getType() == Command::ErrorNoSuchChannel){
        ::Util::Thread::ScopedLock scope(lock);
        // Revert old channel
        removeChannel(getChannel()->getName());
        currentChannel = previousActiveChannel;
    } else if (command.getType() == Command::ReplyTopic){
        ::Util::Thread::ScopedLock scope(lock);
        // Set topic
        getChannel()->setTopic(command.getParameters().at(2));
    } else if (command.getType() == Command::ReplyTopicAuthor){
        ::Util::Thread::ScopedLock scope(lock);
        // Set topic and author
        const std::vector<std::string> & params = command.getParameters();
        getChannel()->setTopicAuthor(split(params.at(1), '!').at(0), atoi(params.at(2).c_str()));
    } else if (command.getType() == Command::ReplyNames){
        // Add names
        const std::vector<std::string> & params = command.getParameters();
        //if (params.at(1) == currentChannel->getName()){
            const std::vector<std::string> & names = split(params.at(2), ' ');
            ::Util::Thread::ScopedLock scope(lock);
            //currentChannel->addUsers(names);
            ChannelPointer update = findChannel(params.at(1));
            if (update != NULL){
                update->addUsers(names);
            }
        //}
    }
}

void Client::run(){
    while (!end){
        try {
            const std::string & message = readMessage();
            // Check if the message is empty it might be because of (\n)
            if (!message.empty()){
                Command command(message);
                ::Util::Thread::ScopedLock scope(lock);
                checkResponseAndHandle(command);
                commands.push(command);
            } else {
            }
        } catch (const Network::MessageEnd & ex){
            end = true;
        }
    }
}

ChatInterface::ChatInterface(const std::string & host, int port){
    //client = Util::ReferenceCount< Client >(new Client(host, port));
    //client->connect();
    
    // Setup window size and chat list
    int width = Configuration::getScreenWidth();
    int height = Configuration::getScreenHeight();
    
    Global::debug(0) << "Current Height: " << height * .8 << " Current Width: " << width *.09 << std::endl;
    
    // chat panel 80% width 90% height
    chatBox.location.set(0, 0, 0, 0);
    chatBox.location.setDimensions(width * .8, height * .9);
    // edit box
    inputBox.location.set(0, height * .91, 0, 0);
    inputBox.location.setDimensions(width * .8, height * .09);
    // Set the location of user list width * .8 and height
}

ChatInterface::~ChatInterface(){
}

void ChatInterface::act(){
    chatBox.act(Font::getDefaultFont());
    inputBox.act(Font::getDefaultFont());
}

void ChatInterface::draw(const Graphics::Bitmap & work){
    chatBox.render(work);
    inputBox.render(work);
}

Util::ReferenceCount<Client> ChatInterface::getClient(){
    return client;
}

}
}
