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

/* Might return NULL if the path can't be found */
Util::ReferenceCount<File> Directory::lookup(const Path::AbsolutePath & path){
    if (path.isFile()){
        if (files[path.path()] != NULL){
            return files[path.path()];
        } else {
            return Util::ReferenceCount<File>(NULL);
        }
    } else {
        try{
            string name = path.firstDirectory();
            if (name == "."){
                return lookup(path.removeFirstDirectory());
            } else if (name == ".."){
                throw UpDirectory(path.removeFirstDirectory());
            }
            Util::ReferenceCount<Directory> directory = directories[path.firstDirectory()];
            if (directory != NULL){
                return directory->lookup(path.removeFirstDirectory());
            }
        } catch (const UpDirectory & up){
            return lookup(up.path);
        }
    }
    return Util::ReferenceCount<File>(NULL);
}

}
