#ifndef _paintown_token_h
#define _paintown_token_h

#include <string>
#include <vector>
#include <ostream>
#include "token_exception.h"

class TokenReader;
class Configuration;
class Token;

class TokenView{
public:
    TokenView(std::vector<const Token *> tokens);
    TokenView(const TokenView & view);

    bool hasMore() const;

    TokenView & operator>>(const Token* & item);
    TokenView & operator>>(std::string & item);
    TokenView & operator>>(int & item);
    TokenView & operator>>(double & item);
        
protected:
    std::vector<const Token*> tokens;
    std::vector<const Token*>::iterator current;
};

class TokenMatcher{
public:
    template <typename X1>
    bool match(X1 & obj1);

    template <typename X1, typename X2>
    bool match(X1 & obj1, X2 & obj2);

    TokenMatcher & operator=(const TokenMatcher & matcher);

protected:
    TokenMatcher(std::vector<const Token*> tokens);
    explicit TokenMatcher();
    friend class Token;

    std::vector<const Token*> tokens;
    std::vector<const Token*>::iterator current;
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

    /* add an existing token to the tree */
    void addToken(Token * t);

    /* creates a new empty token and returns it */
    Token * newToken();

    /*
       inline const string & getName(){
       return name;
       }
       */
    const std::string & getName() const;
    const Token * getParent() const;
    /* get the original parent, the parent with no parents */
    const Token * getRootParent() const;

    void setFile( const std::string & s );
    const std::string getFileName() const;

    const std::string getLineage() const;

    void print( const std::string space );
    /* a pretty printed s-expression */
    void toString(std::ostream & stream, const std::string & space);
    /* no extra whitespace */
    void toStringCompact(std::ostream & stream);

    bool match(const std::string & subject) const {
        TokenMatcher matcher = getMatcher(subject);
        return false;
    }

    template <typename X>
    bool match(const std::string & subject, X & obj) const {
        TokenMatcher matcher = getMatcher(subject);
        return matcher.match(obj);
    }

    template <typename X1, typename X2>
    bool match(const std::string & subject, X1 & obj1, X2 & obj2) const {
        TokenMatcher matcher = getMatcher(subject);
        return matcher.match(obj1) &&
               matcher.match(obj2);
    }

    template <typename X1, typename X2, typename X3>
    bool match(const std::string & subject, X1 & obj1, X2 & obj2, X3 & obj3) const {
        TokenMatcher matcher = getMatcher(subject);
        return matcher.match(obj1) &&
               matcher.match(obj2) &&
               matcher.match(obj3);
    }

    template <typename X1, typename X2, typename X3, typename X4>
    bool match(const std::string & subject, X1 & obj1, X2 & obj2, X3 & obj3, X4 & obj4) const {
        TokenMatcher matcher = getMatcher(subject);
        return matcher.match(obj1) &&
               matcher.match(obj2) &&
               matcher.match(obj3) &&
               matcher.match(obj4);
    }

    template <typename X1, typename X2, typename X3, typename X4, typename X5>
    bool match(const std::string & subject, X1 & obj1, X2 & obj2, X3 & obj3, X4 & obj4, X5 & obj5) const {
        TokenMatcher matcher = getMatcher(subject);
        return matcher.match(obj1) &&
               matcher.match(obj2) &&
               matcher.match(obj3) &&
               matcher.match(obj4) &&
               matcher.match(obj5);
    }

    TokenView view() const;

    TokenMatcher getMatcher(const std::string & subject) const;

    Token * getToken( unsigned int n ) const;

    /* xpath-esque searching for tokens
     * '/' delimits tokens
     * <literal> matches a token
     */
    const Token * findToken(const std::string & path) const;

    /* find all tokens. special characters _ and * */
    std::vector<const Token *> findTokens(const std::string & path) const;

    inline signed int numTokens() const {
        return tokens.size() - 1;
    }

    inline bool isData() const {
        return numTokens() == -1;
    }

    inline const std::vector< Token * > * getTokens() const{
        return &tokens;
    }

    inline void resetToken(){
        num_token = 1;
    }

    /* returns a deep copy of this token. the parent field is set to null */
    Token * copy() const;

    Token * readToken();
    bool hasTokens();

    bool operator== ( const std::string & rhs ) const;
    bool operator!= ( const std::string & rhs ) const;

    Token & operator>>( std::string & rhs ) throw( TokenException );
    Token & operator>>( int & rhs ) throw( TokenException );
    Token & operator>>( double & rhs ) throw( TokenException );
    Token & operator>>( Token * & rhs ) throw( TokenException );
    Token & operator>>( bool & rhs ) throw( TokenException );
    
public:
// protected:
    /* Only TokenReader and Configuration can create and destroy a Token */
    Token();
    Token( std::string tok, bool parse = true );
    friend class TokenReader;
    friend class Configuration;

    Token & operator<<( const std::string rhs );
    Token & operator<<( const int rhs );
    Token & operator<<(Token * token);
    Token & operator<<( const double rhs );

    virtual inline const std::string & _getName(){
        return name;
    }

    virtual inline void setParent( const Token * const parent ){
        this->parent = parent;
    }

    std::string lowerCase(const std::string & s) const;
    void finalize();

    unsigned int num_token;
    std::vector< Token * > tokens;
    std::string filename;
    Token const * parent;
    std::string name;
    bool own;
};


template <typename X1>
bool TokenMatcher::match(X1 & obj1){
    if (current == tokens.end()){
        return false;
    }

    const Token * token = *current;
    current++;

    try{
        token->view() >> obj1;
        return true;
    } catch (const TokenException & t){
    }

    return false;
}

template <typename X1, typename X2>
bool TokenMatcher::match(X1 & obj1, X2 & obj2){
    if (current == tokens.end()){
        return false;
    }

    const Token * token = *current;
    current++;

    try{
        token->view() >> obj1 >> obj2;
        return true;
    } catch (const TokenException & t){
    }

    return false;

}

#endif
