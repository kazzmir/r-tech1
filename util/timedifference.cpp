#include "timedifference.h"
#include <sstream>
#include <iostream>
#include <string>
using namespace std;

TimeDifference::TimeDifference(){
	start.tv_usec = 0;
	start.tv_sec = 0;
	end.tv_usec = 0;
	end.tv_sec = 0;
}

const string TimeDifference::printTime(){
	return this->printTime("Function");
}

unsigned long long int TimeDifference::getTime(){
	unsigned long long int g = (end.tv_sec*1000000+end.tv_usec) - (start.tv_sec*1000000 + start.tv_usec );
	return g;
}

const string TimeDifference::printTime( const string & s ){

	unsigned long long int micro = (end.tv_sec*1000000+end.tv_usec) - (start.tv_sec*1000000 + start.tv_usec );
	unsigned long long int milli = micro / 1000;
	unsigned long long int sec = milli / 1000;

	//cout<<s<<" took "<<micro<<" microseconds / "<< milli << " milliseconds / " <<sec<< " seconds "<< endl;
	ostringstream o;
	o << s <<" took "<<micro<<" microseconds / "<< milli << " milliseconds / " <<sec<< " seconds";
	return o.str();

}

TimeDifference::~TimeDifference(){
}
