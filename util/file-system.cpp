#ifdef USE_ALLEGRO
#include <allegro.h>

/* FIXME: replace with <winalleg.h> */
#ifdef _WIN32
#define BITMAP dummyBITMAP
#include <windows.h>
#undef BITMAP
#endif

#endif
#include <algorithm>
#include "funcs.h"
#include "file-system.h"
#include "thread.h"
#include "system.h"
#include "globals.h"
#include <dirent.h>
#include <sstream>
#include <exception>
#include <string>
#include <fstream>
#include <ostream>

#include "zip/unzip.h"
#include "zip/ioapi.h"

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

/* some filesystem access can only be done by one thread at a time. specifically, sfl
 * has its own allocator that is meant to be used in a single-threaded manner.
 * rather than try to add locks to sfl we just wrap all sfl calls with a lock.
 *
 * initialize() must be called to initialize this lock
 */
// Util::Thread::Lock lock;



namespace Path{

/* remove extra path separators (/) */
string sanitize(string path){
    size_t double_slash = path.find("//");
    while (double_slash != string::npos){
        path.erase(double_slash, 1);
        double_slash = path.find("//");
    }
    return path;
}

static int invert(int c){
    if (c == '\\'){
        return '/';
    }
    return c;
}

std::string invertSlashes(string str){
    transform(str.begin(), str.end(), str.begin(), invert);
    return str;
}

const string & Path::path() const {
    return mypath;
}
        
const string Path::getExtension() const {
    size_t dot = mypath.rfind('.');
    if (dot == string::npos){
        return "";
    } else {
        return mypath.substr(dot + 1);
    }
}

bool Path::isEmpty() const {
    return mypath.empty();
}
        
Path::~Path(){
}

Path::Path(){
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
        throw Storage::IllegalPath(__FILE__, __LINE__, out.str());
    }
}

/* a/b/c/d -> b/c/d */
std::string stripFirstDir(const std::string & str){
    if (str.find("/") != std::string::npos || str.find( "\\") != std::string::npos){
        std::string temp = str;
        size_t rem = temp.find("/");
        if (rem != std::string::npos){
            return str.substr(rem+1,str.size());
        }
        rem = temp.find("\\");
        if( rem != std::string::npos ){
            return str.substr(rem+1,str.size());
        }
    }
    return str; 
}

static vector<string> splitPath(string path){
    vector<string> all;
    if (path.size() > 0 && path[0] == '/'){
        all.push_back("/");
    }
    size_t found = path.find('/');
    while (found != string::npos){
        if (found > 0){
            all.push_back(path.substr(0, found));
        }
        path.erase(0, found + 1);
        found = path.find('/');
    }   
    if (path.size() != 0){
        all.push_back(path);
    }   
    return all;
}

static string joinPath(const vector<string> & paths){
    ostringstream out;
    bool first = true;
    for (vector<string>::const_iterator it = paths.begin(); it != paths.end(); it++){
        if (!first){
            out << '/';
        }
        out << *it;
        first = false;
    }
    return out.str();
}

/* a/b/c/d -> a/b/c
 * a/b/c/d/ -> a/b/c
 */
static string dirname(string path){
    vector<string> paths = splitPath(path);
    if (paths.size() > 1){
        paths.pop_back();
        return joinPath(paths);
    } else if (paths.size() == 1){
        if (paths[0] == "/"){
            return "/";
        }
        return ".";
    } else {
        return ".";
    }
    /*
    while (path.size() > 0 && path[path.size() - 1] == '/'){
        path.erase(path.size() - 1);
    }

    if (path.find("/") != string::npos ||
        path.find("\\") != string::npos){
        size_t rem = path.find_last_of("/");
        if (rem != string::npos){
            return path.substr(0, rem + 1);
        }

        rem = path.find_last_of("\\");
        if (rem != string::npos){
            return path.substr(0, rem + 1);
        }
    }

    return "";
    */
}

RelativePath::RelativePath(const RelativePath & path):
Path(path){
}
        
RelativePath RelativePath::removeFirstDirectory() const {
    return RelativePath(stripFirstDir(path()));
}
        
bool RelativePath::isFile() const {
    vector<string> paths = splitPath(path());
    return paths.size() == 1;
}
        
