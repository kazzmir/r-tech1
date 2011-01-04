#ifndef _paintown_token_exception_h
#define _paintown_token_exception_h

#include "exceptions/exception.h"
#include <string>

class TokenException: public Exception::Base {
public:
    TokenException(const std::string & file, int line, const std::string reason = "");
    TokenException(const TokenException & copy);
    TokenException(const Exception::Base & base);

    virtual ~TokenException() throw();

    virtual void throwSelf() const {
        throw *this;
    }

protected:
    virtual Exception::Base * copy() const;

    virtual inline const std::string getReason() const {
        return reason;
    }

    std::string reason;
};

#endif
