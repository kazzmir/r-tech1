#ifndef _paintown_debug_h
#define _paintown_debug_h

#include <ostream>
#include <sstream>

namespace Global{

#ifdef ANDROID
class android_ostream: public std::ostream {
public:
    android_ostream(bool enabled = true);
    static android_ostream stream;
    /* make these private at some point */
public:
    bool enabled;
    std::ostringstream buffer;
};

typedef android_ostream stream_type;
android_ostream & operator<<(android_ostream & stream, const std::string & input);
android_ostream & operator<<(android_ostream & stream, const char * input);
android_ostream & operator<<(android_ostream & stream, const char);
android_ostream & operator<<(android_ostream & stream, const double);
android_ostream & operator<<(android_ostream & stream, const int);
android_ostream & operator<<(android_ostream & stream, const short int);
android_ostream & operator<<(android_ostream & stream, const short unsigned int);
android_ostream & operator<<(android_ostream & stream, const unsigned int);
android_ostream & operator<<(android_ostream & stream, const bool);
android_ostream & operator<<(android_ostream & stream, const long int);
android_ostream & operator<<(android_ostream & stream, const unsigned long int);
android_ostream & operator<<(android_ostream & stream, const void *);
android_ostream & operator<<(android_ostream & stream, std::ostream & (*f)(std::ostream &));
#else
typedef std::ostream stream_type;
#endif

void setDebug( int i );
int getDebug();
stream_type & debug(int i, const std::string & context = "paintown");

}

#endif
