#ifndef _paintown_network_system_h
#define _paintown_network_system_h

#include <string>
#include "../file-system.h"

#ifdef NACL
namespace pp{
    class Instance;
}
#endif

namespace Storage{
    
class NetworkSystem: public System {
public:
#ifdef NACL
    NetworkSystem(const std::string & serverPath, pp::Instance * instance);
#else
    NetworkSystem();
#endif
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

protected:
#ifdef NACL
    pp::Instance * instance;
    std::string serverPath;
#endif

};

}

#endif