#include <exception>
#include <string>
#include "token_exception.h"
	
TokenException::TokenException():
std::exception(){
}

TokenException::TokenException( const std::string & reason ):
std::exception(){
	this->reason = reason;
}
	
TokenException::~TokenException() throw() {
}
