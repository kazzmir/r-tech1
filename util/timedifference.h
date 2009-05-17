#ifndef _time_difference_h
#define _time_difference_h

#include <time.h>
#include <sys/time.h>
#include <string>

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

	const std::string printTime();
	const std::string printTime( const std::string & s );

	~TimeDifference();

protected:
	struct timeval start, end;

};

#endif
