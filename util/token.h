#ifndef _paintown_token_h
#define _paintown_token_h

#include <string>
#include <vector>
#include <ostream>
#include "token_exception.h"

class TokenReader;
class Configuration;
class Token;

class TokenMatcher{
public:
    /* TODO */
    template <typename X1>
    bool match(X1 & obj1){
        return false;
    }

    template <typename X1, typename X2>
    bool match(X1 & obj1, X2 & obj2){
        return false;
    }

    TokenMatcher & operator=(const TokenMatcher & matcher);

protected:
    TokenMatcher(std::vector<Token*> tokens);
    friend class Token;
};

/* Token:
 * Basically a tree where each node stores a value in a string
 * and can have 0 or more children
 */
class Token{
public:

    typedef TokenMatcher Matcher;

    Token(Token const & copy);
    virtual ~Token();

    void addToken(Token * t) throw (TokenException);

    /*
       inline const string & getName(){
       return name;
       }
       */
    const std::string & getName() const;
    const Token * getParent() const;

    void setFile( const std::string & s );
    const std::string getFileName() const;

    const std::string getLineage() const;

    void print( const std::string space );
    void toString( std::ostream & stream, const std::string & space );

    template <typename X>
    bool match(const std::string & subject, X & obj){
        TokenMatcher matcher = getMatcher(subject);
        return matcher.match(obj);
    }

    TokenMatcher getMatcher(const std::string & subject);

    Token * getToken( unsigned int n ) const;

    /* xpath-esque searching for tokens
     * '/' delimits tokens
     * <literal> matches a token
     */
    Token * findToken(const std::string & path);

    /* find all tokens */
    std::vector<Token *> findTokens(const std::string & path);

    inline signed int numTokens() const {
        return tokens.size() - 1;
    }

    inline const std::vector< Token * > * getTokens() const{
        return &tokens;
    }

    inline void resetToken(){
        num_token = 1;
    }

    /* returns a deep copy of this token. the parent field is set to null */
    Token * copy();

    Token * readToken();
    bool hasTokens();

    bool operator== ( const std::string rhs );
    bool operator!= ( const std::string rhs );

    Token & operator>>( std::string & rhs ) throw( TokenException );
    Token & operator>>( int & rhs ) throw( TokenException );
    Token & operator>>( double & rhs ) throw( TokenException );
    Token & operator>>( Token * & rhs ) throw( TokenException );
    Token & operator>>( bool & rhs ) throw( TokenException );

protected:
    /* Only TokenReader and Configuration can create and destroy a Token */
    Token();
    Token( std::string tok, bool parse = true );
    friend class TokenReader;
    friend class Configuration;

    Token & operator<<( const std::string rhs );
    Token & operator<<( const int rhs );
    Token & operator<<( const double rhs );

    virtual inline const std::string & _getName(){
        return name;
    }

    virtual inline void setParent( const Token * const parent ){
        this->parent = parent;
    }

    std::string lowerCase( const std::string & s );
    void finalize();

    unsigned int num_token;
    std::vector< Token * > tokens;
    std::string filename;
    Token const * parent;
    std::string name;
    bool own;
};

#endif
