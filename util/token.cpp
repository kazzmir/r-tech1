#include "token.h"
#include "token_exception.h"
#include <string>
#include <vector>
#include <ostream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <string.h>

using namespace std;

Token::Token():
num_token(1),
parent( NULL ),
own(true){
	name = "HEAD";
}

Token::Token( string tok, bool parse ):
num_token( 1 ),
parent( NULL ),
own(true){

	/* legacy code, not used much */
	if ( !parse ){
		name = tok;
		while ( name.find(' ' ) == 0 )
			name.erase( 0, 1 );
		// lowerCase( name );
		return;
	}
	
}
    
Token::Token(Token const & copy):
num_token(1),
parent(copy.parent),
own(false){
    this->tokens = copy.tokens;
    this->name = copy.name;
    this->filename = copy.filename;
}

/* Dump token to the screen */
void Token::print( const string space ) const {
    cout<<space<<"Token: "<< getName() << endl;
    for ( signed int i = 0; i < numTokens(); i++ ){
        Token * x = getToken( i );
        x->print( space + " |--" );
    }
}

void Token::toStringCompact(ostream & stream) const {
    if (numTokens() == -1){
        stream << getName();
    } else {
        stream << "(" << getName();
        for (signed int i = 0; i < numTokens(); i++){
            Token * x = getToken(i);
            stream << " ";
            x->toStringCompact(stream);
        }
        stream << ")";
    }
}

void Token::toString(ostream & stream, const string & space) const {
    if (numTokens() == -1){
        stream << getName();
    } else {
        stream << endl;
        stream << space << "(" << getName();
        for ( signed int i = 0; i < numTokens(); i++ ){
            Token * x = getToken(i);
            stream << " ";
            x->toString( stream, space + "  " );
        }
        stream << ")";
    }
}

/* helper function */
string Token::lowerCase( const string & s ) const {
    string ret = s;
    for ( unsigned int q = 0; q < s.length(); q++ ){
        if ( s[q] >= 'A' && s[q] <= 'Z' ){
            ret[q] = s[q] - 'A' + 'a';
        } else {
            // ret[q] = s[q];
        }
    }
    return ret;
}
	
/* Return next token and increment the internal position
 * of the current token
 */
Token * Token::readToken(){
    if ( num_token < tokens.size() ){
        return tokens[ num_token++ ];
    }
    return NULL;
}
	
bool Token::hasTokens() const {
    return num_token < tokens.size();
}

/*
vector<Token*> Token::findTokens(const string & path){
    vector<Token*> whoba;
    cout << "whoba has " << whoba.size() << endl;
    whoba.push_back(this);
    if (whoba[0] != this){
        cout << "complete failure!!!" << endl;
    }
    int x = 2;
    x = x + 12;
    return whoba;
}
*/

vector<const Token *> Token::findTokens(const string & path) const {
    vector<const Token *> found;
    if (path == ""){
        return found;
    }

    size_t find = path.find('/');
    string self;
    if (find == string::npos){
        self = path;
    } else {
        self = path.substr(0, find);
    }

    /* a name of `_' means succeed with the current token no matter
     * what its called.
     * `*' means test all children. this is useful if you dont know where in the
     * tree a given node lives.
     * (... (... (... (foo ...))))
     * *\foo would find the foo token (I used backslash because forward slash will
     * kill the current c++ comment)
     */
    if (self == "*"){
        /* `*' and 'blah/stuff', test the current token for 'blah/stuff' */
        string rest = path.substr(find+1);
        vector<const Token*> more = findTokens(rest);
        found.insert(found.end(), more.begin(), more.end());
        /* then test all children for the original path */
        for (int i = 0; i < numTokens(); i++){
            Token * next = getToken(i);
            if (next != NULL){
                vector<const Token *> more = next->findTokens(path);
                found.insert(found.end(), more.begin(), more.end());
            }
        }
    } else if (self == "_" || *this == self){
        if (find == string::npos){
            found.push_back(this);
            if (found[0] != this){
                cout << "internal consistency error!!!!" << endl;
                throw exception();
            }
        } else {
            string rest = path.substr(find+1);
            for (int i = 0; i < numTokens(); i++){
                Token * next = getToken(i);
                if (next != NULL){
                    vector<const Token *> more = next->findTokens(rest);
                    found.insert(found.end(), more.begin(), more.end());
                }
            }
        }
    }

    return found;
}
   
TokenMatcher Token::getMatcher(const std::string & subject) const {
    TokenMatcher matcher(findTokens(subject));
    return matcher;
}

const Token * Token::findToken(const string & path) const {
    vector<const Token *> all = findTokens(path);
    if (all.size() == 0){
        return NULL;
    }
    return all[0];
}

/*
Token * Token::findToken(const string & path){
    if (path == ""){
        return NULL;
    }

    size_t find = path.find('/');
    string self;
    if (find == string::npos){
        self = path;
    } else {
        self = path.substr(0, find);
    }
    if (*this == self){
        if (find == string::npos){
            return this;
        } else {
            for (int i = 0; i < numTokens(); i++){
                Token * next = getToken(i);
                if (next != NULL){
                    Token * ok = next->findToken(path.substr(find+1));
                    if (ok != NULL){
                        return ok;
                    }
                }
            }
        }
    }
    return NULL;
}
*/
	
