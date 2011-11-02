#ifdef NACL

/* documentation for ppapi
 * http://code.google.com/chrome/nativeclient/docs/reference/peppercpp/inherits.html
 */

/* issues with getting data
 * 1. the function that starts the game is called from the main chrome thread
 * which starts from a javascript call to module.PostMessage('run').
 * ...
 * 
 */

#include <unistd.h>
#include <errno.h>
#include "network-system.h"
#include <sstream>
#include <fstream>
#include "../funcs.h"
#include "../debug.h"
#include "../sfl/sfl.h"
#include <ppapi/c/pp_errors.h>
#include <ppapi/cpp/url_loader.h>
#include <ppapi/cpp/url_request_info.h>
#include <ppapi/cpp/url_response_info.h>
#include <ppapi/c/ppb_url_request_info.h>
#include <ppapi/cpp/completion_callback.h>

using std::string;
using std::map;
using std::vector;
using std::ostringstream;
using std::ifstream;

namespace Nacl{

static const char * CONTEXT = "nacl";

typedef Path::AbsolutePath AbsolutePath;
typedef Path::RelativePath RelativePath;

enum RequestType{
    Exists
};

struct Request{
    RequestType type;
    AbsolutePath absolute;
    RelativePath relative;
    bool complete;
    bool success;
};

Request operation;

struct NaclRequest{
    virtual ~NaclRequest(){
    }

    virtual void start() = 0;
};

struct NaclRequestOpen: public NaclRequest {
    NaclRequestOpen(pp::Instance * instance, const string & url, Manager * manager):
        request(instance),
        loader(instance),
        url(url),
        manager(manager){
            request.SetURL(url);
            request.SetMethod("GET");
            // request.SetProperty(PP_URLREQUESTPROPERTY_RECORDDOWNLOADPROGRESS, pp::Var((bool) PP_TRUE));
        }

    void start(){
        Global::debug(2) << "Request open for url " << url << std::endl;
        pp::CompletionCallback callback(&NaclRequestOpen::onFinish, this);
        int32_t ok = loader.Open(request, callback);
        Global::debug(2) << "Open " << ok << std::endl;
        if (ok != PP_OK_COMPLETIONPENDING){
            // Global::debug(0) << "Call on main thread" << std::endl;
            // core->CallOnMainThread(0, callback, ok);
            callback.Run(ok);
        }
        // Global::debug(1) << "Callback running" << std::endl;
    }

    static void onFinish(void * me, int32_t result){
        NaclRequestOpen * self = (NaclRequestOpen*) me;
        self->finish(result);
    }

    void finish(int32_t result);

    pp::URLRequestInfo request;
    pp::URLLoader loader;
    string url;
    Manager * manager;
};

struct NaclRequestExists: public NaclRequest {
    NaclRequestExists(pp::Instance * instance, const string & url, Manager * manager):
        request(instance),
        loader(instance),
        url(url),
        manager(manager){
            request.SetURL(url);
            request.SetMethod("GET");
        }

    void start(){
        pp::CompletionCallback callback(&NaclRequestExists::onFinish, this);
        int32_t ok = loader.Open(request, callback);
        if (ok != PP_OK_COMPLETIONPENDING){
            callback.Run(ok);
        }
    }

    static void onFinish(void * me, int32_t result){
        NaclRequestExists * self = (NaclRequestExists*) me;
        self->finish(result);
    }

    void finish(int32_t result);

    pp::URLRequestInfo request;
    pp::URLLoader loader;
    string url;
    Manager * manager;
};

/* responsible for reading the data from the nacl pepper input stream
 */
class FileHandle{
public:
    FileHandle():
    buffer(NULL){
    }

    ~FileHandle(){
        delete[] buffer;
    }

    class Reader{
    public:
        static const int PAGE_SIZE = 1024 * 32;
        struct Page{
            Page():
            buffer(NULL),
            size(0),
            next(NULL){
                buffer = new char[PAGE_SIZE];
            }

            char * buffer;
            int size;
            Page * next;

            ~Page(){
                delete[] buffer;
                delete next;
            }
        };

