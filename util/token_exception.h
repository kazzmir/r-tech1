#ifndef _token_exception_h
#define _token_exception_h

#include <exception>
#include <string>

using namespace std;

class TokenException : public exception{
public:
	TokenException();
	TokenException( const string & reason );

	~TokenException() throw();

	inline const string & getReason() const{
		return reason;
	}

protected:
	string reason;

};

#endif
