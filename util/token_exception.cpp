#include <string>
#include "token_exception.h"
	
TokenException::TokenException(const std::string & file, int line, const std::string reason):
Exception::Base(file, line),
reason(reason){
}
    
TokenException::TokenException(const TokenException & copy):
Exception::Base(copy),
reason(copy.reason){
}

Exception::Base * TokenException::copy() const {
    return new TokenException(*this);
}

TokenException::~TokenException() throw() {
}
