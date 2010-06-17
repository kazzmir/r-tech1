#ifndef _paintown_token_exception_h
#define _paintown_token_exception_h

#include "exceptions/exception.h"
#include <string>

class TokenException: public Exception::Base {
public:
    TokenException(const std::string & file, int line, const std::string reason = "");

    virtual ~TokenException() throw();

    inline const std::string & getReason() const{
        return reason;
    }

    TokenException(const TokenException & copy);

protected:
    virtual Exception::Base * copy() const;

    std::string reason;
};

#endif
