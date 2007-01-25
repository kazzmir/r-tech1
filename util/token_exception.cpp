#include <exception>
#include <string>
#include "token_exception.h"
	
TokenException::TokenException():
exception(){
}

TokenException::TokenException( const string & reason ):
exception(){
	this->reason = reason;
}
	
TokenException::~TokenException() throw() {
}
