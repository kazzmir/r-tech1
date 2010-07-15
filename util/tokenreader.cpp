#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>

#include "token.h"
#include "token_exception.h"
#include "tokenreader.h"

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

    char open_paren = 'x';
    int parens = 1;
    while (input.good() && open_paren != '('){
        input >> open_paren;
    }
    // token_string += '(';

    Token * cur_token = new Token();
    // cur_token->setFile(myfile);
    my_tokens.push_back( cur_token );
    Token * first = cur_token;
    vector< Token * > token_stack;
    /* tokens that were ignored using ;@, and should be deleted */
    vector<Token*> ignore_list;
    token_stack.push_back( cur_token );

    /* when a ;@ is seen, read the next s-expression but throw it away */
    bool do_ignore = false;

    char n;
    string cur_string = "";

    /* in_quote is true if a " is read and before another " is read */
    bool in_quote = false;

    /* escaped unconditionally adds the next character to the string */
    bool escaped = false;
    while ( !token_stack.empty() ){
        if ( !input ){
            // cout<<__FILE__<<": "<<myfile<<" is bad. Open parens "<<parens<<endl;
            // cout<<"Dump: "<< token_string << "Last token = [" << n << "]" << (int)n << endl;
            // first->print( " " );
            ostringstream failure;
            failure << "Wrong number of parentheses. Open parens is " << parens;
            throw TokenException(__FILE__, __LINE__, failure.str());
        }
        // char n;
        // slow as we go
        input >> n;

        const char * alpha = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./-_!:";
        const char * nonalpha = " ;()#\"";
        // cout<<"Alpha char: "<<n<<endl;

        if ( escaped ){
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

        if ( n == '\\' ){
            escaped = true;
            continue;
        }

        if ( in_quote ){
            if ( n == '"' ){
                in_quote = false;

                Token * sub = new Token( cur_string, false );
                sub->setParent( cur_token );
                if (do_ignore){
                    ignore_list.push_back(sub);
                    do_ignore = false;
                } else {
                    cur_token->addToken( sub );
                }
                cur_string = "";

            } else
                cur_string += n;

        } else {
            if ( n == '"' )
                in_quote = true;

            if ( strchr( alpha, n ) != NULL ){
                cur_string += n;
            } else if ( cur_string != "" && strchr( nonalpha, n ) != NULL ){
                // cout<<"Made new token "<<cur_string<<endl;
                Token * sub = new Token( cur_string, false );
                sub->setParent( cur_token );
                if (do_ignore){
                    do_ignore = false;
                    ignore_list.push_back(sub);
                } else {
                    cur_token->addToken( sub );
                }
                cur_string = "";
            }
        }

        if ( n == '#' || n == ';' ){
            input >> n;
            if (n == '@'){
                do_ignore = true;
            } else {
                while ( n != '\n' && !input.eof() ){
                    input >> n;
                }
                continue;
            }
        } else if ( n == '(' ){
            Token * another = new Token();
            another->setParent( cur_token );

            if (do_ignore){
                ignore_list.push_back(another);
                do_ignore = false;
            } else {
                cur_token->addToken(another);
            }

            cur_token = another;
            token_stack.push_back( cur_token );
            /*
               parens++;
               cout<<"Inc Parens is "<<parens<<endl;
               */
        } else if ( n == ')' ){

            if ( token_stack.empty() ){
                cout<<"Stack is empty"<<endl;
                throw TokenException(__FILE__, __LINE__, "Stack is empty");
            }
            token_stack.pop_back();

            if ( ! token_stack.empty() ){
                cur_token = token_stack.back();
            }
        }
    }

    for (vector<Token*>::iterator it = ignore_list.begin(); it != ignore_list.end(); it++){
        delete (*it);
    }

    // first->print("");
    first->finalize();
    // return first;
}
