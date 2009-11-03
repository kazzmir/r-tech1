#include <string>
/* gnu/posix's regex header */
#include <regex.h>
/* our regex header */
#include "regex.h"

using namespace std;
    
/* http://www.gnu.org/s/libc/manual/html_node/Regular-Expressions.html */
bool Util::matchRegex(const string & str, const string & pattern){
    regex_t regex;
    if (regcomp(&regex, pattern.c_str(), 0) != 0){
        return false;
    }
    bool matched = regexec(&regex, str.c_str(), 0, NULL, 0) == 0;
    return matched;
}
