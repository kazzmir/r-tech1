#ifndef _time_difference_h
#define _time_difference_h

#include <time.h>
#include <sys/time.h>
#include <string>

using namespace std;

class TimeDifference{
public:

	TimeDifference();

	inline void startTime(){
		#ifndef WINDOWS
		gettimeofday( &start, NULL );
		#endif
	}

	inline void endTime(){
		#ifndef WINDOWS
		gettimeofday( &end, NULL );
		#endif
	}

	unsigned long long int getTime();

	void printTime();
	void printTime( const string & s );

	~TimeDifference();

protected:
	struct timeval start, end;

};

#endif
