#include "argument.h"
#include <string.h>

using std::vector;
using std::string;

bool Argument::isArg(const string & what) const {
    std::vector<std::string> match = keywords();
    for (std::vector<std::string>::iterator it = match.begin(); it != match.end(); it++){
        if (strcasecmp(it->c_str(), what.c_str()) == 0){
            return true;
        }
    }
    return false;
}

Argument::~Argument(){
}
    
ArgumentAction::~ArgumentAction(){
}
