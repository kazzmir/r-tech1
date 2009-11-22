#include <string>
#include "language-string.h"

using namespace std;
    
LanguageString::LanguageString(){
}

LanguageString::LanguageString(const std::string & stuff){
    add(stuff);
}

LanguageString::LanguageString(const char * stuff){
    add(stuff);
}

LanguageString::LanguageString(const std::string & stuff, const std::string & language){
    add(stuff, language);
}

LanguageString::LanguageString(const LanguageString & language){
    languages = language.languages;
}
    
LanguageString & LanguageString::operator=(const LanguageString & obj){
    this->languages = obj.languages;
    return *this;
}
    
void LanguageString::add(const std::string & stuff, const std::string & language){
    languages[language] = stuff;
}

void LanguageString::add(const std::string & stuff){
    add(stuff, defaultLanguage());
}
    
const std::string & LanguageString::get(){
    if (languages[currentLanguage()] != ""){
        return languages[currentLanguage()];
    }
    return languages[defaultLanguage()];
}