        Reader(pp::CompletionCallback finish, pp::URLLoader & loader, FileHandle * handle, pp::Core * core):
        finish(finish),
        loader(loader),
        handle(handle),
        core(core),
        tries(0){
            current = &page;
        }

        int getSize(){
            Page * use = &page;
            int total = 0;
            while (use != NULL){
                total += use->size;
                use = use->next;
            }
            return total;
        }

        void copy(char * buffer){
            Page * use = &page;
            while (use != NULL){
                memcpy(buffer, use->buffer, use->size);
                buffer += use->size;
                use = use->next;
            }
        }

        void read(){
            pp::CompletionCallback callback(&Reader::onRead, this);
            if (current->size == PAGE_SIZE){
                Page * next = new Page();
                current->next = next;
                current = next;
            }
            int32_t ok = loader.ReadResponseBody(current->buffer + current->size, PAGE_SIZE - current->size, callback);
            if (ok != PP_OK_COMPLETIONPENDING){
                callback.Run(ok);
            }
        }

        static void onRead(void * self, int32_t result){
            Reader * reader = (Reader*) self;
            reader->didRead(result);
        }

        void didRead(int32_t result){
            Global::debug(2) << "Read " << result << " bytes" << std::endl;
            current->size += result;
            if (result > 0){
                tries = 0;
                read();
            } else {
                if (tries >= 3){
                    handle->readDone(this);
                } else {
                    tries += 1;
                    pp::CompletionCallback callback(&Reader::doRead, this);
                    core->CallOnMainThread((tries - 1) * 25, callback, 0);
                }
            }
        }

        static void doRead(void * self, int32_t result){
            Reader * reader = (Reader*) self;
            reader->read();
        }

        pp::CompletionCallback finish;
        pp::URLLoader & loader;
        Page page;
        Page * current;
        FileHandle * handle;
        pp::Core * core;
        int tries;
    };

    void readAll(pp::CompletionCallback finish, pp::Core * core, pp::URLLoader & loader){
        reader = new Reader(finish, loader, this, core);
        reader->read();
    }

    void readDone(Reader * reader){
        length = reader->getSize();
        Global::debug(2) << "Done reading, got " << length << " bytes" << std::endl;
        buffer = new char[length];
        reader->copy(buffer);
        reader->finish.Run(0);
    }

    int read(void * buffer, off_t position, size_t count){
        size_t bytes = position + count < length ? count : (length - position);
        memcpy(buffer, this->buffer + position, bytes);
        return bytes;
    }

    off_t getLength() const {
        return length;
    }

    off_t length;
    char * buffer;
    Util::ReferenceCount<Reader> reader;
};

/* acts similar to an stdio FILE object in that this stores the current
 * position in the buffer.
 */
class File{
public:
    File(const Util::ReferenceCount<FileHandle> & handle):
    position(0),
    handle(handle){
    }

    int read(void * buffer, size_t count){
        int got = handle->read(buffer, position, count);
        position += got;
        return got;
    }

    off_t seek(off_t offset, int whence){
        switch (whence){
            case SEEK_SET: {
                position = offset;
                break;
            }
            case SEEK_CUR: {
                position += offset;
                break;
            }
            case SEEK_END: {
                position = handle->getLength() - offset;
                break;
            }
        }

        return position;
    }

protected:
    off_t position;
    Util::ReferenceCount<FileHandle> handle;
};

class Manager{
public:
    Manager(pp::Instance * instance, pp::Core * core):
    instance(instance),
    core(core),
    factory(this){
    }

    pp::Instance * instance;
    pp::Core * core;
    Util::ReferenceCount<NaclRequest> request;
    pp::CompletionCallbackFactory<Manager> factory;

    struct OpenFileData{
        const char * path;
        Util::ReferenceCount<FileHandle> file;
    };

    struct ExistsData{
        const char * path;
        bool exists;
    };

    OpenFileData openFileData;
    ExistsData existsData;

    Util::Thread::LockObject lock;
    volatile bool done;

