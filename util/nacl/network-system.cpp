#ifdef NACL

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
};

Request operation;

NetworkSystem::NetworkSystem(const string & serverPath, pp::Instance * instance):
instance(instance),
serverPath(serverPath){
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

    /*
    void wait(){
        Global::debug(0) << "Waiting.." << std::endl;
        lock.acquire();
        Util::restSeconds(4);
        lock.release();
        // Util::restSeconds(4);
        // lock.wait(http);
        Global::debug(0) << "Done waiting" << std::endl;
    }
    */

    pp::URLRequestInfo request;
    pp::URLLoader loader;
    pp::CompletionCallbackFactory<Handler> factory;
    volatile bool http;
    NetworkSystem * system;
};

void NetworkSystem::run(){
    portal.acquire();
    portal.wait();
    Global::debug(0) << "Requesting a file operation" << std::endl;

    Handler * handler = new Handler(instance, operation.absolute, this);
    handler->start();
    /* the handler will call into the browser. the browser will asynchounsly
     * call the handler again and the handler will call run2 to finish up
     */
    Global::debug(0) << "Releasing control back to the browser" << std::endl;
}

void NetworkSystem::run2(){
    Global::debug(0) << "File operation done" << std::endl;
    operation.complete = true;
    portal.release();
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
    Global::debug(0) << "Request open" << std::endl;
    portal.signal();
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
