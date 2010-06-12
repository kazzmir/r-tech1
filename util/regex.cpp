#include <string>
#include <string.h>
/* gnu/posix's regex header */
// #include <regex.h>
// #include "trex/trex.h"
#include "pcre/pcre.h"
/* our regex header */
#include "regex.h"

using namespace std;
    
/* http://www.gnu.org/s/libc/manual/html_node/Regular-Expressions.html */
bool Util::matchRegex(const string & str, const string & pattern){
    pcre * regex;
    const char * error;
    int errorOffset;
    int count;
    regex = pcre_compile(pattern.c_str(), 0, &error, &errorOffset, NULL);
    if (regex == NULL){
        return false;
    }

    count = pcre_exec(regex, NULL, str.c_str(), str.size(), 0, 0, NULL, 0);
    pcre_free(regex);
    return count >= 0;

    /*
    TRex * regex;
    regex = trex_compile(pattern.c_str(), NULL);
    if (regex == NULL){
        return false;
    }

    bool matched = trex_match(regex, str.c_str());
    trex_free(regex);
    return matched;
    */

    /*
    regex_t regex;
    if (regcomp(&regex, pattern.c_str(), REG_EXTENDED) != 0){
        return false;
    }
    bool matched = regexec(&regex, str.c_str(), 0, NULL, 0) == 0;
    regfree(&regex);
    return matched;
    */
}
    
string Util::captureRegex(const string & str, const string & pattern, int capture){
    pcre * regex;
    const char * error;
    int errorOffset;
    int count;
    const int captureMax = 100;
    int captures[captureMax];
    regex = pcre_compile(pattern.c_str(), 0, &error, &errorOffset, NULL);
    if (regex == NULL){
        return false;
    }

    count = pcre_exec(regex, NULL, str.c_str(), str.size(), 0, 0, captures, captureMax);
    pcre_free(regex);
    if (count >= 0 && capture < count){
        int start = captures[(capture + 1) * 2];
        int end = captures[(capture + 1) * 2 + 1];
        return str.substr(start, end - start);
    }
    
    return "";
#if 0
    TRex * regex;
    regex = trex_compile(pattern.c_str(), NULL);
    if (regex == NULL){
        return "";
    }

    string out = "";
    bool matched = trex_match(regex, str.c_str());
    if (matched){
        TRexMatch match;
        /* match 0 is the full string matched */
        if (trex_getsubexp(regex, capture + 1, &match)){
            /* WARNING: hack.. */
            static char tmp[2048];
            memset(tmp, 0, sizeof(tmp));
            strncpy(tmp, match.begin, match.len < (int) sizeof(tmp) ? match.len : sizeof(tmp));
            out = tmp;
        }
    }

    trex_free(regex);
    return out;
#endif

    /* FIXME */

    /*
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
    */
}
