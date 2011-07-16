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
        Global::debug(1) << "Request open for url " << url << std::endl;
        pp::CompletionCallback callback(&NaclRequestOpen::onFinish, this);
        int32_t ok = loader.Open(request, callback);
        Global::debug(1) << "Open " << ok << std::endl;
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

struct NaclRequestRead: public NaclRequest {
    NaclRequestRead(pp::URLLoader & loader, Manager * manager, void * buffer, int read):
        loader(loader),
        manager(manager),
        buffer(buffer),
        read(read){
        }

    void start(){
        pp::CompletionCallback callback(&NaclRequestRead::onFinish, this);
        int32_t ok = loader.ReadResponseBody(buffer, read, callback);
        Global::debug(1) << "Read " << ok << std::endl;
        if (ok != PP_OK_COMPLETIONPENDING){
            // Global::debug(0) << "Call on main thread" << std::endl;
            // core->CallOnMainThread(0, callback, ok);
            callback.Run(ok);
        }
        Global::debug(1) << "Callback running" << std::endl;
    }

    static void onFinish(void * me, int32_t result){
        NaclRequestRead * self = (NaclRequestRead*) me;
        self->finish(result);
    }

    void finish(int32_t result);

    pp::URLLoader loader;
    Manager * manager;
    void * buffer;
    int read;
};

class FileHandle{
public:
    FileHandle(const Util::ReferenceCount<NaclRequestOpen> & open):
        open(open),
        buffer(NULL){
        }

    ~FileHandle(){
        delete[] buffer;
    }

    pp::URLLoader & getLoader(){
        return open->loader;
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
            Global::debug(1) << "Read " << result << " bytes" << std::endl;
            current->size += result;
            if (result > 0){
                tries = 0;
                read();
            } else {
                if (tries >= 2){
                    handle->readDone(this);
                } else {
                    tries += 1;
                    pp::CompletionCallback callback(&Reader::doRead, this);
                    core->CallOnMainThread(50, callback, 0);
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

    void readAll(pp::CompletionCallback finish, pp::Core * core){
        reader = new Reader(finish, open->loader, this, core);
        reader->read();
    }

    void readDone(Reader * reader){
        length = reader->getSize();
        Global::debug(1) << "Done reading, got " << length << " bytes" << std::endl;
        position = 0;
        buffer = new char[length];
        reader->copy(buffer);
        reader->finish.Run(0);
    }

    int read(void * buffer, size_t count){
        size_t bytes = position + count < length ? count : (length - position);
        memcpy(buffer, this->buffer + position, bytes);
        position += bytes;
        return bytes;
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
                position = length - offset;
                break;
            }
        }

        return position;
    }

    Util::ReferenceCount<NaclRequestOpen> open;
    off_t position;
    off_t length;
    char * buffer;
    Util::ReferenceCount<Reader> reader;
};

class Manager{
public:
    Manager(pp::Instance * instance, pp::Core * core):
    instance(instance),
    core(core),
    factory(this){
        next = 2;
    }

    pp::Instance * instance;
    pp::Core * core;
    Util::ReferenceCount<NaclRequest> request;
    pp::CompletionCallbackFactory<Manager> factory;

    map<int, Util::ReferenceCount<FileHandle> > fileTable;

    int next;

    struct OpenFileData{
        const char * path;
        int file;
    };

    struct ExistsData{
        const char * path;
        bool exists;
    };

    struct ReadFileData{
        int file;
        void * buffer;
        /* how much to read */
        int count;
        /* how much was read */
        int read;
    };

    struct CloseFileData{
        int fd;
    };

    OpenFileData openFileData;
    ReadFileData readFileData;
    CloseFileData closeFileData;
    ExistsData existsData;

    Util::Thread::LockObject lock;
    volatile bool done;