    Util::ReferenceCount<FileHandle> openFile(const char * path){
        Global::debug(1, CONTEXT) << "open " << path << std::endl;
        Util::Thread::ScopedLock scoped(lock);
        done = false;
        openFileData.path = path;
        pp::CompletionCallback callback(&Manager::doOpenFile, this);
        core->CallOnMainThread(0, callback, 0);
        lock.wait(done);
        return openFileData.file;
    }

    bool exists(const string & path){
        Global::debug(1, CONTEXT) << "exists " << path << std::endl;
        Util::Thread::ScopedLock scoped(lock);
        done = false;
        existsData.exists = false;
        existsData.path = path.c_str();
        pp::CompletionCallback callback(&Manager::doExists, this);
        core->CallOnMainThread(0, callback, 0);
        lock.wait(done);
        return existsData.exists;
    }

    static void doExists(void * self, int32_t result){
        Manager * manager = (Manager*) self;
        manager->continueExists();
    }

    void continueExists(){
        request = new NaclRequestExists(instance, existsData.path, this);
        request->start();
    }

    void success(NaclRequestExists & exists){
        existsData.exists = true;
        requestComplete();
    }

    void failure(NaclRequestExists & exists){
        existsData.exists = false;
        requestComplete();
    }

    /* called by the main thread */
    static void doOpenFile(void * self, int32_t result){
        Manager * manager = (Manager*) self;
        manager->continueOpenFile();
    }

    void continueOpenFile(){
        request = new NaclRequestOpen(instance, openFileData.path, this);
        request->start();
    }

    void requestComplete(){
        /* destroy request on the main thread */
        request = NULL;
        lock.lockAndSignal(done, true);
    }

    void success(NaclRequestOpen & open){
        pp::URLResponseInfo info = open.loader.GetResponseInfo();
        if (info.GetStatusCode() == 200){
            /*
            int64_t received = 0;
            int64_t total = 0;
            if (open.loader.GetDownloadProgress(&received, &total)){
                Global::debug(0) << "Downloaded " << received << " total " << total << std::endl;
            }
            */
            Util::ReferenceCount<FileHandle> handle(new FileHandle());
            pp::CompletionCallback callback(&Manager::completeRead, this);
            openFileData.file = handle;
            handle->readAll(callback, core, open.loader);
            /*
            openFileData.file = nextFileDescriptor();
            fileTable[openFileData.file] = new FileHandle(request.convert<NaclRequestOpen>());
            readEntireFile(fileTable[openFileData.file]);
            */
        } else {
            Global::debug(1) << "Could not open file" << std::endl;
            requestComplete();
        }
    }

    static void completeRead(void * self, int32_t result){
        Manager * manager = (Manager*) self;
        manager->requestComplete();
    }

