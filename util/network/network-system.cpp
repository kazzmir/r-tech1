#ifdef HAVE_NETWORKING

#include "network-system.h"

namespace Storage{

typedef Path::AbsolutePath AbsolutePath;
typedef Path::RelativePath RelativePath;

/* TODO */
NetworkSystem::NetworkSystem(){
}

/* TODO */
NetworkSystem::~NetworkSystem(){
}

/* TODO */
AbsolutePath NetworkSystem::find(const RelativePath & path){
    return AbsolutePath();
}

/* TODO */
RelativePath NetworkSystem::cleanse(const AbsolutePath & path){
    return RelativePath();
}

/* TODO */
bool NetworkSystem::exists(const RelativePath & path){
    return false;
}

/* TODO */
bool NetworkSystem::exists(const AbsolutePath & path){
    return false;
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
