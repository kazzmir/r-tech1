#ifndef _paintown_language_string_h
#define _paintown_language_string_h

#include <string>

class LanguageString: public std::string {
public:
    LanguageString(const std::string & stuff);
    LanguageString(const std::string & stuff, const std::string & language);

    const std::string & getLanguage() const {
        return language;
    }

protected:
    std::string language;
};

#endif