RelativePath RelativePath::firstDirectory() const {
    vector<string> paths = splitPath(path());
    if (paths.size() > 1){
        return RelativePath(paths[0]);
    }
    return RelativePath();
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
        
bool RelativePath::operator!=(const RelativePath & path) const {
    return !(*this == path);
}

RelativePath RelativePath::join(const RelativePath & him) const {
    return RelativePath(path() + "/" + him.path());
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
        
bool AbsolutePath::operator!=(const AbsolutePath & path) const {
    return !(*this == path);
}
        
bool AbsolutePath::operator<(const AbsolutePath & path) const {
    return this->path() < path.path();
}
        
RelativePath AbsolutePath::remove(const AbsolutePath & what) const {
    string real = path();
    real.erase(0, what.path().size());
    while (real.find("/") == 0){
        real.erase(0, 1);
    }
    return RelativePath(real);
}
        
AbsolutePath AbsolutePath::getDirectory() const {
    return AbsolutePath(dirname(path()));
}

AbsolutePath AbsolutePath::getFilename() const {
    return AbsolutePath(stripDir(path()));
}
        
AbsolutePath AbsolutePath::join(const RelativePath & path) const {
    return AbsolutePath(this->path() + "/" + path.path());
}
        
InsensitivePath::InsensitivePath(const Path & what):
Path(what){
}

bool InsensitivePath::operator==(const Path & path) const {
    return Util::upperCaseAll(this->path()) == Util::upperCaseAll(path.path());
}

std::string removeExtension(const std::string & str){
    if (str.find(".") != std::string::npos){
        return str.substr(0, str.find_last_of("."));
    }
    return str;
}

/* a/b/c/d -> d */
std::string stripDir(const std::string & str){
    if (str.find("/") != std::string::npos || str.find("\\") != std::string::npos){
        std::string temp = str;
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

/* a/b/c/d -> a/b/c/ */
std::string stripFilename(const std::string & str){
    std::string temp = str;
    if( str.find( "/") != std::string::npos || str.find( "\\") != std::string::npos ){
        size_t rem = temp.find_last_of( "/" );
        if( rem != std::string::npos ){
            return str.substr(0,rem+1);
        }
        rem = temp.find_last_of( "\\" );
        if( rem != std::string::npos ){
            return str.substr(0,rem+1);
        }
    }
    return "";
}

}

namespace Storage{
        
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
Exception(where, line, file){
}

NotFound::NotFound(const std::string & where, int line, const Exc::Base & nested, const std::string & file):
Exception(where, line, nested, file){
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

System::System(){
}

System::~System(){
}

vector<Filesystem::AbsolutePath> System::getFiles(const Filesystem::AbsolutePath & dataPath, const Filesystem::RelativePath & find, bool caseInsensitive){
    if (find.isFile()){
        return getFiles(dataPath, find.path(), caseInsensitive);
    }

    /* split the path into its consituent parts
     * a/b/c -> a/b and c
     * search for a/b, then search for c in the results
     */
    Filesystem::RelativePath directory = find.getDirectory();
    Filesystem::RelativePath file = find.getFilename();

    vector<Filesystem::AbsolutePath> more = getFiles(dataPath, directory, caseInsensitive);
    vector<Filesystem::AbsolutePath> out;
    for (vector<Filesystem::AbsolutePath>::iterator it = more.begin(); it != more.end(); it++){
        Filesystem::AbsolutePath path = *it;
        /* if its not a directory then we can't keep searching */
        if (::System::isDirectory(path.path())){
            vector<Filesystem::AbsolutePath> findMore = getFiles(path, file, caseInsensitive);
            out.insert(out.end(), findMore.begin(), findMore.end());
        }
    }
    return out;
}

Util::ReferenceCount<System> self;
System & instance(){
    if (self != NULL){
        return *self;
    }
    self = new Filesystem(Util::getDataPath2());
    return *self;
}
        
File::File(){
}
        
File::~File(){
}
        
NormalFile::NormalFile(const Path::AbsolutePath & path, Access mode):
path(path){
    ios_base::openmode iosMode = fstream::in;
    switch (mode){
        case Read: iosMode = fstream::in; break;
        case Write: iosMode = fstream::out; break;
        case ReadWrite: iosMode = fstream::in | fstream::out; break;
    }

    in.open(path.path().c_str(), iosMode | fstream::binary);
    in >> noskipws;
}
        
bool NormalFile::eof(){
    return in.eof();
}

int NormalFile::getSize(){
    streampos here = in.tellg();
    in.seekg(0, ios::end);
    int length = in.tellg();
    in.seekg(here, ios::beg);
    return length;
}

bool NormalFile::good(){
    return in.good();
}

File & NormalFile::operator>>(unsigned char & c){
    in >> c;
    return *this;
}

int NormalFile::readLine(char * output, int size){
    in.read(output, size);
    return in.gcount();
}

NormalFile::~NormalFile(){
    in.close();
}


StringFile::StringFile(const std::string & start):
data(start),
stream(start){
    stream >> noskipws;
}

int StringFile::readLine(char * output, int size){
    stream.read(output, size);
    return stream.gcount();
}

int StringFile::getSize(){
    return data.size();
}

bool StringFile::eof(){
    return stream.eof();
}

bool StringFile::good(){
    return stream.good();
}

File & StringFile::operator>>(unsigned char & c){
    stream >> c;
    return *this;
}

StringFile::~StringFile(){
}

/* overlays:
 * add x/y.zip
 * y.zip contains
 *   example.txt
 * try to read x/example.txt, goto y.zip
 * or y.zip could contain y/
 * 
 * z.zip contains
 *   example.txt
 *
 * how can you prevent two zip files from providing the same files?
 */
class ZipContainer{
public:
    ZipContainer(const string & path, const Filesystem::AbsolutePath & start):
    path(path),
    start(start){
        zipFile = unzOpen(path.c_str());
        if (zipFile == NULL){
            throw Exception(__FILE__, __LINE__, "Could not open zip file");
        }

        if (unzGoToFirstFile(zipFile) != UNZ_OK){
            throw Exception(__FILE__, __LINE__, "Could not get to first file");
        }

        do{
            char filename[1024];
            unz_file_info fileInfo;
            unzGetCurrentFileInfo(zipFile, &fileInfo, filename, sizeof(filename), NULL, 0, NULL, 0);
            files.push_back(string(filename));

        } while (unzGoToNextFile(zipFile) != UNZ_END_OF_LIST_OF_FILE);

        /*
        zlib_filefunc64_32_def functions;
        functions.open_file_func = __real_open;
        functions.tell_file_func = __real_lseek;
        functions.seek_file_func = __real_lseek;
        */
        /*
        unzFile zip = unzOpen(path.c_str());
        unz_global_info info;
        if (unzGetGlobalInfo(zip, &info) == UNZ_OK){
            Global::debug(0) << "Entries: " << info.number_entry << std::endl;
        }

        if (unzGoToFirstFile(zip) != UNZ_OK){
            throw Exception(__FILE__, __LINE__, "Could not get to first file");
        }
        do{
            char filename[1024];
            unz_file_info fileInfo;
            unzGetCurrentFileInfo(zip, &fileInfo, filename, sizeof(filename), NULL, 0, NULL, 0);
            Global::debug(0) << "Got file " << filename << std::endl;
            Global::debug(0) << " Compressed " << fileInfo.compressed_size << " uncompressed " << fileInfo.uncompressed_size << std::endl;

        } while (unzGoToNextFile(zip) != UNZ_END_OF_LIST_OF_FILE);
        unzClose(zip);
        */
    }

    ~ZipContainer(){
        if (zipFile != NULL){
            unzClose(zipFile);
        }
    }

    void findFile(const Path::AbsolutePath & file){
        Path::RelativePath find(file.remove(start));
        if (unzLocateFile(zipFile, find.path().c_str(), 2) != UNZ_OK){
            Global::debug(0) << "Could not find " << find.path() << std::endl;
        } else {
            Global::debug(0) << "Found " << find.path() << " in zip file " << path << std::endl;
        }
    }

    int currentFileSize(){
        char filename[1024];
        unz_file_info fileInfo;
        unzGetCurrentFileInfo(zipFile, &fileInfo, filename, sizeof(filename), NULL, 0, NULL, 0);
        return fileInfo.uncompressed_size;
    }

    int read(char * buffer, int size){
        int got = unzReadCurrentFile(zipFile, buffer, size);
        if (got <= 0){
            throw Exception(__FILE__, __LINE__, "Could not read bytes from zip");
        }
        return got;
    }

    void open(const Path::AbsolutePath & file){
        findFile(file);
        if (unzOpenCurrentFile(zipFile) != UNZ_OK){
            std::ostringstream out;
            out << "Could not open zip file entry " << file.path();
            throw Exception(__FILE__, __LINE__, out.str());
        }
    }

    void close(){
        unzCloseCurrentFile(zipFile);
    }

    vector<string> getFiles() const {
        return files;
    }

protected:
    /* Path to the zip file itself */
    const string path;
    /* Where overlay starts */
    const Path::AbsolutePath start;

    unzFile zipFile;
    vector<string> files;
};


ZipFile::ZipFile(const Path::AbsolutePath & path, const Util::ReferenceCount<ZipContainer> & zip):
path(path),
zip(zip),
atEof(false){
    zip->open(path);
}

ZipFile::~ZipFile(){
    zip->close();
}
        
bool ZipFile::eof(){
    return atEof;
}

bool ZipFile::good(){
    /* FIXME */
    return true;
}

File & ZipFile::operator>>(unsigned char & c){
    readLine((char*) &c, 1);
    return *this;
}
        
int ZipFile::getSize(){
    return zip->currentFileSize();
}
        
int ZipFile::readLine(char * output, int size){
    try{
        return zip->read(output, size);
    } catch (const Exception & nomore){
        atEof = true;
        return 0;
    }
}

void System::overlayFile(const AbsolutePath & where, Util::ReferenceCount<ZipContainer> zip){
    overlays[where] = zip;
}

void System::unoverlayFile(const AbsolutePath & where){
    overlays.erase(where);
}

void System::addOverlay(const AbsolutePath & container, const AbsolutePath & where){
    Global::debug(1) << "Opening zip file " << container.path() << std::endl;
    Util::ReferenceCount<ZipContainer> zip(new ZipContainer(container.path(), where));
    vector<string> files = zip->getFiles();
    for (vector<string>::const_iterator it = files.begin(); it != files.end(); it++){
        string path = *it;
        Global::debug(0) << "Add overlay to " << where.join(Filesystem::RelativePath(path)).path() << std::endl;
        overlayFile(where.join(Filesystem::RelativePath(path)), zip);
    }
}

void System::removeOverlay(const AbsolutePath & container, const AbsolutePath & where){
    Util::ReferenceCount<ZipContainer> zip(new ZipContainer(container.path(), where));
    vector<string> files = zip->getFiles();
    for (vector<string>::const_iterator it = files.begin(); it != files.end(); it++){
        string path = *it;
        Global::debug(0) << "Add overlay to " << where.join(Filesystem::RelativePath(path)).path() << std::endl;
        unoverlayFile(where.join(Filesystem::RelativePath(path)));
    }
}

Util::ReferenceCount<File> System::open(const AbsolutePath & path, File::Access mode){
    if (overlays[path] != NULL){
        return Util::ReferenceCount<File>(new ZipFile(path, overlays[path]));
    } else {
        return Util::ReferenceCount<File>(new NormalFile(path, mode));
    }
}

bool hasInstance(){
    // return true;
    return self != NULL;
}

System & setInstance(const Util::ReferenceCount<System> & what){
    self = what;
    return *self;
}

/* will read upto 'length' bytes unless a null byte is seen first */
string EndianReader::readStringX(int length){
    ostringstream out;
    uint8_t letter = readByte1();
    while (letter != 0 && length > 0){
        out << letter;
        letter = readByte1();
        length -= 1;
    }
    return out.str();
}

/* unconditionally reads 'length' bytes */
std::string EndianReader::readString2(int length){
    ostringstream out;
    vector<uint8_t> bytes = readBytes(length);
    for (vector<uint8_t>::iterator it = bytes.begin(); it != bytes.end(); it++){
        char byte = *it;
        if (byte == 0){
            break;
        }
        out << *it;
    }
    return out.str();
}
            
void EndianReader::seekEnd(streamoff where){
    stream.seekg(where, ios::end);
}

void EndianReader::seek(streampos where){
    stream.seekg(where);
}

int EndianReader::position(){
    return stream.tellg();
}
            
void EndianReader::readBytes(uint8_t * out, int length){
    stream.read((char*) out, length);
}

vector<uint8_t> EndianReader::readBytes(int length){
    vector<uint8_t> bytes;
    for (int i = 0; i < length; i++){
        uint8_t byte = 0;
        stream.read((char*) &byte, 1);
        if (stream.eof()){
            throw Eof();
        } else {
        }
        bytes.push_back(byte);
    }
    return bytes;
}

}

Filesystem::Filesystem(const AbsolutePath & path):
dataPath(path){
}

#ifdef _WIN32
Filesystem::AbsolutePath Filesystem::userDirectory(){
    ostringstream str;
    char path[MAX_PATH];
    SHGetSpecialFolderPathA(0, path, CSIDL_APPDATA, false);
    str << path << "/paintown/";
    return Filesystem::AbsolutePath(str.str());
}

Filesystem::AbsolutePath Filesystem::configFile(){
    ostringstream str;
    char path[MAX_PATH];
    SHGetSpecialFolderPathA(0, path, CSIDL_APPDATA, false);
    str << path << "/paintown_configuration.txt";
    return Filesystem::AbsolutePath(str.str());
}
#else
Filesystem::AbsolutePath Filesystem::configFile(){
    ostringstream str;
    /* what if HOME isn't set? */
    str << getenv("HOME") << "/.paintownrc";
    return Filesystem::AbsolutePath(str.str());
}

Filesystem::AbsolutePath Filesystem::userDirectory(){
    ostringstream str;
    char * home = getenv("HOME");
    if (home == NULL){
        str << "/tmp/paintown";
    } else {
        str << home << "/.paintown/";
    }
    return Filesystem::AbsolutePath(str.str());
}
#endif

Filesystem::AbsolutePath Filesystem::lookup(const RelativePath path){
    vector<Filesystem::AbsolutePath> places;

#define push(x) try{ places.push_back(x); } catch (const Storage::IllegalPath & fail){ }
    push(dataPath.join(path));
    push(userDirectory().join(path));
    push(Filesystem::AbsolutePath(path.path()));
#undef push

    /* start error stuff early */
    ostringstream out;
    out << "Cannot find " << path.path() << ". I looked in ";
    bool first = true;
    for (vector<Filesystem::AbsolutePath>::iterator it = places.begin(); it != places.end(); it++){
        Filesystem::AbsolutePath & final = *it;
        if (overlays[final] != NULL || ::System::readable(final.path())){
            return final;
        }
        if (!first){
            out << ", ";
        } else {
            first = false;
        }
        out << "'" << final.path() << "'";
    }

    // out << "Cannot find " << path.path() << ". I looked in '" << dataPath.join(path).path() << "', '" << userDirectory().join(path).path() << "', and '" << path.path() << "'";

    throw NotFound(__FILE__, __LINE__, out.str());


#if 0
    /* first try the main data directory */
    Filesystem::AbsolutePath final = dataPath.join(path);
    if (::System::readable(final.path())){
        return final;
    }
    /* then try the user directory, like ~/.paintown */
    final = userDirectory().join(path);
    if (::System::readable(final.path())){
        return final;
    }
    /* then just look in the cwd */
    if (::System::readable(path.path())){
        return Filesystem::AbsolutePath(path.path());
    }

    ostringstream out;
    out << "Cannot find " << path.path() << ". I looked in '" << dataPath.join(path).path() << "', '" << userDirectory().join(path).path() << "', and '" << path.path() << "'";

    throw NotFound(__FILE__, __LINE__, out.str());
#endif
}

Filesystem::AbsolutePath Filesystem::lookupInsensitive(const Filesystem::AbsolutePath & directory, const Filesystem::RelativePath & path){
    if (path.path() == ""){
        throw NotFound(__FILE__, __LINE__, "Given empty path to lookup");
    }
    if (path.path() == "."){
        return directory;
    }
    if (path.path() == ".."){
        return directory.getDirectory();
    }
    if (path.isFile()){
        vector<AbsolutePath> all = getFiles(directory, "*", true);
        for (vector<AbsolutePath>::iterator it = all.begin(); it != all.end(); it++){
            AbsolutePath & check = *it;
            if (InsensitivePath(check.getFilename()) == path){
                return check;
            }
        }

        ostringstream out;
        out << "Cannot find " << path.path() << " in " << directory.path();
        throw NotFound(__FILE__, __LINE__, out.str());
    } else {
        return lookupInsensitive(lookupInsensitive(directory, path.firstDirectory()), path.removeFirstDirectory());
    }
}

vector<Filesystem::AbsolutePath> Filesystem::findDirectoriesIn(const Filesystem::AbsolutePath & path){
    vector<Filesystem::AbsolutePath> dirs;
    DIR * dir = opendir(path.path().c_str());
    if (dir == NULL){
        return dirs;
    }

    struct dirent * entry = readdir(dir);
    while (entry != NULL){
        if (string(entry->d_name) != "." && string(entry->d_name) != ".."){
            string total = path.path() + "/" + entry->d_name;
            if (::System::isDirectory(total)){
                dirs.push_back(AbsolutePath(total));
            }
        }
        entry = readdir(dir);
    }

    closedir(dir);

    return dirs;
}

vector<Filesystem::AbsolutePath> Filesystem::findDirectories(const RelativePath & path){
    typedef vector<AbsolutePath> Paths;
    Paths dirs;

    Paths main_dirs = findDirectoriesIn(dataPath.join(path));
    Paths user_dirs = findDirectoriesIn(userDirectory().join(path));
    Paths here_dirs = findDirectoriesIn(Filesystem::AbsolutePath(path.path()));

    dirs.insert(dirs.end(), main_dirs.begin(), main_dirs.end());
    dirs.insert(dirs.end(), user_dirs.begin(), user_dirs.end());
    dirs.insert(dirs.end(), here_dirs.begin(), here_dirs.end());

    return dirs;
}

/* a/b/c/ -> a/b/c */
static string removeTrailingSlash(string str){
    while (str.size() > 0 && str[str.size() - 1] == '/'){
        str = str.erase(str.size() - 1);
    }
    return str;
}

vector<Filesystem::AbsolutePath> Filesystem::getFiles(const AbsolutePath & dataPath, const string & find, bool caseInsensitive){
#ifdef USE_ALLEGRO
    struct al_ffblk info;
    vector<AbsolutePath> files;

    if (al_findfirst((dataPath.path() + "/" + find).c_str(), &info, FA_ALL ) != 0){
        return files;
    }
    files.push_back(AbsolutePath(dataPath.path() + "/" + string(info.name)));
    while ( al_findnext( &info ) == 0 ){
        files.push_back(AbsolutePath(dataPath.path() + "/" + string(info.name)));
    }
    al_findclose( &info );
    return files;
#else
    Util::Thread::ScopedLock scoped(lock);
    vector<AbsolutePath> files;
    DIRST sflEntry;
    // bool ok = open_dir(&sflEntry, removeTrailingSlash(dataPath.path()).c_str());
    bool ok = open_dir(&sflEntry, dataPath.path().c_str());
    if (!ok){
        close_dir(&sflEntry);
        return files;
    }
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
    
vector<Filesystem::AbsolutePath> Filesystem::getAllDirectories(const AbsolutePath & path){
    vector<AbsolutePath> all = findDirectoriesIn(path);
    vector<AbsolutePath> final;
    append(final, all);
    for (vector<AbsolutePath>::iterator it = all.begin(); it != all.end(); it++){
        vector<AbsolutePath> more = getAllDirectories(*it);
        append(final, more);
    }
    return final;
}

vector<Filesystem::AbsolutePath> Filesystem::getFilesRecursive(const AbsolutePath & dataPath, const string & find, bool caseInsensitive){
    if (!exists(dataPath)){
        return vector<AbsolutePath>();
    }
    vector<AbsolutePath> directories = getAllDirectories(dataPath);
    directories.push_back(dataPath);
    vector<AbsolutePath> files;
    for (vector<AbsolutePath>::iterator it = directories.begin(); it != directories.end(); it++){
        vector<AbsolutePath> found = getFiles(*it, find, caseInsensitive);
        append(files, found);
    }
    return files;
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

Filesystem::AbsolutePath Filesystem::find(const RelativePath & path){
    if (path.isEmpty()){
        throw NotFound(__FILE__, __LINE__, "No path given");
    }

    AbsolutePath out = lookup(path);
    if (::System::isDirectory(out.path())){
        return AbsolutePath(out.path() + "/");
    }
    return AbsolutePath(out.path());
}
    
Filesystem::AbsolutePath Filesystem::findInsensitive(const RelativePath & path){
    try{
        /* try sensitive lookup first */
        return lookup(path);
    } catch (const NotFound & fail){
    }
    /* get the base directory */
    AbsolutePath directory = lookup(path.getDirectory());
    return lookupInsensitive(directory, path.getFilename());
}
    
bool Filesystem::exists(const RelativePath & path){
    try{
        AbsolutePath absolute = find(path);
        return true;
    } catch (const NotFound & found){
        return false;
    }
}

bool Filesystem::exists(const AbsolutePath & path){
    return ::System::readable(path.path());
}

Filesystem::RelativePath Filesystem::cleanse(const AbsolutePath & path){
    string str = path.path();
    if (str.find(dataPath.path()) == 0){
        str.erase(0, dataPath.path().length());
    } else if (str.find(userDirectory().path()) == 0){
        str.erase(0, userDirectory().path().length());
    }
    return RelativePath(str);
}
