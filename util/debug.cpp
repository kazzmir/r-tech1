#include "debug.h"
#include <iostream>
#include <string>

#ifdef ANDROID
#include <android/log.h>
#define ANDROID_LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "paintown", __VA_ARGS__)
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