Token * Token::getToken( unsigned int n ) const {
	int q = numTokens();
	if ( q == -1 ) return NULL;
	if ( n >= 0 && (signed int)n < q )
		return tokens[n+1];
	return NULL;
}
	
/* If the token has children then the name of this token
 * is the name of the first child token.
 * Otherwise, the name is this token's name
 */
const string & Token::getName() const {
	if ( numTokens() != -1 ){
		return tokens[0]->_getName();
	}
	// cout<<"No tokens!!"<<endl;
	return name;
}

const Token * Token::getParent() const {
    return this->parent;
}

const Token * Token::getRootParent() const {
    if (getParent() != NULL){
        return getParent()->getRootParent();
    }
    return this;
}

const string Token::getLineage() const {
	if ( getParent() != NULL ){
		return getParent()->getLineage() + " -> " + getName();
	}

	return getName();
}

/* A token's identity is its name 
 */
bool Token::operator== (const string & rhs) const {
    return lowerCase(getName())  == lowerCase(rhs);
}

bool Token::operator!=(const string & rhs) const {
    return !(*this == rhs);
}

void Token::setFile( const string & s ){
	filename = s;
}

const string Token::getFileName() const {
	if ( parent ){
		return parent->getFileName();
	} else {
		return filename;
	}
}

/*
Token & Token::operator>>( Token * & rhs ) throw( TokenException ){
    Token * x = readToken();
    if ( x == NULL ){
        throw TokenException(__FILE__, __LINE__, getFileName() + ": " + string("Tried to read a token from ") + this->getName() + string(" but there are no more elements") );
    }
    rhs = x;
    return *this;
}

Token & Token::operator>>( string & rhs ) throw( TokenException ){
    Token * token = readToken();
    if (token == NULL){
        throw TokenException(__FILE__, __LINE__, getFileName() + ":" + string("Tried to read a string from '") + this->getLineage() + string("' but there no more elements") );
    }
    if (!token->isData()){
        ostringstream out;
        out << getFileName() << ": Element is not a string '";
        token->toString(out, "");
        // throw TokenException(__FILE__, __LINE__, getFileName() + ":" + string(" Element is not a string: "));
        throw TokenException(__FILE__, __LINE__, out.str());
    }

    rhs = token->getName();

    // rhs = getName();

    return *this;
}
	
Token & Token::operator>>( int & rhs ) throw( TokenException ){
	Token * l = readToken();
	if ( l == NULL ){
		throw TokenException(__FILE__, __LINE__, getFileName() + ": " + string("Tried to read an int from ") + this->getLineage() + string(" but there are no more elements") );
	}
        if (!l->isData()){
            throw TokenException(__FILE__, __LINE__, getFileName() + ":" + string(" Element is not a string"));
        }
	istringstream is ( l->getName() );
	is >> rhs;
	return *this;
}

Token & Token::operator>>( double & rhs ) throw( TokenException ){
	Token * l = readToken();
	if ( l == NULL ){
		throw TokenException(__FILE__, __LINE__, getFileName() + ": " + string("Tried to read a double from ") + this->getLineage() + string(" but there no more elements") );
	}
        if (!l->isData()){
            throw TokenException(__FILE__, __LINE__, getFileName() + ":" + string(" Element is not a string"));
        }
	istringstream is ( l->getName() );
	is >> rhs;
	return *this;
}

Token & Token::operator>>( bool & rhs ) throw( TokenException ){
	Token * l = readToken();
	if ( l == NULL ){
		throw TokenException(__FILE__, __LINE__, getFileName() + ": " + string("Tried to read a bool from ") + this->getLineage() + string(" but there no more elements") );
	}
        if (!l->isData()){
            throw TokenException(__FILE__, __LINE__, getFileName() + ":" + string(" Element is not a string"));
        }
	istringstream is ( l->getName() );
	is >> rhs;
	return *this;
}
*/

void Token::addToken(Token * t){
    /*
    if (!own){
        throw TokenException(__FILE__, __LINE__, "This token does not own its own tokens, so you cannot add tokens to it");
    }
    */
    tokens.push_back( t );
}

Token * Token::newToken(){
    Token * token = new Token();
    addToken(token);
    return token;
}

TokenView Token::view() const {
    vector<const Token*> out;
    out.insert(out.end(), tokens.begin(), tokens.end());
    return TokenView(out);
}

static bool needQuotes(const std::string & what){
    /* ripped from tokenreader.cpp, maybe use a variable for nice sharing.. */
    const char * alpha = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./-_!:";
    for (unsigned int position = 0; position < what.size(); position++){
        if (strchr(alpha, what[position]) == NULL){
            return true;
        }

        /*
        if (what[position] == '"' ||
            what[position] == ' ' ||
            (unsigned char) what[position] > 127){
            return true;
        }
        */
    }

    return false;
}

