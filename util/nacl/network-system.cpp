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
#include "../funcs.h"
#include "../debug.h"
#include <ppapi/c/pp_errors.h>
#include <ppapi/cpp/url_loader.h>
#include <ppapi/cpp/url_request_info.h>
#include <ppapi/cpp/completion_callback.h>

using std::string;

namespace Nacl{

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

    virtual void start(pp::CompletionCallback callback, pp::Core * core) = 0;
};

struct NaclRequestOpen: public NaclRequest {
    NaclRequestOpen(pp::Instance * instance, const string & url):
        request(instance),
        loader(instance){
            request.SetURL(url);
            request.SetMethod("GET");
        }

    void start(pp::CompletionCallback callback, pp::Core * core){
        Global::debug(0) << "Request open" << std::endl;
        int32_t ok = loader.Open(request, callback);
        Global::debug(0) << "Open " << ok << std::endl;
        if (ok != PP_OK_COMPLETIONPENDING){
            // Global::debug(0) << "Call on main thread" << std::endl;
            // core->CallOnMainThread(0, callback, ok);
            callback.Run(ok);
        }
        Global::debug(0) << "Callback running" << std::endl;
    }

    pp::URLRequestInfo request;
    pp::URLLoader loader;
};

class Manager{
public:
    Manager(pp::Instance * instance, pp::Core * core, Util::Thread::LockObject & portal):
    instance(instance),
    core(core),
    portal(portal),
    factory(NULL){
    }

    pp::Instance * instance;
    pp::Core * core;
    Util::Thread::LockObject & portal;
    Util::ReferenceCount<NaclRequest> request;
    pp::CompletionCallbackFactory<Manager> * factory;

    /*
    void start(){
        pp::CompletionCallback callback = factory.NewCallback(&Manager::OnOpen);
        int32_t ok = loader.Open(request, callback);
        Global::debug(0) << "Open " << ok << std::endl;
        if (ok != PP_OK_COMPLETIONPENDING){
            callback.Run(ok);
        }
        Global::debug(0) << "Callback running" << std::endl;
    }
    */

    static void doStartOpen(void * self, int32_t result){
        Manager * me = (Manager*) self;
        me->start();
    }

    void start(){
        factory = new pp::CompletionCallbackFactory<Manager>(this);
        request = new NaclRequestOpen(instance, operation.absolute.path());
        pp::CompletionCallback callback(&Manager::doOnOpen, this);
        request->start(callback, core);
    }

    static void doOnOpen(void * self, int32_t result){
        Manager * me = (Manager*) self;
        me->OnOpen(result);
    }

    void OnOpen(int32_t response){
        if (response == 0){
            Global::debug(0) << "Opened!" << std::endl;
            operation.success = true;
        } else {
            Global::debug(0) << "Failed to open :( " << response << std::endl;
            operation.success = false;
        }

        run2();
    }

    void run(){
        Global::debug(0) << "Requesting a file operation" << std::endl;
            
        // Global::debug(0) << "PPB_URLLoader;0.1 = " << pp::Module::Get()->GetBrowserInterface("PPB_URLLoader;0.1") << std::endl;

        Global::debug(0) << "Made request. Making callback.." << std::endl;
        // pp::CompletionCallback callback = factory->NewCallback(&Manager::OnOpen);
        pp::CompletionCallback callback(&Manager::doStartOpen, this);
        Global::debug(0) << "Made callback" << std::endl;
        core->CallOnMainThread(0, callback, 0);
        // request->start(callback, core);
        /* the handler will call into the browser. the browser will asynchounsly
         * call the handler again and the handler will call run2 to finish up
         */
        Global::debug(0) << "Releasing control back to the browser" << std::endl;
    }

    void run2(){
        portal.acquire();
        Global::debug(0) << "File operation done" << std::endl;
        operation.complete = true;
        portal.signal();
        delete factory;
        factory = NULL;
        portal.release();
    }

    /*
    pp::URLRequestInfo request;
    pp::URLLoader loader;
    */
};

NetworkSystem::NetworkSystem(const string & serverPath, pp::Instance * instance, pp::Core * core):
instance(instance),
serverPath(serverPath),
manager(new Manager(instance, core, portal)){
}

/* TODO */
NetworkSystem::~NetworkSystem(){
}

/* TODO */
AbsolutePath NetworkSystem::find(const RelativePath & path){
    return Util::getDataPath2().join(path);
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

void NetworkSystem::run(){
    manager->run();
    /*
    Util::Thread::Id thread;
    Util::Thread::createThread(&thread, NULL, &Manager::run, manager);
    */
    // manager->run();
}

bool NetworkSystem::exists(const AbsolutePath & path){
    /*
    Global::debug(0) << "Checking for " << path.path() << std::endl;
    pp::CompletionCallback::Block blocking;
    pp::CompletionCallback callback(blocking);
    pp::URLRequestInfo request(instance);
    pp::URLLoader loader(instance); 
    string url = serverPath + path.path();
    Global::debug(0) << "Url: " << url << std::endl;
    // request.SetURL(serverPath + path.path());
    request.SetURL("/" + path.path());
    request.SetMethod("GT");
    int32_t ok = loader.Open(request, callback);
    Global::debug(0) << "Ok: " << ok << std::endl;
    if (ok == PP_OK){
        return true;
    } else {
        return false;
    }
    */
    
    Global::debug(0) << "Getting portal lock" << std::endl;
    if (portal.acquire() != 0){
        Global::debug(0) << "Lock failed!" << std::endl;
    }
    /*
    Global::debug(0) << "Getting portal lock again" << std::endl;
    if (portal.acquire() != 0){
        Global::debug(0) << "Lock failed again!" << std::endl;
    }
    Global::debug(0) << "Somehow got the portal lock twice??" << std::endl;
    */
    operation.type = Exists;
    operation.absolute = path;
    operation.complete = false;
    operation.success = false;
    Global::debug(0) << "Request open" << std::endl;
    run();
    // portal.signal();
    Global::debug(0) << "Waiting on portal" << std::endl;
    portal.wait(operation.complete);
    portal.release();
    Global::debug(0) << "Request open complete" << std::endl;

    return operation.success;
    /*
    Handler handler(instance, path);
    handler.start();
    handler.wait();
    return false;
    */
}

/* TODO */
std::vector<AbsolutePath> NetworkSystem::getFilesRecursive(const AbsolutePath & dataPath, const std::string & find, bool caseInsensitive){
    std::vector<AbsolutePath> paths;
    return paths;
}

/* TODO */
std::vector<AbsolutePath> NetworkSystem::getFiles(const AbsolutePath & dataPath, const std::string & find, bool caseInsensitive){
    std::vector<AbsolutePath> paths;
    return paths;
}

/* TODO */
AbsolutePath NetworkSystem::configFile(){
    return AbsolutePath();
}

/* TODO */
AbsolutePath NetworkSystem::userDirectory(){
    return AbsolutePath();
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

}

/* NOTE FIXME Missing I/O in Native Client */

extern "C" {

/* http://sourceware.org/binutils/docs-2.21/ld/Options.html#index-g_t_002d_002dwrap_003d_0040var_007bsymbol_007d-261
 * --wrap=symbol
 * Use a wrapper function for symbol. Any undefined reference to symbol will be resolved to __wrap_symbol. Any undefined reference to __real_symbol will be resolved to symbol.
 */
int __wrap_open(const char * path, int mode, int params){
    Global::debug(0) << "Called open" << std::endl;
    return -1;
}

ssize_t __wrap_read(int fd, void * buf, size_t count){
    Global::debug(0) << "Called read" << std::endl;
    return EBADF;
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
