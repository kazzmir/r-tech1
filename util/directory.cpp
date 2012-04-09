#include "file-system.h"

namespace Storage{

Directory::Directory(){
}

Directory::~Directory(){
}

/* Might return NULL if the path can't be found */
Util::ReferenceCount<File> Directory::lookup(const Path::AbsolutePath & path){
    return Util::ReferenceCount<File>(NULL);
}

}