    void failure(NaclRequestOpen & open){
        requestComplete();
    }
};

void NaclRequestOpen::finish(int32_t result){
    if (result == 0){
        manager->success(*this);
    } else {
        manager->failure(*this);
    }
}

void NaclRequestExists::finish(int32_t result){
    if (result == 0){
        pp::URLResponseInfo info = loader.GetResponseInfo();
        if (info.GetStatusCode() == 200){
            manager->success(*this);
        } else {
            manager->failure(*this);
        }
    } else {
        manager->failure(*this);
    }
}

static void testMatchFile();

NetworkSystem::NetworkSystem(pp::Instance * instance, pp::Core * core):
instance(instance),
core(core){
    // testMatchFile();
}

NetworkSystem::~NetworkSystem(){
}

AbsolutePath NetworkSystem::find(const RelativePath & path){
    AbsolutePath all = Util::getDataPath2().join(path);
    if (exists(all)){
        return all;
    }
    throw Storage::NotFound(__FILE__, __LINE__, path.path());
}

RelativePath NetworkSystem::cleanse(const AbsolutePath & path){
    string str = path.path();
    if (str.find(Util::getDataPath2().path()) == 0){
        str.erase(0, Util::getDataPath2().path().length());
    } else if (str.find(userDirectory().path()) == 0){
        str.erase(0, userDirectory().path().length());
    }
    return RelativePath(str);
}

bool NetworkSystem::exists(const RelativePath & path){
    try{
        AbsolutePath absolute = find(path);
        return true;
    } catch (const Storage::NotFound & found){
        return false;
    }
}

bool NetworkSystem::exists(const AbsolutePath & path){
    Util::Thread::ScopedLock scoped(lock);
    if (existsCache.find(path) != existsCache.end()){
        return existsCache[path];
    }
    Manager manager(instance, core);
    bool what = manager.exists(path.path());
    existsCache[path] = what;
    return what;
}

string NetworkSystem::readFileAsString(const AbsolutePath & path){
    if (!exists(path)){
        ostringstream fail;
        fail << "Could not read " << path.path();
        throw Filesystem::NotFound(__FILE__, __LINE__, fail.str());
    }
    ostringstream buffer;
    ifstream input(path.path().c_str());
    char stuff[1024];
    while (input.good()){
        input.read(stuff, sizeof(stuff) - 1);
        int in = input.gcount();
        stuff[in] = '\0';
        buffer << stuff;
    }
    return buffer.str();
}

static vector<string> split(string input, char splitter){
    vector<string> all;
    size_t found = input.find(splitter);
    while (found != string::npos){
        all.push_back(input.substr(0, found));
        input.erase(0, found + 1);
        found = input.find(splitter);
    }
    if (input.size() != 0){
        all.push_back(input);
    }
    return all;
}

vector<AbsolutePath> NetworkSystem::readDirectory(const AbsolutePath & dataPath){
    /* assume existence of 'directory.txt' in the given directory */
    AbsolutePath fullPath = dataPath.join(RelativePath("directory.txt"));
    string all = readFileAsString(fullPath);
    vector<string> files = split(all, '\n');
    vector<AbsolutePath> paths;
    for (vector<string>::iterator it = files.begin(); it != files.end(); it++){
        string what = *it;
        if (what != ""){
            paths.push_back(dataPath.join(RelativePath(what)));
        }
    }
    return paths;
}

static bool matchFile(const AbsolutePath & path, const string & find, bool insensitive = false){
    return false;
    /*
    string file = path.path();
    for (unsigned int index = 0; index < find.size(); index += 1){
        if (index >= file.size()){
            return false;
        }
        char find_letter = find[find.size() - index - 1];
        char file_letter = file[file.size() - index - 1];
        if (insensitive){
            if (tolower(find_letter) != tolower(file_letter)){
                return false;
            }
        } else {
            if (find_letter != file_letter){
                return false;
            }
        }
    }

    return true;
    */
}

/* check if 'foo/bar/baz.txt' matches *.txt */
static bool matchFile2(const AbsolutePath & path, const string & find, bool insensitive = false){
    string file = path.getFilename().path();
    Global::debug(0) << "Find " << find << " in " << path.path() << " insensitive? " << insensitive << std::endl;
    unsigned int index = 0;
    while (index < file.size()){
        if (index >= find.size()){
            return false;
        }
        if (find[index] == '*'){
            return true;
        }
        if (insensitive){
            if (tolower(find[index]) != tolower(file[index])){
                return false;
            }
        } else {
            if (find[index] != file[index]){
                return false;
            }
        }
        index += 1;
    }
    return true;
}

static void testMatchFile(){
#define test_match(check, against, insensitive) if (!matchFile(Filesystem::AbsolutePath(check), against, insensitive)){ Global::debug(0) << "Test failed, expected a match: file " << check << " find " << against << std::endl; }
#define test_no_match(check, against, insensitive) if (matchFile(Filesystem::AbsolutePath(check), against, insensitive)){ Global::debug(0) << "Test failed, expected no match: file " << check << " find " << against << std::endl; }

    test_match("foo", "foo", false);
    test_match("fOo", "foO", true);
    test_match("bar/foo", "foo", false);
    test_match("bar/fOo", "foO", true);
    test_match("bar/baz/foo.txt", "baz/foo.txt", false);
    test_match("foobar.txt", "*.txt", false);
    test_match("bar/foobar.txt", "*.txt", false);
    test_no_match("fuz", "foo", false);
    test_no_match("foo/bar.txt", "foo", false);
    test_no_match("buz/foobar.txt", "bar/*.txt", false);

#undef test_match
#undef test_no_match
}

std::vector<AbsolutePath> NetworkSystem::getFiles(const AbsolutePath & dataPath, const std::string & find, bool caseInsensitive){
    vector<AbsolutePath> files = readDirectory(dataPath);
    vector<AbsolutePath> paths;
    for (vector<AbsolutePath>::iterator it = files.begin(); it != files.end(); it++){
        AbsolutePath check = *it;
        if (file_matches(check.getFilename().path().c_str(), find.c_str())){
            paths.push_back(check);
        }
    }
    return paths;
}

std::vector<AbsolutePath> NetworkSystem::getFilesRecursive(const AbsolutePath & dataPath, const std::string & find, bool caseInsensitive){
    vector<AbsolutePath> files = readDirectory(dataPath);
    vector<AbsolutePath> paths;
    for (vector<AbsolutePath>::iterator it = files.begin(); it != files.end(); it++){
        AbsolutePath check = *it;
        if (matchFile(check, find)){
            paths.push_back(check);
        }

        vector<AbsolutePath> more = getFilesRecursive(check, find, caseInsensitive);
        paths.insert(paths.end(), more.begin(), more.end());
    }
    return paths;
}

AbsolutePath NetworkSystem::configFile(){
    return AbsolutePath("paintownrc");
}

AbsolutePath NetworkSystem::userDirectory(){
    return AbsolutePath("paintown-user");
}

std::vector<AbsolutePath> NetworkSystem::findDirectories(const RelativePath & path){
    vector<AbsolutePath> files = readDirectory(find(path));
    vector<AbsolutePath> paths;
    for (vector<AbsolutePath>::iterator it = files.begin(); it != files.end(); it++){
        AbsolutePath check = *it;
        try{
            /* if we can read directory contents then its a directory */
            vector<AbsolutePath> more = readDirectory(check);
            paths.push_back(check);
        } catch (const Filesystem::NotFound & fail){
        }
    }
    return paths;

}

AbsolutePath NetworkSystem::findInsensitive(const RelativePath & path){
    try{
        /* try sensitive lookup first */
        return find(path);
    } catch (const Filesystem::NotFound & fail){
    }
    /* get the base directory */
    AbsolutePath directory = find(path.getDirectory());
    return lookupInsensitive(directory, path.getFilename());

}

/*
AbsolutePath NetworkSystem::lookupInsensitive(const AbsolutePath & directory, const RelativePath & path){
    vector<AbsolutePath> files = readDirectory(directory);
    vector<AbsolutePath> paths;
    for (vector<AbsolutePath>::iterator it = files.begin(); it != files.end(); it++){
        AbsolutePath check = *it;
        if (matchFile(check, path.path(), true)){
            return check;
        }
    }
    ostringstream out;
    out << "Cannot find " << path.path() << " in " << directory.path();
    throw Filesystem::NotFound(__FILE__, __LINE__, out.str());
}
*/

Filesystem::AbsolutePath NetworkSystem::lookupInsensitive(const Filesystem::AbsolutePath & directory, const Filesystem::RelativePath & path){
    if (path.path() == ""){
        throw Filesystem::NotFound(__FILE__, __LINE__, "Given empty path to lookup");
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
        throw Filesystem::NotFound(__FILE__, __LINE__, out.str());
    } else {
        return lookupInsensitive(lookupInsensitive(directory, path.firstDirectory()), path.removeFirstDirectory());
    }
}

int nextFileDescriptor(){
    static int next = 3;
    int n = next;
    next += 1;
    return n;
}

int NetworkSystem::libcOpen(const char * path, int mode, int params){
    Util::Thread::ScopedLock scoped(lock);
    if (files[path] == NULL){
        Manager manager(instance, core);
        Util::ReferenceCount<FileHandle> handle = manager.openFile(path);
        if (handle == NULL){
            return -1;
        }
        files[path] = handle;
    }

    int file = nextFileDescriptor();
    fileTable[file] = new File(files[path]);
    return file;
}
    
ssize_t NetworkSystem::libcRead(int fd, void * buffer, size_t count){
    Util::Thread::ScopedLock scoped(lock);
    if (fileTable.find(fd) == fileTable.end()){
        return EBADF;
    }
    Util::ReferenceCount<File> handle = fileTable[fd];
    if (handle == NULL){
        Global::debug(0) << "Handle is null!!" << std::endl;
    }
    return handle->read(buffer, count);
}

int NetworkSystem::libcClose(int fd){
    Util::Thread::ScopedLock scoped(lock);
    if (fileTable.find(fd) == fileTable.end()){
        return -1;
        /* set errno to EBADF */
    }
        
    fileTable.erase(fileTable.find(fd));
    return 0;
}
    
off_t NetworkSystem::libcLseek(int fd, off_t offset, int whence){
    Global::debug(2, CONTEXT) << "seek fd " << fd << " offset " << offset << " whence " << whence << std::endl;
    Util::Thread::ScopedLock scoped(lock);
    if (fileTable.find(fd) == fileTable.end()){
        return -1;
    }

    Util::ReferenceCount<File> handle = fileTable[fd];
    return handle->seek(offset, whence);
}

}

