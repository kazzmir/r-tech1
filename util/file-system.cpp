#include "funcs.h"
#include "file-system.h"
#include "system.h"
#include <sstream>
#include <exception>
#include <string>

using namespace std;

namespace Filesystem{

NotFound::NotFound(const std::string & file):
exception(),
reason(file){
}

NotFound::~NotFound() throw(){
}

static string userDirectory(){
	ostringstream str;
	str << getenv("HOME") << "/.paintown/";
	return str.str();
}

static string lookup(const std::string & path) throw (NotFound){
    /* first try the main data directory */
    string final = Util::getDataPath2() + path;
    if (System::readable(final)){
        return final;
    }
    /* then try the user directory, like ~/.paintown */
    final = userDirectory() + path;
    if (System::readable(final)){
        return final;
    }
    /* then just look in the cwd */
    if (System::readable(path)){
        return path;
    }
    throw NotFound("Cannot find " + path);
}

std::string find(const std::string & path) throw (NotFound){
    if (path.length() == 0){
        throw NotFound("No path given");
    }
    if (path[0] == '/'){
        string str(path);
        str.erase(0, 1);
        return lookup(str);
    }
    return lookup(path);
}

std::string cleanse(const std::string & path){
    string str = path;
    str.erase(0, Util::getDataPath2().length());
    return str;
}

}
