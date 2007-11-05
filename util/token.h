#ifndef _token_h
#define _token_h

#include <string>
#include <vector>
#include "token_exception.h"

class TokenReader;

using namespace std;

/* Token:
 * Basically a tree where each node stores a value in a string
 * and can have 0 or more children
 */
class Token{
public:

	void addToken( Token * t );
	
	/*
	inline const string & getName(){
		return name;
	}
	*/
	const string & getName() const;
	const Token * const getParent() const;

	void setFile( const string & s );
	const string getFileName() const;

	const string getLineage() const;

	void print( const string & space );

	Token * getToken( unsigned int n );

	inline signed int numTokens() const{
		return tokens.size() - 1;
	}

	inline const vector< Token * > * getTokens() const{
		return &tokens;
	}

	inline void resetToken(){
		num_token = 1;
	}

	Token * readToken();
	bool hasTokens();

	bool operator== ( const string & rhs );
	bool operator!= ( const string & rhs );

	Token & operator>>( string & rhs ) throw( TokenException );
	Token & operator>>( int & rhs ) throw( TokenException );
	Token & operator>>( double & rhs ) throw( TokenException );
	Token & operator>>( Token * & rhs ) throw( TokenException );
	Token & operator>>( bool & rhs ) throw( TokenException );


protected:
	/* Only TokenReader can create and destroy a Token */
	Token();
	Token( string tok, bool parse = true );
	virtual ~Token();
	friend class TokenReader;

	virtual inline const string & _getName(){
		return name;
	}

	virtual inline void setParent( const Token * const parent ){
		this->parent = parent;
	}

	string lowerCase( const string & s );
	void finalize();

	unsigned int num_token;
	vector< Token * > tokens;
	string filename;
	Token const * parent;
	string name;
};

#endif
