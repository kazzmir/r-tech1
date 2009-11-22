#ifndef _paintown_language_string_h
#define _paintown_language_string_h

#include <string>
#include <map>

class LanguageString: public std::string {
public:
    LanguageString();
    LanguageString(const char * stuff);
    LanguageString(const std::string & stuff);
    LanguageString(const std::string & stuff, const std::string & language);
    LanguageString(const LanguageString & language);
    LanguageString & operator=(const LanguageString & obj);

    static const std::string defaultLanguage(){
        return "english";
    }

    void add(const std::string & stuff, const std::string & language);

    const std::string & get(){
        return languages[defaultLanguage()];
    }

protected:
    std::map<std::string, std::string> languages;
};

#endif
