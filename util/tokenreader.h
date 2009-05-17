#ifndef _tokenreader_h
#define _tokenreader_h

#include <fstream>
#include <string>
#include <vector>
#include "token_exception.h"

class Token;

class TokenReader{
public:
	TokenReader( const std::string & s );
	TokenReader( const char * filename );

	virtual Token * readToken() throw (TokenException);

	virtual ~TokenReader();

protected:

        std::ifstream ifile;
        std::string myfile;
        std::vector< Token * > my_tokens;
};

#endif
