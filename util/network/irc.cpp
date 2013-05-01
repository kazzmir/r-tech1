#include "irc.h"

#include "util/funcs.h"
#include "util/font.h"
#include "util/regex.h"
#include "util/system.h"
#include "util/graphics/bitmap.h"
#include "util/configuration.h"
#include "util/gui/context-box.h"

#include "util/timedifference.h"

#include <stdexcept>
#include <queue>

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
end(false),
disableMessages(true){
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
    if (!disableMessages){
        sendCommand(Command::PrivateMessage, getChannel()->getName(), ":" + msg);
    }
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

Message::EventInterface::EventInterface(){
}

Message::EventInterface::~EventInterface(){
}

void Message::EventInterface::EventInterface::localNotify(const std::string &){
}

void Message::EventInterface::EventInterface::localCommand(const std::vector<std::string> &){
}

void Message::EventInterface::EventInterface::remoteNotify(const std::string &){
}

void Message::EventInterface::EventInterface::remoteCommand(const Command &){
}


Message::HandlerInterface::HandlerInterface(){
}

Message::HandlerInterface::~HandlerInterface(){
}

Message::QueueInterface::QueueInterface(){
}

Message::QueueInterface::~QueueInterface(){
}

Message::QueueInterface::FontWrapper::FontWrapper(){
}

Message::QueueInterface::FontWrapper::~FontWrapper(){
}

void Message::QueueInterface::processMessages(Message::QueueInterface::FontWrapper & wrapper, int width, int height){
    while (!messages.empty()){
        const std::string message = messages.front();
        messages.pop();
        // Check message if it exceeds the length of the box so we can split it
        if (wrapper.getWidth(message) > width-15){
            unsigned int marker = 0;
            unsigned int length = 0;
            while ((marker+length) < message.size()){
                //Global::debug(0) << "Substring: " << message.substr(marker, length) << " Marker: " << marker << " and Current length: " << length << std::endl;
                if (wrapper.getWidth(message.substr(marker, length)) < width-15){
                    length++;
                    continue;
                } else {
                    if (message[marker+length] == ' '){
                        buffer.push_front(message.substr(marker, length));
                        marker += length+1;
                        length = 0;
                    } else {
                        // Search for previous space
                        unsigned int cutoff = marker+length;
                        while ((marker+length) > marker){
                            if (message[marker+length] == ' '){
                                break;
                            }
                            length--;
                        }
                        if ((marker+length) > marker){
                            buffer.push_front(message.substr(marker, length));
                            marker += length+1;
                            length = 0;
                        } else {
                            buffer.push_front(message.substr(marker, cutoff));
                            marker = cutoff+1;
                            length = 0;
                        }
                    }
                }
            }
            // Add last item
            if ((marker+length) > marker){
                buffer.push_front(message.substr(marker, length));
            }
        } else {
            buffer.push_front(message);
        }
        
        // Drop out of sight
        if ((buffer.size() * (wrapper.getHeight()+2)) > (unsigned int)height){
            buffer.pop_back();
        }
    }
}

class ChannelTab: public Gui::TabItem, Message::QueueInterface{
public:
    ChannelTab(const std::string & name):
    TabItem(name),
    width(0),
    height(0),
    changed(false){
    }
    virtual ~ChannelTab(){
    }
    void act(const Font & font){
        class Wrapper: public Message::QueueInterface::FontWrapper{
        public:
            Wrapper(const Font & font):
            font(font){
            }
            virtual ~Wrapper(){
            }
            const Font & font;
            int getWidth(const std::string & text){
                return font.textLength(text.c_str());
            }
            int getHeight(){
                return font.getHeight();
            }
                
        };
        Wrapper wrapper(font);
        processMessages(wrapper, width, height);
    }
    void draw(const Font& font, const Graphics::Bitmap & work){
        if (!isActive()){
            return;
        }
        work.clear();
        int mod = height-(font.getHeight()+2);
        for (std::deque<std::string>::iterator i = buffer.begin(); i != buffer.end(); ++i){
            const std::string & text = *i;
            font.printf(0, mod, Graphics::makeColor(255,255,255), work, text, 0);
            mod-=(font.getHeight()+2);
        }
    }
    
    void addMessage(const std::string & message){
        if (!isActive() && !changed){
            changed = true;
        }
        messages.push(message);
    }
    
    void addMessage(const std::string & name, const std::string & message){
        addMessage("<"+name+"> " + message);
    }

    void inspectBody(const Graphics::Bitmap & body){
        width = body.getWidth();
        height = body.getHeight();
    }
    void toggleActive(){
        if (!active){
            active = true;
            changed = false;
        } else {
            active = false;
        }
    }
private:
    int width;
    int height;
    bool changed;
};

static Util::ReferenceCount<ChannelTab> convertTab(ChatInterface * chat, std::string name = ""){
    if (!name.empty()){
        try{
            return chat->getTabByName(name).convert<ChannelTab>();
        } catch (const Gui::TabContainer::NoSuchTab & ex){
        }
    }
    return chat->getCurrentTab().convert<ChannelTab>();
}

void ChatInterface::submit(void * interface){
    ChatInterface * chat = (ChatInterface *)interface;
    if (!chat->getInputBox().getText().empty()){
        const std::string & text = chat->getInputBox().getText();
        // check if it's a command
        if (Util::matchRegex(text, Util::Regex("^/.*"))){
            const std::vector<std::string> & command = Util::splitString(text.substr(1), ' ');
            chat->localCommand(command);
        } else {
            convertTab(chat)->addMessage(chat->getClient()->getName(), text);
            chat->getClient()->sendMessage(text);
            chat->localNotify(text);
        }
        chat->getInputBox().clear();
    }
}

ChatInterface::ChatInterface(const std::string & host, int port):
host(host),
widthRatio(.8),
heightRatio(.95),
width(0),
height(0),
checkWidth(Configuration::getScreenWidth()),
checkHeight(Configuration::getScreenHeight()),
serverTab(Util::ReferenceCount<Gui::TabItem>(new ChannelTab(host))){
    client = Util::ReferenceCount< Client >(new Client(host, port));
    client->connect();
    
    // Setup window size and chat list
    updateDimensions();
    
    // Add server tab
    chatBox.add(serverTab);
}

ChatInterface::~ChatInterface(){
}

void ChatInterface::act(){
    updateDimensions();
    // Default size of fonts
    const int size = height * (1 - (heightRatio + .01));
    // Size is important
    const Font & font = Font::getDefaultFont(size, size);
    
    processRemoteCommands();
    chatBox.act(font);
    inputBox.act(font);
}

void ChatInterface::draw(const Graphics::Bitmap & work){
    checkWidth = work.getWidth();
    checkHeight = work.getHeight();
    const int size = height * (1 - (heightRatio + .01));
    const Font & font = Font::getDefaultFont(size, size);
    
    chatBox.draw(font, work);
    inputBox.draw(font, work);
}

void ChatInterface::nextChannel(){
    chatBox.next();
    const std::string & name = chatBox.getCurrent()->getName();
    if (name == host){
        client->setMessagesDisabled(true);
    } else {
        client->setMessagesDisabled(false);
        client->setChannel(client->getChannelIndex(name));
    }
}

void ChatInterface::previousChannel(){
    chatBox.previous();
    const std::string & name = chatBox.getCurrent()->getName();
    if (chatBox.getCurrent()->getName() == host){
        client->setMessagesDisabled(true);
    } else {
        client->setMessagesDisabled(false);
        client->setChannel(client->getChannelIndex(name));
    }
}

void ChatInterface::gotoChannel(const std::string & channel){
    chatBox.gotoTabByName(channel);
    const std::string & name = chatBox.getCurrent()->getName();
    if (chatBox.getCurrent()->getName() == host){
        client->setMessagesDisabled(true);
    } else {
        client->setMessagesDisabled(false);
        client->setChannel(client->getChannelIndex(name));
    }
}

Util::ReferenceCount<Client> ChatInterface::getClient(){
    return client;
}

Util::ReferenceCount<Gui::TabItem> ChatInterface::getCurrentTab(){
    return chatBox.getCurrent();
}

Util::ReferenceCount<Gui::TabItem> ChatInterface::getTabByName(const std::string & name){
    return chatBox.getByName(name);
}

void ChatInterface::addMessageToTab(const std::string & message){
    convertTab(this)->addMessage(message);
}

void ChatInterface::addMessageToTab(const std::string & name, const std::string & message){
    convertTab(this)->addMessage(name, message);
}

void ChatInterface::updateDimensions(){
    if (width == checkWidth && height == checkHeight){
        return;
    }
    width = checkWidth;
    height = checkHeight;
    
    chatBox.transforms.setRadius(15);
    Gui::ColorInfo tabbed;
    tabbed.body = Graphics::makeColor(255,255,255);
    tabbed.bodyAlpha = 128;
    tabbed.border = Graphics::makeColor(0,0,255);
    tabbed.borderAlpha = 255;
    chatBox.colors = tabbed;
    
    // chat panel widthRatio% heightRatio%
    chatBox.location.setPosition(Gui::AbsolutePoint(0, 0));
    chatBox.location.setDimensions(width * widthRatio, height * heightRatio);
    
    // edit box widthRatio% remaining (heightRatio + .01)%
    const double inputStart = heightRatio + .01;
    inputBox.transforms.setRadius(15);
    inputBox.location.setPosition(Gui::AbsolutePoint(0, height * inputStart));
    inputBox.location.setDimensions(width * widthRatio, height * (1 - inputStart));
    inputBox.addHook(Keyboard::Key_ENTER, submit, this);
    // Set the location of user list width * widthRatio and height
}

void ChatInterface::remoteNotify(const std::string & message){
    for (std::vector<NotifyCallback>::iterator i = callbacks.begin(); i != callbacks.end(); ++i){
        (*i)(message);
    }
    
    for (std::vector<Message::EventInterface *>::iterator i = subscribers.begin(); i != subscribers.end(); ++i){
        (*i)->localNotify(message);
    }
}

void ChatInterface::processRemoteCommands(){
    while (client->hasCommands()){
        ::Network::IRC::Command command = client->nextCommand();
        const std::vector<std::string> & params = command.getParameters();
        //Global::debug(0) << "Got message: " << command.getSendable() << std::endl;
        try {
            if (command.getType() == ::Network::IRC::Command::Ping){
                client->sendPong(command);
                serverTab.convert<ChannelTab>()->addMessage(command.getOwner(), "*** Ping!");
            } else if (command.getType() == ::Network::IRC::Command::PrivateMessage){
                // Check channel
                const std::string & channel = params.at(0);
                if (channel == client->getName()){
                    // Private message to user (not channel)
                    convertTab(this, channel)->addMessage("\""+command.getOwner() + "\" whispered: " + params.at(1));
                } else if (client->isCurrentChannel(channel)){
                    // Username and message 
                    convertTab(this, channel)->addMessage(command.getOwner(), params.at(1));
                }
                // notify
                remoteNotify(params.at(1));
            } else if (command.getType() == ::Network::IRC::Command::Notice){
                // Username and message 
                serverTab.convert<ChannelTab>()->addMessage(command.getOwner(), params.at(1));
                // notify
                remoteNotify(params.at(1));
            } else if (command.getType() == ::Network::IRC::Command::ReplyMOTD ||
                       command.getType() == ::Network::IRC::Command::ReplyMOTDStart ||
                       command.getType() == ::Network::IRC::Command::ReplyMOTDEndOf){
                serverTab.convert<ChannelTab>()->addMessage("*** MOTD " + params.at(1));
            } else if (command.getType() == ::Network::IRC::Command::Join){
                serverTab.convert<ChannelTab>()->addMessage("*** You have joined the channel " + params.at(0) + ".");
                chatBox.add(Util::ReferenceCount<Gui::TabItem>(new ChannelTab(params.at(0))));
                gotoChannel(params.at(0));
            } else if (command.getType() == ::Network::IRC::Command::ReplyNoTopic){
                serverTab.convert<ChannelTab>()->addMessage("*** The channel " + params.at(1) + " has no topic set.");
            } else if (command.getType() == ::Network::IRC::Command::ReplyTopic){
                serverTab.convert<ChannelTab>()->addMessage("*** The channel topic for " + params.at(1) + " is: \"" + params.at(2) + "\".");
            } else if (command.getType() == ::Network::IRC::Command::ReplyNames){
                std::vector<std::string> names = split(params.at(2), ' ');
                std::map<std::string, std::vector<std::string> >::iterator check = namesRequest.find(params.at(1));
                if (check == namesRequest.end()){
                    namesRequest[params.at(1)] = std::vector<std::string>();
                    check = namesRequest.find(params.at(1));
                }
                check->second.insert(check->second.begin(), names.begin(), names.end());
            } else if (command.getType() == ::Network::IRC::Command::ReplyNamesEndOf){
                std::map<std::string, std::vector<std::string> >::iterator check = namesRequest.find(params.at(1));
                if (check != namesRequest.end()){
                    serverTab.convert<ChannelTab>()->addMessage("*** Current users on " + params.at(1) + " \"" + Util::joinStrings(check->second) + "\".");
                    namesRequest.erase(check);
                }
            } else if (command.getType() == ::Network::IRC::Command::ErrorNickInUse){
                serverTab.convert<ChannelTab>()->addMessage("[Error] " + params.at(1) + ": Nick already in use.");
            } else if (command.getType() == ::Network::IRC::Command::ErrorNoSuchNick){
                serverTab.convert<ChannelTab>()->addMessage("[Error] " + params.at(1) + ": No such nick.");
            } else if (command.getType() == ::Network::IRC::Command::ErrorNoSuchChannel){
                serverTab.convert<ChannelTab>()->addMessage("[Error] " + params.at(1) + ": No such channel.");
            } else if (command.getType() == ::Network::IRC::Command::Error){
                Global::debug(0) << "Received Error: " << command.getSendable() << "... Aborting." << std::endl;
                throw Exception::Return(__FILE__, __LINE__);
            }
        } catch (const std::out_of_range & ex){
        }
        // Check if we got any CTCP delimited messages
        if (command.hasCtcp()){
            const std::vector<std::string> & ctcp = command.getCtcp();
            try {
                if (ctcp.at(0) == "PING"){
                    // Lets check if there is an existing query otherwise send off request
                    if (command.getType() == ::Network::IRC::Command::Notice){
                        std::map<std::string, uint64_t>::iterator check = pingReply.find(command.getOwner());
                        if (check != pingReply.end()){
                            // there is an existing entry lets display our ping
                            std::ostringstream difference;
                            difference << (double)((System::currentMilliseconds() - check->second)/1000000);
                            serverTab.convert<ChannelTab>()->addMessage("[CTCP] Received CTCP-PING reply from " + check->first + ": " + difference.str() + "second(s)" );
                            pingReply.erase(check);
                        }
                    } else if (command.getType() == ::Network::IRC::Command::PrivateMessage){
                        client->sendCommand(::Network::IRC::Command::Notice, command.getOwner(), ":\001PING " + ctcp.at(1) + "\001");
                        serverTab.convert<ChannelTab>()->addMessage("[CTCP] Received CTCP-PING request from " + command.getOwner() + ", sending answer.");
                    }
                }
            } catch (const std::out_of_range & ex){
            }
        }
        // Pass out the command to listeners
        for (std::vector<Message::EventInterface *>::iterator i = subscribers.begin(); i != subscribers.end(); ++i){
            (*i)->remoteCommand(command);
        }    
    }
}

void ChatInterface::localNotify(const std::string & message){
    for (std::vector<NotifyCallback>::iterator i = callbacks.begin(); i != callbacks.end(); ++i){
        (*i)(message);
    }
    
    for (std::vector<Message::EventInterface *>::iterator i = subscribers.begin(); i != subscribers.end(); ++i){
        (*i)->localNotify(message);
    }
}

void ChatInterface::localCommand(const std::vector<std::string> & command){
    if (command.at(0) == "help"){
        convertTab(this)->addMessage("* commands: help, nick, whisper, ping, join, names, topic, previous, next, channels, quit");
    } else if (command.at(0) == "nick"){
        try {
            const std::string & nick = command.at(1);
            if (!nick.empty()){
                client->setName(nick);
                convertTab(this)->addMessage("* nick changed to " + nick);
            } 
        } catch (const std::out_of_range & ex){
            convertTab(this)->addMessage("* /nick [name]");
        }
    } else if (command.at(0) == "whisper"){
        try {
            const std::string & who = command.at(1);
            const std::string & message = Util::joinStrings(command, 2);
            if (!who.empty() && !message.empty()){
                client->sendCommand(::Network::IRC::Command::PrivateMessage, who, ":" + message);
                convertTab(this)->addMessage("-> " + who + " " + message); 
            }
        } catch (const std::out_of_range & ex){
            convertTab(this)->addMessage("* /whisper [nick] [message]");
        }
    } else if (command.at(0) == "ping"){
        try {
            const std::string & who = command.at(1);
            if (!who.empty()){
                std::ostringstream timestamp;
                uint64_t time = System::currentMilliseconds();
                timestamp << time;
                client->sendCommand(::Network::IRC::Command::PrivateMessage, who, ":\001PING " + timestamp.str() + "\001");
                convertTab(this)->addMessage("[CTCP] Sending CTCP-PING request to " + who);
                // Log user
                pingReply[who] = time;
            }
        } catch (const std::out_of_range & ex){
            convertTab(this)->addMessage("* /ping [nick]");
        }
    } else if (command.at(0) == "join"){
        try {
            const std::string & channel = command.at(1);
            if (!channel.empty()){
                client->joinChannel(channel);
            }
        } catch (const std::out_of_range & ex){
            convertTab(this)->addMessage("* /join [channel]");
        }
    } else if (command.at(0) == "names"){
        try {
            const std::string & channel = command.at(1);
            if (!channel.empty()){
                client->sendCommand(::Network::IRC::Command::Names, channel);
                namesRequest[channel] = std::vector<std::string>();
            }
        } catch (const std::out_of_range & ex){
            convertTab(this)->addMessage("* /names [channel]");
        }
    } else if (command.at(0) == "topic"){
        try {
            const std::string & channel = command.at(1);
            if (!channel.empty()){
                client->sendCommand(::Network::IRC::Command::Topic, channel);
            }
        } catch (const std::out_of_range & ex){
            client->sendCommand(::Network::IRC::Command::Topic, client->getChannel()->getName());
        }
    } else if (command.at(0) == "previous"){
        client->previousChannel();
    } else if (command.at(0) == "next"){
        client->nextChannel();
    } else if (command.at(0) == "channels"){
        convertTab(this)->addMessage("* Current Channels: " + client->channelListAsString());
    } else if (command.at(0) == "quit"){
        const std::string & message = Util::joinStrings(command, 1);
        if (!message.empty()){
            client->sendCommand(::Network::IRC::Command::Quit, ":" + message);
            Global::debug(0) << "Quit (" + message + "). Waiting for server to close connection..." << std::endl;
        } else {
            client->sendCommand(::Network::IRC::Command::Quit);
            Global::debug(0) << "Quit. Waiting for server to close connection..." << std::endl;
        }
    } else {
        //convertTab(this)->addMessage("* Uknown command.");
    }
    // pass local commands out to listeners
    for (std::vector<Message::EventInterface *>::iterator i = subscribers.begin(); i != subscribers.end(); ++i){
        (*i)->localCommand(command);
    }    
}

}
}
