#ifndef _time_difference_h
#define _time_difference_h

#include <time.h>
#include <sys/time.h>
#include <string>

class TimeDifference{
public:

	TimeDifference();

	void startTime();
	void endTime();

	unsigned long long int getTime();

	const std::string printTime();
	const std::string printTime( const std::string & s );

	~TimeDifference();

protected:
	unsigned long long start, end;

};

#endif
