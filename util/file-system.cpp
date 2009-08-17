#include "funcs.h"
#include "file-system.h"
#include <exception>
#include <string>

using namespace std;

namespace Filesystem{

NotFound::NotFound(const std::string & file):
exception(){
}

NotFound::~NotFound() throw(){
}

std::string find(const std::string & path) throw (NotFound){
    if (path.length() == 0){
        throw NotFound("No path given");
    }
    if (path[0] == '/'){
        string str(path);
        str.erase(0, 1);
        return Util::getDataPath2() + str;
    }
    return Util::getDataPath2() + path;
}

std::string cleanse(const std::string & path){
    string str = path;
    str.erase(0, Util::getDataPath2().length());
    return str;
}

}
