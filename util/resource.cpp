#include "bitmap.h"
#include "resource.h"
#include "factory/collector.h"
#include "funcs.h"
#include "sound.h"
#include "load_exception.h"
#include "file-system.h"
#include <string>
#include <vector>

using namespace std;

Resource * Resource::resource = NULL;
Sound * Resource::getSound(const Filesystem::RelativePath & path) throw (LoadException){
    return resource->_getSound(Filesystem::find(path));
}

Bitmap * Resource::getBitmap(const Filesystem::RelativePath & path) throw (LoadException){
    return resource->_getBitmap(Filesystem::find(path));
}

/* the resource is created in the Collector */
Resource::Resource(){
    resource = this;
}

Resource::~Resource(){
    for (std::map<std::string, Sound*>::iterator it = sounds.begin(); it != sounds.end(); it++){
        delete (*it).second;
    }
    
    for (std::map<std::string, Bitmap*>::iterator it = bitmaps.begin(); it != bitmaps.end(); it++){
        delete (*it).second;
    }
}

Sound * Resource::_getSound(const Filesystem::AbsolutePath & path) throw (LoadException){
    if (sounds[path.path()] == NULL){
        sounds[path.path()] = new Sound(path.path());
    }
    return sounds[path.path()];
}
    
Bitmap * Resource::_getBitmap(const Filesystem::AbsolutePath & path) throw (LoadException){
    if (bitmaps[path.path()] == NULL){
        bitmaps[path.path()] = new Bitmap(path.path());
    }

    return bitmaps[path.path()];
}
