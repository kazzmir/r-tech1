#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>

#include "token.h"
#include "token_exception.h"
#include "tokenreader.h"
#include "pointer.h"

using namespace std;

/* tokenreader reads a file formatted with s-expressions. examples:
 * (hello)
 * (hello world)
 * (hello "world")
 * (hello (world))
 * (hello (world hi))
 */
    
TokenReader::TokenReader(){
}

TokenReader::TokenReader(const char * file){
    readTokenFromFile(file);
}

TokenReader::TokenReader(const string & file){
    readTokenFromFile(file.c_str());
}

Token * TokenReader::readTokenFromFile(const char * path){
    ifstream file(path);
    if (!file){
        ostringstream out;
        out << "Could not open '" << path << "'";
        throw TokenException(__FILE__, __LINE__, out.str());
    }
    file >> noskipws;
    readTokens(file);
    file.close();
    if (my_tokens.size() > 0){
        return my_tokens[0];
    }
    ostringstream out;
    out << "No tokens read from " << path;
    throw TokenException(__FILE__, __LINE__, out.str());
}

Token * TokenReader::readToken(){
    if (my_tokens.size() > 0){
        return my_tokens[0];
    }
    throw TokenException(__FILE__, __LINE__, "No tokens read");
}
    
Token * TokenReader::readToken(const string & path) throw (TokenException){
    return readTokenFromFile(path.c_str());
}

Token * TokenReader::readToken(const char * path) throw (TokenException){
    return readTokenFromFile(path);
}
    
Token * TokenReader::readTokenFromString(const string & stuff) throw (TokenException){
    istringstream input(stuff);
    input >> noskipws;
    readTokens(input);
    if (my_tokens.size() > 0){
        return my_tokens[0];
    }
    throw TokenException(__FILE__, __LINE__, "No tokens read");
}

TokenReader::~TokenReader(){
    // ifile.close();

    /* tokenreader giveth, and tokenreader taketh */
    for ( vector< Token * >::iterator it = my_tokens.begin(); it != my_tokens.end(); it++ ){
        delete *it;
    }
}

void TokenReader::readTokens(istream & input) throw (TokenException){

    /*
    if ( !ifile ){
        ostringstream out;
        out << "Could not open '" << myfile << "'";
        throw TokenException(__FILE__, __LINE__, out.str());
    }
    */
    // Token * t;

    // string token_string;

    // char open_paren = 'x';
    int parens = 0;
    // int position = 0;
    /*
    while (input.good() && open_paren != '('){
        input >> open_paren;
        position += 1;
    }
    */
    // token_string += '(';

    Token * currentToken = NULL;
    // Token * cur_token = new Token();
    // cur_token->setFile(myfile);
    // my_tokens.push_back(cur_token);
    // Token * first = cur_token;
    vector<Token *> token_stack;
    /* tokens that were ignored using ;@, and should be deleted */
    vector<Util::ReferenceCount<Token> > ignore_list;
    // token_stack.push_back(cur_token);

    /* when a ;@ is seen, read the next s-expression but throw it away */
    bool do_ignore = false;

    unsigned char n;
    string cur_string = "";

    /* in_quote is true if a " is read and before another " is read */
    bool in_quote = false;

    /* escaped unconditionally adds the next character to the string */
    bool escaped = false;
    while (input.good() && !input.eof()){
            // char n;
        // slow as we go
        input >> n;
        if (input.eof()){
            break;
        }
        // position += 1;
        // printf("Read character '%c' %d at %d\n", n, n, input.tellg());
        // cout << "Read character '" << n << "' " << (int) n << " at " << input.tellg() << endl;

        const char * alpha = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./-_!:*";
        const char * nonalpha = " ;()#\"";
        // cout<<"Alpha char: "<<n<<endl;

        if (escaped){
            switch (n){
                case 'n' : {
                    cur_string += "\n";
                    break;
                }
                default : {
                    cur_string += n;
                    break;
                }
            }
            escaped = false;
            continue;
        }

        if (n == '\\'){
            escaped = true;
            continue;
        }

        if (in_quote){
            if (n == '"'){
                in_quote = false;

                Token * sub = new Token(cur_string, false);
                if (currentToken != NULL){
                    sub->setParent(currentToken);
                    if (do_ignore){
                        ignore_list.push_back(sub);
                        do_ignore = false;
                    } else {
                        currentToken->addToken(sub);
                    }
                    cur_string = "";
                } else {
                    delete sub;
                }
            } else {
                cur_string += n;
            }
        } else {
            /* not in a quote */

            if (n == '"'){
                in_quote = true;
            } else if (strchr(alpha, n) != NULL){
                cur_string += n;
            } else if (cur_string != "" && strchr(nonalpha, n) != NULL){
                // cout<<"Made new token "<<cur_string<<endl;
                Token * sub = new Token(cur_string, false);
                if (currentToken != NULL){
                    sub->setParent(currentToken);
                    if (do_ignore){
                        do_ignore = false;
                        ignore_list.push_back(sub);
                    } else {
                        currentToken->addToken(sub);
                    }
                } else {
                    delete sub;
                }
                cur_string = "";
            }

            if (n == '#' || n == ';'){
                input >> n;

                /* if the next character is a @ then ignore the next entire s-expression
                 * otherwise just ignore the current line
                 */
                if (n == '@'){
                    do_ignore = true;
                } else {
                    while (n != '\n' && !input.eof()){
                        input >> n;
                    }
                    continue;
                }
            } else if (n == '('){
                Token * another = new Token();
                if (currentToken != NULL){
                    another->setParent(currentToken);
                }

                if (do_ignore){
                    ignore_list.push_back(another);
                    do_ignore = false;
                } else {
                    if (currentToken != NULL){
                        currentToken->addToken(another);
                    } else {
                        /* top level token */
                        my_tokens.push_back(another);
                    }
                }

                parens += 1;
                currentToken = another;
                token_stack.push_back(currentToken);
            } else if (n == ')'){
                parens -= 1;
                if (token_stack.empty()){
                    // cout << "Stack is empty"<<endl;
                    throw TokenException(__FILE__, __LINE__, "Stack is empty");
                }
                token_stack.pop_back();

                if (! token_stack.empty()){
                    currentToken = token_stack.back();
                } else {
                    currentToken = NULL;
                }
            }
        }
    }

    if (!token_stack.empty()){
        ostringstream failure;
        failure << "Wrong number of parentheses. Open parens is " << parens;
        throw TokenException(__FILE__, __LINE__, failure.str());
    }

    for (vector<Token*>::iterator it = my_tokens.begin(); it != my_tokens.end(); it++){
        Token * token = *it;
        token->finalize();
    }
}
