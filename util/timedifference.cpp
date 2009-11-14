#include "timedifference.h"
#include <sstream>
#include <iostream>
#include <string>
#include <math.h>

using namespace std;

TimeDifference::TimeDifference(){
	start.tv_usec = 0;
	start.tv_sec = 0;
	end.tv_usec = 0;
	end.tv_sec = 0;
}

const string TimeDifference::printTime(){
	return this->printTime("Function took");
}

unsigned long long int TimeDifference::getTime(){
	unsigned long long int g = (end.tv_sec*1000000+end.tv_usec) - (start.tv_sec*1000000 + start.tv_usec );
	return g;
}

static double roundit(double number, int digits){
    return (long long) (number * pow(10, digits)) / pow(10, digits);
}

const string TimeDifference::printTime(const string & s){

    double total = (unsigned long long) (end.tv_sec*1000000+end.tv_usec) - (unsigned long long) (start.tv_sec*1000000 + start.tv_usec );
    string units = "microseconds";

    int unit_times[] = {1000, 1000, 60};
    string unit_descriptions[] = {"milliseconds", "seconds", "minutes"};

    for (int index = 0; index < sizeof(unit_times) / sizeof(int); index++){
        if (total > unit_times[index]){
            total /= unit_times[index];
            units = unit_descriptions[index];
        } else {
            break;
        }
    }

    ostringstream o;
    o << s << " " << roundit(total, 3) << " " << units;
    return o.str();

    /*

	unsigned long long int micro = (end.tv_sec*1000000+end.tv_usec) - (start.tv_sec*1000000 + start.tv_usec );
	unsigned long long int milli = micro / 1000;
	unsigned long long int sec = milli / 1000;

	//cout<<s<<" took "<<micro<<" microseconds / "<< milli << " milliseconds / " <<sec<< " seconds "<< endl;
	ostringstream o;
	o << s <<" took "<<micro<<" microseconds / "<< milli << " milliseconds / " <<sec<< " seconds";
	return o.str();
        */
}

TimeDifference::~TimeDifference(){
}
