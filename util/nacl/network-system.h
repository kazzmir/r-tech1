#ifndef _paintown_network_system_h
#define _paintown_network_system_h

#ifdef NACL

#include <map>
#include <string>
#include <vector>
#include "../file-system.h"
#include "../thread.h"
#include "../pointer.h"

namespace pp{
    class Instance;
    class Core;
}

namespace Nacl{

typedef Path::AbsolutePath AbsolutePath;
typedef Path::RelativePath RelativePath;
typedef Path::InsensitivePath InsensitivePath;
    
class Manager;
class FileHandle;

class NetworkSystem: public Storage::System {
public:
    NetworkSystem(pp::Instance * instance, pp::Core * core);
    virtual ~NetworkSystem();

    virtual AbsolutePath find(const RelativePath & path);
    virtual RelativePath cleanse(const AbsolutePath & path);
    virtual bool exists(const RelativePath & path);
    virtual bool exists(const AbsolutePath & path);
    virtual std::vector<AbsolutePath> getFilesRecursive(const AbsolutePath & dataPath, const std::string & find, bool caseInsensitive = false);
    using Storage::System::getFiles;
    virtual std::vector<AbsolutePath> getFiles(const AbsolutePath & dataPath, const std::string & find, bool caseInsensitive = false);
    virtual AbsolutePath configFile();
    virtual AbsolutePath userDirectory();
    virtual std::vector<AbsolutePath> findDirectories(const RelativePath & path);
    virtual AbsolutePath findInsensitive(const RelativePath & path);
    virtual AbsolutePath lookupInsensitive(const AbsolutePath & directory, const RelativePath & path);

public:
    int libcOpen(const char * path, int mode, int params);
    ssize_t libcRead(int fd, void * buf, size_t count);
    int libcClose(int fd);
    off_t libcLseek(int fd, off_t offset, int whence);

protected:
    std::string readFileAsString(const AbsolutePath & path);
    std::vector<AbsolutePath> readDirectory(const AbsolutePath & dataPath);

protected:
    pp::Instance * instance;
    pp::Core * core;
    /* only one thread at a time to access the network system */
    Util::Thread::LockObject lock;
    // Util::ReferenceCount<Manager> manager;
    std::map<AbsolutePath, bool> existsCache;
    std::map<int, Util::ReferenceCount<FileHandle> > fileTable;

};

}

#endif

#endif
