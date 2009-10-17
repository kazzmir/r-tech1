#include <string>
#include "load_exception.h"

using namespace std;

LoadException::LoadException():
exception(){
}

LoadException::LoadException( const string reason ):
exception(){
	this->reason = reason;
}

LoadException::~LoadException() throw(){
}
