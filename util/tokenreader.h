#ifndef _tokenreader_h
#define _tokenreader_h

#include <fstream>
#include <string>
#include <vector>
#include "token_exception.h"

class Token;

class TokenReader{
public:
    TokenReader();

    virtual Token * readToken(const std::string & path) throw (TokenException);
    virtual Token * readToken(const char * path) throw (TokenException);
    virtual Token * readTokenFromString(const std::string & stuff) throw (TokenException);
    virtual Token * readTokenFromFile(const std::string & path);

    virtual ~TokenReader();

protected:
    /* deprecated */
    TokenReader( const std::string & s );
    TokenReader( const char * filename );
    virtual Token * readToken();

    virtual void readTokens(std::istream & stream) throw (TokenException);
    /*
    std::ifstream ifile;
    std::string myfile;
    */
    std::vector<Token *> my_tokens;
};

#endif