/* put quotes around a string if there are spaces in it */
static string quoteify(const string & rhs){
    if (needQuotes(rhs)){
        return "\"" + rhs + "\"";
    }

    return rhs;
}

Token & Token::operator<<(Token * token){
    if (!own){
        throw TokenException(__FILE__, __LINE__, "Cannot add tokens to a token you don't own");
    }
    this->addToken(token);
    return *this;
}

Token & Token::operator<<( const string rhs ){
    if (!own){
        throw TokenException(__FILE__, __LINE__, "Cannot add raw strings to a token you don't own");
    }

    Token * n = new Token(quoteify(rhs), false );
    this->addToken(n);
    return *this;
}

Token & Token::operator<<( const int rhs ){
    if (!own){
        throw TokenException(__FILE__, __LINE__, "Cannot add raw integers to a token you don't own");
    }

    ostringstream o;
    o << rhs;
    return *this << o.str();
}

Token & Token::operator<<( const double rhs ){
    if (!own){
        throw TokenException(__FILE__, __LINE__, "Cannot add raw doubles to a token you don't own");
    }

    ostringstream o;
    o << setprecision(6) << fixed << rhs;
    return *this << o.str();
}

Token * Token::copy() const {
    Token * token = new Token();
    token->filename = this->filename;
    token->name = this->name;
    for (vector<Token *>::const_iterator it = this->tokens.begin(); it != this->tokens.end(); it++){
        Token * him = (*it)->copy();
        him->setParent(token);
        token->addToken(him);
    }
    return token;
}

/* Delete tokens that are commented.
 * A commented token has a '!' character as the first
 * character in the name, e.g:
 * (!a_token (child_token 2))
 */
void Token::finalize(){
	for ( vector< Token * >::iterator it = tokens.begin(); it != tokens.end(); ){
		Token * t = *it;
		if ( t->getName().find('!') == 0 ){
			delete t;
			it = tokens.erase( it );
		} else {
			t->finalize();
			it++;
		}
	}
}

Token::~Token(){
    if (own){
        for (vector<Token * >::iterator it = tokens.begin(); it != tokens.end(); it++){
            delete *it;
        }
    }
}

TokenMatcher::TokenMatcher(){
    throw std::exception();
}

TokenMatcher & TokenMatcher::operator=(const TokenMatcher & matcher){
    tokens = matcher.tokens;
    current = tokens.begin();
    return *this;
}

TokenMatcher::TokenMatcher(std::vector<const Token*> tokens):
tokens(tokens){
    current = this->tokens.begin();
}

TokenView::TokenView(std::vector<const Token *> tokens):
tokens(tokens){
    current = this->tokens.begin();
    if (current != this->tokens.end()){
        current++;
    }
}

TokenView::TokenView(const TokenView & view):
tokens(view.tokens){
    current = this->tokens.begin();
    if (current != this->tokens.end()){
        current++;
    }
}
    
bool TokenView::hasMore() const {
    return current != tokens.end();
}
    
TokenView & TokenView::operator=(const TokenView & view){
    tokens = view.tokens;
    current = tokens.begin();
    if (current != tokens.end()){
        current++;
    }
    return *this;
}

TokenView & TokenView::operator>>(string & item){
    if (current == tokens.end()){
        throw TokenException(__FILE__, __LINE__, "No more elements");
    }
    const Token * child = *current;
    if (!child->isData()){
        throw TokenException(__FILE__, __LINE__, "Token is not a datum");
    }
    item = child->getName();
    current++;
    return *this;
}

TokenView & TokenView::operator>>(int & item){
    if (current == tokens.end()){
        throw TokenException(__FILE__, __LINE__, "No more elements");
    }
    const Token * child = *current;
    if (!child->isData()){
        throw TokenException(__FILE__, __LINE__, "Token is not a datum");
    }
    istringstream out(child->getName());
    out >> item;
    current++;
    return *this;
}

TokenView & TokenView::operator>>(double & item){
    if (current == tokens.end()){
        throw TokenException(__FILE__, __LINE__, "No more elements");
    }
    const Token * child = *current;
    if (!child->isData()){
        throw TokenException(__FILE__, __LINE__, "Token is not a datum");
    }
    istringstream out(child->getName());
    out >> item;
    current++;
    return *this;
}

TokenView & TokenView::operator>>(bool & item){
    if (current == tokens.end()){
        throw TokenException(__FILE__, __LINE__, "No more elements");
    }
    const Token * child = *current;
    if (!child->isData()){
        throw TokenException(__FILE__, __LINE__, "Token is not a datum");
    }
    istringstream out(child->getName());
    out >> item;
    current++;
    return *this;
}

    
TokenView & TokenView::operator>>(const Token* & item){
    if (current == tokens.end()){
        throw TokenException(__FILE__, __LINE__, "No more elements");
    }
    const Token * child = *current;
    /*
    if (!child->isData()){
        throw TokenException(__FILE__, __LINE__, "Token is not a datum");
    }
    */
    item = child;
    current++;
    return *this;
}
