#include "chat.h"

using namespace Network;
using namespace Chat;

Message::Message():
type(Unknown){
}

Message::Message(Socket socket):
type(Unknown){
    read(socket);
}

Message::Message(const Message::Type & type, const std::string & sender, const std::string & message):
type(type),
sender(sender),
message(message){
}

Message::Message(const Message & copy):
type(copy.type),
sender(copy.sender),
message(copy.message),
parameters(copy.parameters){
}

Message::~Message(){
}

const Message & Message::operator=(const Message & copy){
    type = copy.type;
    sender = copy.sender;
    message = copy.message;
    parameters = copy.parameters;
    return *this;
}

static std::string convertType(const Message::Type & type){
    std::string converted;
    switch (type){
        case Message::Ping:
            converted = "ping";
            break;
        case Message::Chat:
            converted = "chat";
            break;
        case Message::Command:
            converted = "command";
            break;
        case Message::Unknown:
        default:
            converted = "unknown";
            break;
    }
    return converted;
}

static Message::Type convertType(const std::string & type){
    Message::Type converted = Message::Unknown;
    if (type == "ping"){
        converted = Message::Ping;
    } else if (type == "chat"){
        converted = Message::Chat;
    } else if (type == "command"){
        converted = Message::Command;
    } else if (type == "unknown"){
        converted = Message::Unknown;
    } 
    return converted;
}

void Message::read(Socket socket){
    int16_t size = ::Network::read16(socket);
    char * buffer = new char[size];
    ::Network::readBytes(socket, (uint8_t*) buffer, size);
    char * position = buffer;
    uint16_t nextSize = 0;
    std::string typeString;
    position = ::Network::parse16(position, &nextSize);
    position = ::Network::parseString(position, &typeString, nextSize + 1);
    nextSize = 0;
    position = ::Network::parse16(position, &nextSize);
    position = ::Network::parseString(position, &sender, nextSize + 1);
    nextSize = 0;
    position = ::Network::parse16(position, &nextSize);
    position = ::Network::parseString(position, &message, nextSize + 1);
    uint16_t vectorSize = 0;
    position = ::Network::parse16(position, &vectorSize);
    for (int i = 0; i < vectorSize; ++i){
        std::string next;
        nextSize = 0;
        position = ::Network::parse16(position, &nextSize);
        position = ::Network::parseString(position, &next, nextSize + 1);
        parameters.push_back(next);
    }
    delete[] buffer;
}
    
void Message::send(Socket socket){
    // Type
    std::string stringType = convertType(type);
    int size = sizeof(uint16_t) + stringType.size()+1 + 
               sizeof(uint16_t) + sender.size()+1 +
               sizeof(uint16_t) + message.size()+1 +
               sizeof(uint16_t) + parameters.size();
    for (std::vector<std::string>::iterator i = parameters.begin(); i != parameters.end(); ++i){
        size+=sizeof(uint16_t) + (*i).size()+1;
    }
    size+=sizeof(uint16_t);
    char * buffer = new char[size + sizeof(uint16_t)];
    char * position = buffer;
    position = ::Network::dump16(position, size);
    position = ::Network::dump16(position, stringType.size());
    position = ::Network::dumpStr(position, stringType);
    position = ::Network::dump16(position, sender.size());
    position = ::Network::dumpStr(position, sender);
    position = ::Network::dump16(position, message.size());
    position = ::Network::dumpStr(position, message);
    position = ::Network::dump16(position, parameters.size());
    for (std::vector<std::string>::iterator i = parameters.begin(); i != parameters.end(); ++i){
        position = ::Network::dump16(position, (*i).size());
        position = ::Network::dumpStr(position, (*i));
    }
    ::Network::sendBytes(socket, (uint8_t*) buffer, size + sizeof(uint16_t));
    delete[] buffer;
}

const Message::Type & Message::getType() const{
    return type;
}

const std::string & Message::getName() const{
    return sender;
}

const std::string & Message::getMessage() const{
    return message;
}

void Message::setParameters(const std::vector<std::string> & parameters){
    this->parameters = parameters;
}

const std::vector<std::string> & Message::getParameters() const{
    return parameters;
}
