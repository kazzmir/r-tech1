#ifndef _paintown_network_h
#define _paintown_network_h

#include <stdint.h>
#ifdef HAVE_NETWORKING
#include "hawknl/nl.h"
#endif
#include <string>
#include <vector>
#include <exception>

namespace Network{

#ifdef HAVE_NETWORKING
typedef NLsocket Socket;
#else
typedef int Socket;
#endif

const int NO_CONNECTIONS_PENDING = 1;
const int NETWORK_ERROR = 2;

const int DATA_SIZE = 16;

class NetworkException: public std::exception{
public:
	NetworkException( const std::string message = "" ):std::exception(),message(message){}

	inline const std::string getMessage() const {
		return message;
	}
	
	~NetworkException() throw();

protected:
	inline void setMessage( const std::string & m ){
		this->message = m;
	}

private:
	std::string message;
};

class NoConnectionsPendingException: public NetworkException{
public:
	NoConnectionsPendingException(const std::string message = ""):
		NetworkException(message){
	}
};

class MessageEnd: public NetworkException {
public:
    MessageEnd();
};

class InvalidPortException: public NetworkException{
public:
	InvalidPortException( int port, const std::string message = "" );
};

class CannotListenException: public NetworkException{
public:
	CannotListenException( const std::string message = "" ):
		NetworkException( message ){
	}
};

/*
template <class M>
int totalSize(const std::vector<M> & messages);

template <class M>
void dump(const std::vector<M> & messages, uint8_t * buffer );
*/

#ifdef HAVE_NETWORKING
int16_t read16(Socket socket);
int32_t read32(Socket socket);
char * dump16(char * where, int16_t length);
void send16(Socket socket, int16_t length);
std::string readStr(Socket socket, const uint16_t length);
void sendStr(Socket socket, const std::string & str );
void sendBytes(Socket socket, const uint8_t * data, int length);
void readBytes(Socket socket, uint8_t * data, int length);
char * dumpStr(char * where, const std::string & str);
char * parse16(char * where, uint16_t * out);
char * parseString(char * where, std::string * out, uint16_t length);
void init();
void shutdown();
/* Whether or not blocking is enabled by default for new sockets */
void blocking(bool b);
/* Enable/disable blocking for a specific socket */
bool blocking(Socket s, bool b);
/* Enable/disable NODELAY -- the Nagle algorithm for TCP */
bool noDelay(Socket s, bool b);

void listen(Socket s) throw (NetworkException);
Socket accept(Socket s) throw (NetworkException);

Socket open(int port) throw (InvalidPortException);
Socket connect( std::string server, int port ) throw (NetworkException);
void close(Socket);
void closeAll();

static std::vector<Socket> open_sockets;
#endif

}

#endif
