#include <string>
#include "language-string.h"

using namespace std;

LanguageString::LanguageString(const std::string & stuff):
string(stuff),
language("english"){
}

LanguageString::LanguageString(const std::string & stuff, const std::string & language):
string(stuff),
language(language){
}