/* NOTE FIXME Missing I/O in Native Client */

Nacl::NetworkSystem & getSystem(){
    return (Nacl::NetworkSystem&) Storage::instance();
}

extern "C" {

/* http://sourceware.org/binutils/docs-2.21/ld/Options.html#index-g_t_002d_002dwrap_003d_0040var_007bsymbol_007d-261
 * --wrap=symbol
 * Use a wrapper function for symbol. Any undefined reference to symbol will be resolved to __wrap_symbol. Any undefined reference to __real_symbol will be resolved to symbol.
 */
int __wrap_open(const char * path, int mode, int params){
    return getSystem().libcOpen(path, mode, params);
}

ssize_t __wrap_read(int fd, void * buf, size_t count){
    return getSystem().libcRead(fd, buf, count);
}

extern int __real_close(int fd);

int __wrap_close(int fd){
    /* we may be given a file descriptor that we do not own, probably
     * because some file descriptors are really tied to sockets so
     * if we don't own the fd then pass it to the real close function.
     */
    int ok = getSystem().libcClose(fd);
    if (ok == -1){
        return __real_close(fd);
    }
    return ok;
}

off_t __wrap_lseek(int fd, off_t offset, int whence){
    return getSystem().libcLseek(fd, offset, whence);
}

int pipe (int filedes[2]){
    return -1;
}

int mkdir (const char *filename, mode_t mode){
    return -1;
}

int __wrap_access(const char *filename, int mode){
    Global::debug(0) << "Access for " << filename << std::endl;
    if (mode == R_OK){
        return getSystem().exists(Filesystem::AbsolutePath(filename));
    }
    return -1;
}

char * getcwd (char *buffer, size_t size){
    return NULL;
}

int __wrap_lstat (const char *path, struct stat *buf){
    Global::debug(0) << "Lstat for " << path << std::endl;
    return -1;
}

int rmdir (const char *filename){
    return -1;
}

int chdir (const char *filename){
    return -1;
}

int setuid (uid_t newuid){
    return 0;
}

int seteuid (uid_t uid){
    return 0;
}

uid_t geteuid (void){
    return NULL;
}

int setgid (gid_t gid){
    return 0;
}

gid_t getgid (void){
    return NULL;
}

int setegid (gid_t gid){
    return 0;
}

gid_t getegid (void){
    return NULL;
}

char * getlogin (void){
    return NULL;
}

uid_t getuid(void){
    return NULL;
}

struct passwd * getpwuid (uid_t uid){
    return NULL;
}

struct passwd * getpwnam (const char *name){
    return NULL;
}

struct group * getgrnam(const char *name){
    return NULL;
}

struct group * getgrgid(gid_t gid){
    return NULL;
}

int link (const char *oldname, const char *newname){
    return -1;
}

int unlink (const char *filename){
    return -1;
}

int kill(pid_t pid, int sig){
    return -1;
}

}

#endif
