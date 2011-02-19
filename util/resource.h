#ifndef _paintown_resource_manager_h
#define _paintown_resource_manager_h

#include "factory/collector.h"
#include "load_exception.h"
#include "file-system.h"
#include <map>
#include <string>
#include <vector>

class Sound;
namespace Graphics{
class Bitmap;
}

class Resource{
public:
    /* do not prepend Util::getDataPath() to paths. Resource will do it for you.
     */
    static Sound * getSound(const Filesystem::RelativePath & path) throw (LoadException);
    static Graphics::Bitmap * getBitmap(const Filesystem::RelativePath & path) throw (LoadException);
private:
    friend class Collector;
    Resource();
    virtual ~Resource();

    Sound * _getSound(const Filesystem::AbsolutePath & path) throw (LoadException);
    Graphics::Bitmap * _getBitmap(const Filesystem::AbsolutePath & path) throw (LoadException);

private:
    static Resource * resource;
    std::map<std::string, Sound*> sounds;
    std::map<std::string, Graphics::Bitmap*> bitmaps;
};

#endif
