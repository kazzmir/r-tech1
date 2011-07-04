#ifndef _paintown_network_system_h
#define _paintown_network_system_h

#ifdef NACL

#include <string>
#include "../file-system.h"
#include "../thread.h"

namespace pp{
    class Instance;
}

namespace Nacl{

typedef Path::AbsolutePath AbsolutePath;
typedef Path::RelativePath RelativePath;
    
class NetworkSystem: public Storage::System {
public:
    NetworkSystem(const std::string & serverPath, pp::Instance * instance);
    virtual ~NetworkSystem();

    virtual AbsolutePath find(const RelativePath & path);
    virtual RelativePath cleanse(const AbsolutePath & path);
    virtual bool exists(const RelativePath & path);
    virtual bool exists(const AbsolutePath & path);
    virtual std::vector<AbsolutePath> getFilesRecursive(const AbsolutePath & dataPath, const std::string & find, bool caseInsensitive = false);
    virtual std::vector<AbsolutePath> getFiles(const AbsolutePath & dataPath, const std::string & find, bool caseInsensitive = false);
    virtual AbsolutePath configFile();
    virtual AbsolutePath userDirectory();
    virtual std::vector<AbsolutePath> findDirectories(const RelativePath & path);
    virtual AbsolutePath findInsensitive(const RelativePath & path);
    virtual AbsolutePath lookupInsensitive(const AbsolutePath & directory, const RelativePath & path);

    virtual void run();
    virtual void run2();

protected:
    pp::Instance * instance;
    std::string serverPath;
    Util::Thread::LockObject portal;
};

}

#endif

#endif
