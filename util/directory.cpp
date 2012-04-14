#include "file-system.h"
#include <string>

using std::string;

namespace Storage{

Directory::Directory(){
}

Directory::~Directory(){
}

/* Used to signal .. in a path */
class UpDirectory: public std::exception {
public:
    UpDirectory(Path::AbsolutePath path):
    path(path){
    }

    virtual ~UpDirectory() throw () {
    }

    Path::AbsolutePath path;
};

Util::ReferenceCount<File> Directory::doLookup(const Path::AbsolutePath & path){
    if (path.isFile()){
        if (files[path.path()] != NULL){
            return files[path.path()];
        } else {
            return Util::ReferenceCount<File>(NULL);
        }
    } else {
        string name = path.firstDirectory();
        if (name == "."){
            try{
                return doLookup(path.removeFirstDirectory());
            } catch (const UpDirectory & up){
                return doLookup(up.path);
            }
        } else if (name == ".."){
            throw UpDirectory(path.removeFirstDirectory());
        }
        Util::ReferenceCount<Directory> directory = directories[path.firstDirectory()];
        if (directory != NULL){
            try{
                return directory->doLookup(path.removeFirstDirectory());
            } catch (const UpDirectory & up){
                return doLookup(up.path);
            }
        }
    }
    return Util::ReferenceCount<File>(NULL);
}

/* Might return NULL if the path can't be found */
Util::ReferenceCount<File> Directory::lookup(const Path::AbsolutePath & path){
    Path::AbsolutePath use = path;

    /* The path might contain .. paths that would go out of this directory
     * so just ignore those paths and use the rest of the path.
     */
    while (true){
        try{
            return doLookup(use);
        } catch (const UpDirectory & up){
            use = up.path;
        }
    }
}

}
