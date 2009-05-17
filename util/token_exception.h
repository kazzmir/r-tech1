#ifndef _token_exception_h
#define _token_exception_h

#include <exception>
#include <string>

class TokenException : public std::exception {
public:
    TokenException();
    TokenException( const std::string & reason );

    virtual ~TokenException() throw();

    inline const std::string & getReason() const{
        return reason;
    }

protected:
        std::string reason;
};

#endif
