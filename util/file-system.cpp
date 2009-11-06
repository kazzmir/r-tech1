#include "funcs.h"
#include "file-system.h"
#include "system.h"
#include <dirent.h>
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

static vector<string> findDirectoriesIn(const std::string & path){
    vector<string> dirs;
    DIR * dir = opendir(path.c_str());
    if (dir == NULL){
        return dirs;
    }

    struct dirent * entry = readdir(dir);
    while (entry != NULL){
        string total = path + "/" + entry->d_name;
        if (System::isDirectory(total)){
            dirs.push_back(total);
        }
        entry = readdir(dir);
    }

    closedir(dir);

    return dirs;
}

vector<string> findDirectories(const std::string & path){
    vector<string> dirs;

    vector<string> main_dirs = findDirectoriesIn(Util::getDataPath2() + path);
    vector<string> user_dirs = findDirectoriesIn(userDirectory() + path);
    vector<string> here_dirs = findDirectoriesIn(path);

    dirs.insert(dirs.end(), main_dirs.begin(), main_dirs.end());
    dirs.insert(dirs.end(), user_dirs.begin(), user_dirs.end());
    dirs.insert(dirs.end(), here_dirs.begin(), here_dirs.end());

    return dirs;
}

/* remove extra path separators (/) */
static string sanitize(string path){
    size_t double_slash = path.find("//");
    while (double_slash != string::npos){
        path.erase(double_slash, 1);
        double_slash = path.find("//");
    }
    return path;
}

std::string find(const std::string path) throw (NotFound){
    if (path.length() == 0){
        throw NotFound("No path given");
    }
    if (path[0] == '/'){
        string str(path);
        str.erase(0, 1);
        string out = lookup(str);
        if (System::isDirectory(out)){
            return sanitize(out + "/");
        }
        return sanitize(out);
    }
    string out = lookup(path);
    if (System::isDirectory(out)){
        return sanitize(out + "/");
    }
    return sanitize(out);
}

std::string cleanse(const std::string & path){
    string str = path;
    if (str.find(Util::getDataPath2()) == 0){
        str.erase(0, Util::getDataPath2().length());
    } else if (str.find(userDirectory()) == 0){
        str.erase(0, userDirectory().length());
    }
    return str;
}

}
