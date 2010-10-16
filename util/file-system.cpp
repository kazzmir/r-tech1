#ifdef USE_ALLEGRO
#include <allegro.h>

/* FIXME: replace with <winalleg.h> */
#ifdef _WIN32
#define BITMAP dummyBITMAP
#include <windows.h>
#undef BITMAP
#endif

#endif
#include "funcs.h"
#include "file-system.h"
#include "system.h"
#include "globals.h"
#include <dirent.h>
#include <sstream>
#include <exception>
#include <string>
#include <ostream>

#ifndef USE_ALLEGRO
/* some sfl symbols conflict with allegro */
#include "sfl/sfl.h"
#include "sfl/sfldir.h"
#endif

#ifdef _WIN32
#define _WIN32_IE 0x400
#include <shlobj.h>
#endif

using namespace std;

namespace Filesystem{
        
Exception::Exception(const std::string & where, int line, const std::string & file):
Exc::Base(where, line),
reason(file){
}

Exception::Exception(const std::string & where, int line, const Exc::Base & nested, const std::string & file):
Exc::Base(where, line, nested),
reason(file){
}
        
Exception::Exception(const Exception & copy):
Exc::Base(copy),
reason(copy.reason){
}

Exception::~Exception() throw (){
}
        
const std::string Exception::getReason() const {
    return reason;
}

NotFound::NotFound(const std::string & where, int line, const std::string & file):
Exception(where, line, file + string(" was not found")){
}

NotFound::NotFound(const std::string & where, int line, const Exc::Base & nested, const std::string & file):
Exception(where, line, nested, file + string(" was not found")){
}
        
NotFound::NotFound(const NotFound & copy):
Exception(copy){
}

NotFound::~NotFound() throw (){
}

IllegalPath::IllegalPath(const std::string & where, int line, const std::string & file):
Exception(where, line, file){
}

IllegalPath::IllegalPath(const std::string & where, int line, const Exc::Base & nested, const std::string & file):
Exception(where, line, nested, file){
}
        
IllegalPath::IllegalPath(const IllegalPath & copy):
Exception(copy){
}

IllegalPath::~IllegalPath() throw(){
}

#ifdef _WIN32
AbsolutePath userDirectory(){
    ostringstream str;
    char path[MAX_PATH];
    SHGetSpecialFolderPathA(0, path, CSIDL_APPDATA, false);
    str << path << "/paintown/";
    return AbsolutePath(str.str());
}

AbsolutePath configFile(){
    ostringstream str;
    char path[MAX_PATH];
    SHGetSpecialFolderPathA(0, path, CSIDL_APPDATA, false);
    str << path << "/paintown_configuration.txt";
    return AbsolutePath(str.str());
}
#else
AbsolutePath configFile(){
    ostringstream str;
    /* what if HOME isn't set? */
    str << getenv("HOME") << "/.paintownrc";
    return AbsolutePath(str.str());
}

AbsolutePath userDirectory(){
    ostringstream str;
    /* what if HOME isn't set? */
    str << getenv("HOME") << "/.paintown/";
    return AbsolutePath(str.str());
}
#endif

static AbsolutePath lookup(const RelativePath path) throw (NotFound){
    /* first try the main data directory */
    AbsolutePath final = Util::getDataPath2().join(path);
    if (System::readable(final.path())){
        return final;
    }
    /* then try the user directory, like ~/.paintown */
    final = userDirectory().join(path);
    if (System::readable(final.path())){
        return final;
    }
    /* then just look in the cwd */
    if (System::readable(path.path())){
        return AbsolutePath(path.path());
    }

    ostringstream out;
    out << "Cannot find " << path.path() << ". I looked in '" << Util::getDataPath2().join(path).path() << "', '" << userDirectory().join(path).path() << "', and '" << path.path() << "'" << endl;

    throw NotFound(__FILE__, __LINE__, out.str());
}

static vector<AbsolutePath> findDirectoriesIn(const AbsolutePath & path){
    vector<AbsolutePath> dirs;
    DIR * dir = opendir(path.path().c_str());
    if (dir == NULL){
        return dirs;
    }

    struct dirent * entry = readdir(dir);
    while (entry != NULL){
        if (string(entry->d_name) != "." && string(entry->d_name) != ".."){
            string total = path.path() + "/" + entry->d_name;
            if (System::isDirectory(total)){
                dirs.push_back(AbsolutePath(total));
            }
        }
        entry = readdir(dir);
    }

    closedir(dir);

    return dirs;
}

vector<AbsolutePath> findDirectories(const RelativePath & path){
    typedef vector<AbsolutePath> Paths;
    Paths dirs;

    Paths main_dirs = findDirectoriesIn(Util::getDataPath2().join(path));
    Paths user_dirs = findDirectoriesIn(userDirectory().join(path));
    Paths here_dirs = findDirectoriesIn(Filesystem::AbsolutePath(path.path()));

    dirs.insert(dirs.end(), main_dirs.begin(), main_dirs.end());
    dirs.insert(dirs.end(), user_dirs.begin(), user_dirs.end());
    dirs.insert(dirs.end(), here_dirs.begin(), here_dirs.end());

    return dirs;
}

vector<AbsolutePath> getFiles(const AbsolutePath & dataPath, const string & find, bool caseInsensitive){
#ifdef USE_ALLEGRO
    struct al_ffblk info;
    vector<AbsolutePath> files;

    if ( al_findfirst( (dataPath.path() + "/" + find).c_str(), &info, FA_ALL ) != 0 ){
        return files;
    }
    files.push_back(AbsolutePath(dataPath.path() + "/" + string(info.name)));
    while ( al_findnext( &info ) == 0 ){
        files.push_back(AbsolutePath(dataPath.path() + "/" + string(info.name)));
    }
    al_findclose( &info );
    return files;
#else
    vector<AbsolutePath> files;
    DIRST sflEntry;
    bool ok = open_dir(&sflEntry, dataPath.path().c_str());
    while (ok){
        if (file_matches(sflEntry.file_name, find.c_str())){
            files.push_back(AbsolutePath(dataPath.path() + "/" + string(sflEntry.file_name)));
        }
        ok = read_dir(&sflEntry);
    }
    close_dir(&sflEntry);
    // Global::debug(0) << "Warning: Filesystem::getFiles() is not implemented yet for SDL" << endl;
    return files;
#endif
}

template <class X>
static void append(vector<X> & destination, const vector<X> & source){
    /*
    for (typename vector<X>::const_iterator it = source.begin(); it != source.end(); it++){
        destination.push_back(*it);
    }
    */
    copy(source.begin(), source.end(), back_insert_iterator<vector<X> >(destination));
}
    
static vector<AbsolutePath> getAllDirectories(const AbsolutePath & path){
    vector<AbsolutePath> all = findDirectoriesIn(path);
    vector<AbsolutePath> final;
    final.push_back(path);
    append(final, all);
    for (vector<AbsolutePath>::iterator it = all.begin(); it != all.end(); it++){
        vector<AbsolutePath> more = getAllDirectories(*it);
        append(final, more);
    }
    return final;
}

vector<AbsolutePath> getFilesRecursive(const AbsolutePath & dataPath, const string & find, bool caseInsensitive){
    vector<AbsolutePath> directories = getAllDirectories(dataPath);
    vector<AbsolutePath> files;
    for (vector<AbsolutePath>::iterator it = directories.begin(); it != directories.end(); it++){
        vector<AbsolutePath> found = getFiles(*it, find, caseInsensitive);
        append(files, found);
    }
    return files;
}

/* remove extra path separators (/) */
string sanitize(string path){
    size_t double_slash = path.find("//");
    while (double_slash != string::npos){
        path.erase(double_slash, 1);
        double_slash = path.find("//");
    }
    return path;
}

/*
std::string find(const std::string path){
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
*/

AbsolutePath find(const RelativePath & path){
    if (path.isEmpty()){
        throw NotFound(__FILE__, __LINE__, "No path given");
    }

    AbsolutePath out = lookup(path);
    if (System::isDirectory(out.path())){
        return AbsolutePath(sanitize(out.path() + "/"));
    }
    return AbsolutePath(sanitize(out.path()));
}
    
bool exists(const RelativePath & path){
    try{
        AbsolutePath absolute = find(path);
        return true;
    } catch (const NotFound & found){
        return false;
    }
}

RelativePath cleanse(const AbsolutePath & path){
    string str = path.path();
    if (str.find(Util::getDataPath2().path()) == 0){
        str.erase(0, Util::getDataPath2().path().length());
    } else if (str.find(userDirectory().path()) == 0){
        str.erase(0, userDirectory().path().length());
    }
    return RelativePath(str);
}

static string dirname(string path){
    if (path.find("/") != string::npos ||
            path.find("\\") != string::npos){
        size_t rem = path.find_last_of("/");
        if (rem != string::npos){
            return path.substr(0, rem+1);
        }

        rem = path.find_last_of("\\");
        if (rem != string::npos){
            return path.substr(0, rem+1);
        }
    }

    return "";
}

std::string stripDir(const std::string & str){
    std::string temp = str;
    if (str.find( "/") != std::string::npos || str.find( "\\") != std::string::npos){
        size_t rem = temp.find_last_of( "/" );
        if (rem != std::string::npos){
            return str.substr(rem+1,str.size());
        }
        rem = temp.find_last_of( "\\" );
        if( rem != std::string::npos ){
            return str.substr(rem+1,str.size());
        }
    }
    return str; 
}

std::string removeExtension(const std::string & str){
    if (str.find(".") != std::string::npos){
        return str.substr(0, str.find_last_of("."));
    }
    return str;
}

const string & Path::path() const {
    return mypath;
}

bool Path::isEmpty() const {
    return mypath.empty();
}
        
Path::~Path(){
}

Path::Path(){
}

std::string invertSlashes(const string & str){
    string tempStr = str;
    if (tempStr.find('\\') != string::npos){
	for (int i = tempStr.size()-1; i>-1; --i){
	    if (tempStr[i] == '\\'){
                tempStr[i] = '/';
            }
	}
    }

    return tempStr;
}

Path::Path(const std::string & path):
mypath(sanitize(invertSlashes(path))){
}

Path::Path(const Path & path):
mypath(sanitize(invertSlashes(path.path()))){
}

RelativePath::RelativePath(){
}

RelativePath::RelativePath(const std::string & path):
Path(path){
    if (! path.empty() && path[0] == '/'){
        ostringstream out;
        out << "Relative path '" << path << "' cannot start with a /. Only absolute paths can start with /";
        throw IllegalPath(__FILE__, __LINE__, out.str());
    }
}

RelativePath::RelativePath(const RelativePath & path):
Path(path){
}

RelativePath RelativePath::getDirectory() const {
    return RelativePath(dirname(path()));
}

RelativePath RelativePath::getFilename() const {
    return RelativePath(stripDir(path()));
}

bool RelativePath::operator<(const RelativePath & path) const {
    return this->path() < path.path();
}
        
bool RelativePath::operator==(const RelativePath & path) const {
    return this->path() == path.path();
}

RelativePath RelativePath::join(const RelativePath & him) const {
    return RelativePath(sanitize(path() + "/" + him.path()));
}

RelativePath & RelativePath::operator=(const RelativePath & copy){
    setPath(copy.path());
    return *this;
}

AbsolutePath::AbsolutePath(){
}

AbsolutePath::AbsolutePath(const std::string & path):
Path(path){
}

AbsolutePath::AbsolutePath(const AbsolutePath & path):
Path(path){
}

AbsolutePath & AbsolutePath::operator=(const AbsolutePath & copy){
    setPath(copy.path());
    return *this;
}

bool AbsolutePath::operator==(const AbsolutePath & path) const {
    return this->path() == path.path();
}
        
bool AbsolutePath::operator<(const AbsolutePath & path) const {
    return this->path() < path.path();
}
        
AbsolutePath AbsolutePath::getDirectory() const {
    return AbsolutePath(dirname(path()));
}

AbsolutePath AbsolutePath::getFilename() const {
    return AbsolutePath(stripDir(path()));
}
        
AbsolutePath AbsolutePath::join(const RelativePath & path) const {
    return AbsolutePath(sanitize(this->path() + "/" + path.path()));
}

}
