#include <string>
/* gnu/posix's regex header */
#include <regex.h>
/* our regex header */
#include "regex.h"

using namespace std;
    
/* http://www.gnu.org/s/libc/manual/html_node/Regular-Expressions.html */
bool Util::matchRegex(const string & str, const string & pattern){
    regex_t regex;
    if (regcomp(&regex, pattern.c_str(), REG_EXTENDED) != 0){
        return false;
    }
    bool matched = regexec(&regex, str.c_str(), 0, NULL, 0) == 0;
    regfree(&regex);
    return matched;
}
    
string Util::captureRegex(const string & str, const string & pattern, int capture){
    regex_t regex;
    regmatch_t matches[20];
    if (regcomp(&regex, pattern.c_str(), REG_EXTENDED) != 0){
        return "";
    }
    bool matched = regexec(&regex, str.c_str(), 20, matches, 0) == 0;
    regfree(&regex);
    if (matched){
        if (matches[capture+1].rm_so != -1){
            int start = matches[capture+1].rm_so;
            int end = matches[capture+1].rm_eo;
            return str.substr(start, end - start);
        }
    }
    return "";
}
