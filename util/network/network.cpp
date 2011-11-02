#ifdef HAVE_NETWORKING
#include "hawknl/nl.h"
#endif
#include "network.h"
#include "globals.h"
#include "util/debug.h"
#include <string>
#include <sstream>
#include <string.h>
#include "util/system.h"
#include "util/compress.h"
#include "util/thread.h"

#ifdef HAVE_NETWORKING
#ifdef WII
#include <network.h>
#elif defined(WINDOWS)
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif
#else
#define htonl(x) x
#define htons(x) x
#define ntohl(x) x
#define ntohs(x) x
#endif

using namespace std;

/* TODO: Wrap open_sockets with a mutex */
namespace Network{

NetworkException::~NetworkException() throw (){
}
    
MessageEnd::MessageEnd(){
}
	
InvalidPortException::InvalidPortException( int port, const string message ):
NetworkException(""){
	ostringstream num;
	num << port;
	num << ". ";
	num << message;
	this->setMessage( "Invalid port " + num.str() );
}

Message::Message():
id((uint32_t) -1),
timestamp(0),
readFrom(0){
	memset(data, 0, sizeof(data));
	position = data;
}
	
Message::Message(const Message & m):
timestamp(m.timestamp),
readFrom(m.readFrom){
    memcpy( data, m.data, sizeof(data) );
    position = data;
    position += m.position - m.data;
    path = m.path;
    id = m.id;
}
	
Message & Message::operator=( const Message & m ){
    memcpy(data, m.data, sizeof(data));
    position = data;
    position += m.position - m.data;
    path = m.path;
    id = m.id;
    timestamp = m.timestamp;
    readFrom = m.readFrom;
    return *this;
}

Message::Message(Socket socket){
#ifdef HAVE_NETWORKING
    position = data;
    id = read32(socket);
    readBytes(socket, data, DATA_SIZE);
    int str = read16(socket);
    if (str != -1){
        /* cap strings at 1024 bytes */
        char buf[1024];
        str = (signed)(sizeof(buf) - 1) < str ? (signed)(sizeof(buf) - 1) : str;
        readBytes(socket, (uint8_t *) buf, str);
        buf[str] = 0;
        /* this is a string copy, not an assignment to a temporary pointer */
        this->path = buf;
    }
    timestamp = System::currentMicroseconds();
    readFrom = socket;
#endif
}

uint8_t * Message::dump(uint8_t * buffer) const {
    *(uint32_t *) buffer = htonl(id);
    // Global::debug(1, "network") << "Dumped id " << id << " as " << htonl(id) << std::endl;
    buffer += sizeof(id);
    memcpy(buffer, data, DATA_SIZE);
    buffer += DATA_SIZE;
    if (path != ""){
        *(uint16_t *) buffer = htons(path.length() + 1);
        buffer += sizeof(uint16_t);
        memcpy(buffer, path.c_str(), path.length() + 1);
        buffer += path.length() + 1;
    } else {
        *(uint16_t *) buffer = htons((uint16_t) -1);
        buffer += sizeof(uint16_t);
    }
    return buffer;
}

/*
template <typename M>
int messageSize(const M& message);
*/

/*
template <>
int messageSize<Message>(Message const & message){
    return message.size();
}

template <>
int messageSize<Message*>(Message* const & message){
    return message->size();
}
*/
static int messageSize(Message const & message){
    return message.size();
}

static int messageSize(Message* const & message){
    return message->size();
}

/*
template <class M>
uint8_t * messageDump(const M& message, uint8_t * buffer);

template <>
uint8_t * messageDump<Message>(const Message & message, uint8_t * buffer){
    return message.dump(buffer);
}

template <>
uint8_t * messageDump<Message*>(Message* const & message, uint8_t * buffer){
    return message->dump(buffer);
}
*/
static uint8_t * messageDump(const Message & message, uint8_t * buffer){
    return message.dump(buffer);
}

static uint8_t * messageDump(Message* const & message, uint8_t * buffer){
    return message->dump(buffer);
}

template <typename M>
static int totalSize(const vector<M> & messages){
    int size = 0;
    for (typename vector<M>::const_iterator it = messages.begin(); it != messages.end(); it++){
        // size += messageSize<M>(*it);
        size += messageSize(*it);
    }
    return size;
}

template <class M>
static void dump(const std::vector<M> & messages, uint8_t * buffer ){
    for (typename vector<M>::const_iterator it = messages.begin(); it != messages.end(); it++ ){
        buffer = messageDump(*it, buffer);
    }
}

#ifdef HAVE_NETWORKING
template <class M>
static void doSendAllMessages(const vector<M> & messages, Socket socket){
    int length = totalSize<M>(messages);
    uint8_t * data = new uint8_t[length];
    dump<M>(messages, data);
    // Compress::testCompression((unsigned char *) data, length);
    sendBytes(socket, data, length);
    delete[] data;
}

void sendAllMessages(const vector<Message> & messages, Socket socket){
    doSendAllMessages<Message>(messages, socket);
}

void sendAllMessages(const vector<Message*> & messages, Socket socket){
    doSendAllMessages<Message*>(messages, socket);
}
#endif

void Message::send(Socket socket) const {
	/*
	send16( socket, id );
	sendBytes( socket, data, DATA_SIZE );
	if ( path != "" ){
		send16( socket, path.length() + 1 );
		sendStr( socket, path );
	} else {
		send16( socket, -1 );
	}
	*/
#ifdef HAVE_NETWORKING
    uint8_t * buffer = new uint8_t[size()];
    dump(buffer);
    sendBytes(socket, buffer, size());
    delete[] buffer;
#endif
}
	
void Message::reset(){
    position = data;
}

Message & Message::operator<<(int x){
    if (position > data + DATA_SIZE - sizeof(uint16_t)){
        throw NetworkException("Tried to set too much data");
    }

    *(int16_t *) position = htons(x);
    position += sizeof(int16_t);

    return *this;
}

Message & Message::operator<<(unsigned int x){
    if (position > data + DATA_SIZE - sizeof(uint32_t)){
        throw NetworkException("Tried to set too much data");
    }

    *(int32_t *) position = htonl(x);
    position += sizeof(int32_t);
    return *this;
}
	
Message & Message::operator>>(int & x){
    if (position > data + DATA_SIZE - sizeof(uint16_t)){
        throw NetworkException("Tried to read too much data");
    }

    x = ntohs(*(int16_t *) position);
    position += sizeof(int16_t);
    return *this;
}

Message & Message::operator>>(unsigned int & x){
    if (position > data + DATA_SIZE - sizeof(uint32_t)){
        throw NetworkException("Tried to read too much data");
    }

    x = htonl(*(int32_t *) position);
    position += sizeof(int32_t);
    return *this;
}
        
Message & Message::operator>>(std::string & out){
    out = path;
    return *this;
}

Message & Message::operator<<( string p ){
    path = p;
    return *this;
}
	
int Message::size() const {
    return sizeof(id) + DATA_SIZE + 
           (path != "" ? sizeof(uint16_t) + path.length() + 1 : sizeof(uint16_t));
}

#ifdef HAVE_NETWORKING

static string getHawkError(){
	return string(" HawkNL error: '") +
	       string(nlGetErrorStr(nlGetError())) +
	       string("' HawkNL system error: '") +
	       string(nlGetSystemErrorStr(nlGetSystemError()));
}

template<typename X>
static X readX(Socket socket){
    X data;
    readBytes(socket, (uint8_t*) &data, sizeof(X)); 
    return data;
}

int16_t read16(Socket socket){
    return ntohs(readX<uint16_t>(socket));
}

int32_t read32(Socket socket){
    return ntohl(readX<uint32_t>(socket));
}

void send16(Socket socket, int16_t bytes){
    bytes = htons(bytes);
    sendBytes(socket, (uint8_t *) &bytes, sizeof(bytes));
}

string readStr(Socket socket, const uint16_t length){
    char buffer[length + 1];
    NLint bytes = nlRead(socket, buffer, length);
    if (bytes == NL_INVALID){
        throw NetworkException(string("Could not read string.") + getHawkError());
    }
    buffer[length] = 0;
    bytes += 1;
    return string(buffer);
}

void sendStr(Socket socket, const string & str){
    if (nlWrite(socket, str.c_str(), str.length() + 1) != (signed)(str.length() + 1)){
        throw NetworkException( string("Could not write string.") + getHawkError() );
    }
}

void sendBytes(Socket socket, const uint8_t * data, int length){
    const uint8_t * position = data;
    int written = 0;
    while ( written < length ){
        int bytes = nlWrite(socket, position, length - written);
        if (bytes == NL_INVALID){
            throw NetworkException(string("Could not send bytes.") + getHawkError());
        }
        written += bytes;
        position += bytes;
    }
}

void readBytes(Socket socket, uint8_t * data, int length){
    uint8_t * position = data;
    int read = 0;
    while (read < length){
        int bytes = nlRead(socket, position, length - read);
        if (bytes == NL_INVALID){
            switch (nlGetError()){
                case NL_MESSAGE_END : throw MessageEnd();
                default : throw NetworkException(string("Could not read bytes.") + getHawkError());
            }
        }
        read += bytes;
        position += bytes;
    }
}

Util::Thread::Lock socketsLock;

Socket open(int port) throw (InvalidPortException){
    // NLsocket server = nlOpen( port, NL_RELIABLE_PACKETS );
    Global::debug(1, "network") << "Attemping to open port " << port << endl;
    Socket server = nlOpen( port, NL_RELIABLE );
    /* server will either be NL_INVALID (-1) or some low integer. hawknl
     * sockets are mapped internally to real sockets, so don't be surprised
     * if you get a socket back like 0.
     */
    if (server == NL_INVALID){
        throw InvalidPortException(port, nlGetSystemErrorStr(nlGetSystemError()));
    }
    Global::debug(1, "network") << "Successfully opened a socket: " << server << endl;
    Util::Thread::acquireLock(&socketsLock);
    open_sockets.push_back(server);
    Util::Thread::releaseLock(&socketsLock);
    return server;
}

Socket connect(string server, int port) throw (NetworkException){
    NLaddress address;
    nlGetAddrFromName( server.c_str(), &address);
    nlSetAddrPort(&address, port);
    Socket socket = open( 0 );
    if (nlConnect(socket, &address) == NL_FALSE){
        close(socket);
        throw NetworkException( "Could not connect" );
    }
    return socket;
}

void close(Socket s){
    Util::Thread::acquireLock(&socketsLock);
    for (vector< Socket >::iterator it = open_sockets.begin(); it != open_sockets.end(); ){
        if ( *it == s ){
            Global::debug(1, "network") << "Closing socket " << s << endl;
            nlClose(*it);
            Global::debug(1, "network") << "Closed" << endl;
            it = open_sockets.erase(it);
        } else {
            it++;
        }
    }
    Util::Thread::releaseLock(&socketsLock);
}

void closeAll(){
    Global::debug(1, "network") << "Closing all sockets" << std::endl;
    Util::Thread::acquireLock(&socketsLock);
    for (vector<Socket>::iterator it = open_sockets.begin(); it != open_sockets.end(); it++ ){
        nlClose(*it);
    }
    open_sockets.clear();
    Util::Thread::releaseLock(&socketsLock);
}

void init(){
    nlInit();
    nlSelectNetwork(NL_IP);
    nlEnable(NL_BLOCKING_IO);
    Util::Thread::initializeLock(&socketsLock);
    // nlDisable( NL_BLOCKING_IO );
}

void blocking(bool b){
    if (b){
        nlEnable(NL_BLOCKING_IO);
    } else {
        nlDisable(NL_BLOCKING_IO);
    }
}

void listen( Socket s ) throw( NetworkException ){
	if ( nlListen( s ) == NL_FALSE ){
		throw CannotListenException( string(nlGetSystemErrorStr( nlGetSystemError() )) );
	}
}

Socket accept( Socket s ) throw( NetworkException ){
    Socket connection = nlAcceptConnection( s );
    if ( connection == NL_INVALID ){
        /*
           if ( nlGetError() == NL_NO_PENDING ){
           error = NO_CONNECTIONS_PENDING;
           } else {
           error = NETWORK_ERROR;
           }
           return s;
           */
        if ( nlGetError() == NL_NO_PENDING ){
            throw NoConnectionsPendingException();
        }
        throw NetworkException("Could not accept connection");
    }
    Util::Thread::acquireLock(&socketsLock);
    open_sockets.push_back(connection);
    Util::Thread::releaseLock(&socketsLock);
    return connection;
}

void shutdown(){
    nlShutdown();
}

#endif

}
