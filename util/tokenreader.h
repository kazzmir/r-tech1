#ifndef _tokenreader_h
#define _tokenreader_h

#include <fstream>
#include <string>
#include <vector>
#include "token_exception.h"

using namespace std;

class Token;

class TokenReader{
public:
	TokenReader( const char * filename );

	// returns a token which YOU must delete
	Token * readToken() throw( TokenException );

	~TokenReader();

protected:

	ifstream ifile;
	string myfile;
	vector< Token * > my_tokens;

};

#endif