    int openFile(const char * path){
        Global::debug(1, CONTEXT) << "open " << path << std::endl;
        Util::Thread::ScopedLock scoped(lock);
        done = false;
        openFileData.path = path;
        openFileData.file = -1;
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

    off_t lseek(int fd, off_t offset, int whence){
        Global::debug(2, CONTEXT) << "seek fd " << fd << " offset " << offset << " whence " << whence << std::endl;
        Util::Thread::ScopedLock scoped(lock);
        if (fileTable.find(fd) == fileTable.end()){
            return -1;
        }

        Util::ReferenceCount<FileHandle> handle = fileTable[fd];
        return handle->seek(offset, whence);
    }

    int close(int fd){
        Util::Thread::ScopedLock scoped(lock);
        if (fileTable.find(fd) == fileTable.end()){
            return -1;
            /* set errno to EBADF */
        }

        done = false;
        closeFileData.fd = fd;

        pp::CompletionCallback callback(&Manager::doCloseFile, this);
        core->CallOnMainThread(0, callback, 0);
        lock.wait(done);
        return 0;
    }

    static void doCloseFile(void * self, int32_t result){
        Manager * manager = (Manager*) self;
        manager->continueCloseFile();
    }

    /* the destructor for the NaclRequestOpen has to occur in the main thread */
    void continueCloseFile(){
        fileTable.erase(fileTable.find(closeFileData.fd));
        requestComplete();
    }

    ssize_t readFile(int fd, void * buffer, size_t count){
        Util::Thread::ScopedLock scoped(lock);
        if (fileTable.find(fd) == fileTable.end()){
            return EBADF;
        }

        /* dont need to sleep on a condition variable because we are
         * in the game thread.
         */

        Util::ReferenceCount<FileHandle> handle = fileTable[fd];
        return handle->read(buffer, count);

        /*
        done = false;
        readFileData.file = fd;
        readFileData.buffer = buffer;
        readFileData.count = count;
        readFileData.read = 0;
        pp::CompletionCallback callback(&Manager::doReadFile, this);
        core->CallOnMainThread(0, callback, 0);
        lock.wait(done);
        return readFileData.read;
        */
    }

    static void doReadFile(void * self, int32_t result){
        Manager * manager = (Manager*) self;
        manager->continueReadFile();
    }

    void continueReadFile(){
        /* hack to get the open request.. */
        Util::ReferenceCount<FileHandle> open = fileTable[readFileData.file];
        request = new NaclRequestRead(open->getLoader(), this, readFileData.buffer, readFileData.count);
        request->start();
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

    int nextFileDescriptor(){
        int n = next;
        next += 1;
        return n;
    }

    void requestComplete(){
        /* delete the reference counted request object in the main thread */
        request = NULL;
        lock.lockAndSignal(done, true);
    }

    void success(NaclRequestOpen & open){
        pp::URLResponseInfo info = open.loader.GetResponseInfo();
        if (info.GetStatusCode() == 200){
            Global::debug(1) << "Opened file" << std::endl;
            /*
            int64_t received = 0;
            int64_t total = 0;
            if (open.loader.GetDownloadProgress(&received, &total)){
                Global::debug(0) << "Downloaded " << received << " total " << total << std::endl;
            }
            */
            openFileData.file = nextFileDescriptor();
            fileTable[openFileData.file] = new FileHandle(request.convert<NaclRequestOpen>());
            readEntireFile(fileTable[openFileData.file]);
        } else {
            Global::debug(1) << "Could not open file" << std::endl;
            openFileData.file = -1;
            requestComplete();
        }
    }

    void readEntireFile(Util::ReferenceCount<FileHandle> & file){
        pp::CompletionCallback callback(&Manager::completeRead, this);
        file->readAll(callback, core);
    }

    static void completeRead(void * self, int32_t result){
        Manager * manager = (Manager*) self;
        manager->requestComplete();
    }

    void failure(NaclRequestOpen & open){
        openFileData.file = -1;
        requestComplete();
    }

    void success(NaclRequestRead & request, int read){
        readFileData.read = read;
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

void NaclRequestRead::finish(int32_t result){
    manager->success(*this, result);
}

NetworkSystem::NetworkSystem(pp::Instance * instance, pp::Core * core):
instance(instance),
manager(new Manager(instance, core)){
}

/* TODO */
NetworkSystem::~NetworkSystem(){
}

/* TODO */
AbsolutePath NetworkSystem::find(const RelativePath & path){
    AbsolutePath all = Util::getDataPath2().join(path);
    if (exists(all)){
        return all;
    }
    throw Storage::NotFound(__FILE__, __LINE__, path.path());
}

/* TODO */
RelativePath NetworkSystem::cleanse(const AbsolutePath & path){
    return RelativePath();
}

bool NetworkSystem::exists(const RelativePath & path){
    try{
        AbsolutePath absolute = find(path);
        return true;
    } catch (const Storage::NotFound & found){
        return false;
    }
}

/*
class Handler{
public:
    Handler(pp::Instance * instance, const AbsolutePath & path, NetworkSystem * system):
    request(instance),
    loader(instance),
    factory(this),
    http(false),
    system(system){
        request.SetURL(path.path());
        request.SetMethod("GET");
    }

    virtual void start(){
        pp::CompletionCallback callback = factory.NewCallback(&Handler::OnOpen);
        int32_t ok = loader.Open(request, callback);
        Global::debug(0) << "Open " << ok << std::endl;
        if (ok != PP_OK_COMPLETIONPENDING){
            callback.Run(ok);
        }
        Global::debug(0) << "Callback running" << std::endl;
    }

    virtual void OnOpen(int32_t ok){
        Global::debug(0) << "Opened! " << ok << std::endl;
        system->run2();
    }

    pp::URLRequestInfo request;
    pp::URLLoader loader;
    pp::CompletionCallbackFactory<Handler> factory;
    volatile bool http;
    NetworkSystem * system;
};
*/

/*
void NetworkSystem::run(){
    // manager->run();
    Util::Thread::Id thread;
    Util::Thread::createThread(&thread, NULL, &Manager::run, manager);
    // manager->run();
}
*/

bool NetworkSystem::exists(const AbsolutePath & path){
    Util::Thread::ScopedLock scoped(lock);
    if (existsCache.find(path) != existsCache.end()){
        return existsCache[path];
    }
    bool what = manager->exists(path.path());
    existsCache[path] = what;
    return what;
}

/* TODO */
std::vector<AbsolutePath> NetworkSystem::getFilesRecursive(const AbsolutePath & dataPath, const std::string & find, bool caseInsensitive){
    std::vector<AbsolutePath> paths;
    return paths;
}

string NetworkSystem::readFileAsString(const AbsolutePath & path){
    ostringstream buffer;
    ifstream input(path.path().c_str());
    char stuff[1024];
    while (input.good()){
        input.read(stuff, sizeof(stuff) - 1);
        stuff[sizeof(stuff) - 1] = '\0';
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
        paths.push_back(dataPath.join(RelativePath(what)));
    }
    return paths;
}

/* check if 'foo/bar/baz.txt' matches *.txt */
static bool matchFile(const AbsolutePath & path, const string & find){
    /* TODO */
    return false;
}

std::vector<AbsolutePath> NetworkSystem::getFiles(const AbsolutePath & dataPath, const std::string & find, bool caseInsensitive){

    vector<AbsolutePath> files = readDirectory(dataPath);

    if (find == "*"){
        return files;
    }

    vector<AbsolutePath> paths;
    for (vector<AbsolutePath>::iterator it = files.begin(); it != files.end(); it++){
        AbsolutePath check = *it;
        if (matchFile(check, find)){
            paths.push_back(check);
        }
    }
    return paths;
}

/* TODO */
AbsolutePath NetworkSystem::configFile(){
    return AbsolutePath("paintownrc");
}

/* TODO */
AbsolutePath NetworkSystem::userDirectory(){
    return AbsolutePath("paintownrc");
}

/* TODO */
std::vector<AbsolutePath> NetworkSystem::findDirectories(const RelativePath & path){
    return std::vector<AbsolutePath>();
}

/* TODO */
AbsolutePath NetworkSystem::findInsensitive(const RelativePath & path){
    return AbsolutePath();
}

/* TODO */
AbsolutePath NetworkSystem::lookupInsensitive(const AbsolutePath & directory, const RelativePath & path){
    return AbsolutePath();
}
    
int NetworkSystem::libcOpen(const char * path, int mode, int params){
    Util::Thread::ScopedLock scoped(lock);
    return manager->openFile(path);
}
    
ssize_t NetworkSystem::libcRead(int fd, void * buf, size_t count){
    Util::Thread::ScopedLock scoped(lock);
    return manager->readFile(fd, buf, count);
}

int NetworkSystem::libcClose(int fd){
    Util::Thread::ScopedLock scoped(lock);
    return manager->close(fd);
}
    
off_t NetworkSystem::libcLseek(int fd, off_t offset, int whence){
    Util::Thread::ScopedLock scoped(lock);
    return manager->lseek(fd, offset, whence);
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

int access(const char *filename, int how){
    return -1;
}

char * getcwd (char *buffer, size_t size){
    return NULL;
}

int lstat (const char *path, struct stat *buf){
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
