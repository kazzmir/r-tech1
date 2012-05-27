#include "debug.h"
#include <iostream>
#include <string>
#include <stdio.h>

#ifdef ANDROID
#include <android/log.h>
#define ANDROID_LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "paintown", __VA_ARGS__)
#endif

#ifdef NETWORK_DEBUG
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#endif

using namespace std;

static int global_debug_level = 0;

namespace Global{
#ifdef ANDROID
android_ostream::android_ostream(bool enabled):
enabled(enabled){
}

android_ostream & operator<<(android_ostream & stream, const std::string & input){
    stream.buffer << input;
    return stream;
}

android_ostream & operator<<(android_ostream & stream, const char * input){
    stream.buffer << input;
    return stream;
}

android_ostream & operator<<(android_ostream & stream, const char input){
    stream.buffer << input;
    return stream;
}

android_ostream & operator<<(android_ostream & stream, const double input){
    stream.buffer << input;
    return stream;
}

android_ostream & operator<<(android_ostream & stream, const int input){
    stream.buffer << input;
    return stream;
}

android_ostream & operator<<(android_ostream & stream, const short int input){
    stream.buffer << input;
    return stream;
}

android_ostream & operator<<(android_ostream & stream, const short unsigned int input){
    stream.buffer << input;
    return stream;
}

android_ostream & operator<<(android_ostream & stream, const unsigned int input){
    stream.buffer << input;
    return stream;
}

android_ostream & operator<<(android_ostream & stream, const bool input){
    stream.buffer << input;
    return stream;
}

android_ostream & operator<<(android_ostream & stream, const long int input){
    stream.buffer << input;
    return stream;
}

android_ostream & operator<<(android_ostream & stream, const unsigned long int input){
    stream.buffer << input;
    return stream;
}

android_ostream & operator<<(android_ostream & stream, const void * input){
    stream.buffer << input;
    return stream;
}

android_ostream & operator<<(android_ostream & stream, std::ostream & (*f)(std::ostream &)){
    if (stream.enabled){
        ANDROID_LOGV("%s\n", stream.buffer.str().c_str());
    }
    stream.buffer.str("");
    stream.buffer.rdbuf()->pubseekoff(0, ios_base::end, ios_base::out);
    stream.buffer.clear();
    return stream;
}

android_ostream android_ostream::stream;
static android_ostream nullcout(false);
#elif defined(WII) && defined(DEBUG)
wii_ostream::wii_ostream(bool enabled):
enabled(enabled){
}

wii_ostream & operator<<(wii_ostream & stream, const std::string & input){
    stream.buffer << input;
    return stream;
}

wii_ostream & operator<<(wii_ostream & stream, const char * input){
    stream.buffer << input;
    return stream;
}

wii_ostream & operator<<(wii_ostream & stream, const char input){
    stream.buffer << input;
    return stream;
}

wii_ostream & operator<<(wii_ostream & stream, const double input){
    stream.buffer << input;
    return stream;
}

wii_ostream & operator<<(wii_ostream & stream, const int input){
    stream.buffer << input;
    return stream;
}

wii_ostream & operator<<(wii_ostream & stream, uint64_t input){
    stream.buffer << input;
    return stream;
}

wii_ostream & operator<<(wii_ostream & stream, const short int input){
    stream.buffer << input;
    return stream;
}

wii_ostream & operator<<(wii_ostream & stream, const short unsigned int input){
    stream.buffer << input;
    return stream;
}

wii_ostream & operator<<(wii_ostream & stream, const unsigned int input){
    stream.buffer << input;
    return stream;
}

wii_ostream & operator<<(wii_ostream & stream, const bool input){
    stream.buffer << input;
    return stream;
}

wii_ostream & operator<<(wii_ostream & stream, const long int input){
    stream.buffer << input;
    return stream;
}

wii_ostream & operator<<(wii_ostream & stream, const unsigned long int input){
    stream.buffer << input;
    return stream;
}

wii_ostream & operator<<(wii_ostream & stream, const void * input){
    stream.buffer << input;
    return stream;
}

wii_ostream & operator<<(wii_ostream & stream, std::ostream & (*f)(std::ostream &)){
    if (stream.enabled){
        printf("%s\n", stream.buffer.str().c_str());
    }
    stream.buffer.str("");
    stream.buffer.rdbuf()->pubseekoff(0, ios_base::end, ios_base::out);
    stream.buffer.clear();
    return stream;
}

wii_ostream wii_ostream::stream;
static wii_ostream nullcout(false);
#elif defined(NETWORK_DEBUG)
network_ostream::network_ostream(const std::string & host, int port, bool enabled):
host(host),
port(port),
enabled(enabled){
}

network_ostream & operator<<(network_ostream & stream, const std::string & input){
    stream.buffer << input;
    return stream;
}

network_ostream & operator<<(network_ostream & stream, const char * input){
    stream.buffer << input;
    return stream;
}

network_ostream & operator<<(network_ostream & stream, const char input){
    stream.buffer << input;
    return stream;
}

network_ostream & operator<<(network_ostream & stream, const double input){
    stream.buffer << input;
    return stream;
}

network_ostream & operator<<(network_ostream & stream, const int input){
    stream.buffer << input;
    return stream;
}

network_ostream & operator<<(network_ostream & stream, uint64_t input){
    stream.buffer << input;
    return stream;
}

network_ostream & operator<<(network_ostream & stream, const short int input){
    stream.buffer << input;
    return stream;
}

network_ostream & operator<<(network_ostream & stream, const short unsigned int input){
    stream.buffer << input;
    return stream;
}

network_ostream & operator<<(network_ostream & stream, const unsigned int input){
    stream.buffer << input;
    return stream;
}

network_ostream & operator<<(network_ostream & stream, const bool input){
    stream.buffer << input;
    return stream;
}

network_ostream & operator<<(network_ostream & stream, const long int input){
    stream.buffer << input;
    return stream;
}

#ifndef PS3
network_ostream & operator<<(network_ostream & stream, const unsigned long int input){
    stream.buffer << input;
    return stream;
}
#endif

network_ostream & operator<<(network_ostream & stream, const void * input){
    stream.buffer << input;
    return stream;
}

static void sendString(const std::string & host, int port, const std::string & data){
    int gateway = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = inet_addr(host.c_str());
    memset(address.sin_zero, '\0', sizeof(address.sin_zero));
    connect(gateway, (struct sockaddr*) &address, sizeof(address));
    send(gateway, data.c_str(), data.size(), 0);
    send(gateway, "\n", 1, 0);
    close(gateway);
}

network_ostream & operator<<(network_ostream & stream, std::ostream & (*f)(std::ostream &)){
    if (stream.enabled){
        sendString(stream.host, stream.port, stream.buffer.str());
        // printf("%s\n", stream.buffer.str().c_str());
    }
    stream.buffer.str("");
    stream.buffer.rdbuf()->pubseekoff(0, ios_base::end, ios_base::out);
    stream.buffer.clear();
    return stream;
}

network_ostream network_ostream::stream("192.168.1.100", 5670);
static network_ostream nullcout("", 0, false);
#else
class nullstreambuf_t: public std::streambuf {
public:
    nullstreambuf_t():std::streambuf(){
    }
};

static nullstreambuf_t nullstreambuf;

class nullcout_t: public std::ostream {
public:
    nullcout_t():std::ostream(&nullstreambuf){
    }
};

static nullcout_t nullcout;
#endif

#ifdef ANDROID
stream_type & default_stream = android_ostream::stream;
#elif defined(WII) && defined(DEBUG)
stream_type & default_stream = wii_ostream::stream;
#elif defined(NETWORK_DEBUG)
stream_type & default_stream = network_ostream::stream;
#else
stream_type & default_stream = std::cout;
#endif

}

Global::stream_type & Global::debug(int i, const string & context){
    if (global_debug_level >= i){
        Global::stream_type & out = default_stream;
        out << "[" << i << ":" << context << "] ";
        return out;
    }
    return nullcout;
}

void Global::setDebug( int i ){
    global_debug_level = i;
}

int Global::getDebug(){
    return global_debug_level;
}
