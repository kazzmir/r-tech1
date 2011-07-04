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

    virtual void start(pp::CompletionCallback callback) = 0;
};

struct NaclRequestOpen: public NaclRequest {
    NaclRequestOpen(pp::Instance * instance, const string & url):
        request(instance),
        loader(instance){
            request.SetURL(url);
            request.SetMethod("GET");
        }

    void start(pp::CompletionCallback callback){
        Global::debug(0) << "Request open" << std::endl;
        int32_t ok = loader.Open(request, callback);
        Global::debug(0) << "Open " << ok << std::endl;
        if (ok != PP_OK_COMPLETIONPENDING){
            callback.Run(ok);
        }
        Global::debug(0) << "Callback running" << std::endl;
    }

    pp::URLRequestInfo request;
    pp::URLLoader loader;
};

class Manager{
public:
    Manager(pp::Instance * instance, Util::Thread::LockObject & portal):
    instance(instance),
    portal(portal),
    factory(this){
    }

    pp::Instance * instance;
    Util::Thread::LockObject & portal;
    Util::ReferenceCount<NaclRequest> request;
    pp::CompletionCallbackFactory<Manager> factory;

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

    void OnOpen(int32_t response){
        if (response == 0){
            Global::debug(0) << "Opened!" << std::endl;
            operation.success = true;
        } else {
            Global::debug(0) << "Failed to open :(" << std::endl;
            operation.success = false;
        }

        run2();
    }

    void doit(){
        Global::debug(0) << "Requesting a file operation" << std::endl;

        request = new NaclRequestOpen(instance, operation.absolute.path());
        Global::debug(0) << "Made request" << std::endl;
        request->start(factory.NewCallback(&Manager::OnOpen));
        /* the handler will call into the browser. the browser will asynchounsly
         * call the handler again and the handler will call run2 to finish up
         */
        Global::debug(0) << "Releasing control back to the browser" << std::endl;
    }

    static int run(void * me){
        Manager * self = (Manager*) me;
        self->doit();
        return 0;
    }

    void run2(){
        portal.acquire();
        Global::debug(0) << "File operation done" << std::endl;
        operation.complete = true;
        portal.signal();
        portal.release();
    }

    /*
    pp::URLRequestInfo request;
    pp::URLLoader loader;
    */
};

NetworkSystem::NetworkSystem(const string & serverPath, pp::Instance * instance):
instance(instance),
serverPath(serverPath),
manager(new Manager(instance, portal)){
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
    Util::Thread::Id thread;
    Util::Thread::createThread(&thread, NULL, &Manager::run, manager);
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
    
    portal.acquire();
    operation.type = Exists;
    operation.absolute = path;
    operation.complete = false;
    operation.success = false;
    Global::debug(0) << "Request open" << std::endl;
    run();
    // portal.signal();
    portal.wait(operation.complete);
    portal.release();
    Global::debug(0) << "Request open complete" << std::endl;

    return false;
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

#endif
