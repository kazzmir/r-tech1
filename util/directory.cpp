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

class Traverser{
public:
    Traverser(){
    }

    virtual void traverseFile(Directory & directory, const string & file) = 0;
    virtual void traverseDirectory(Directory & directory, const string & path) = 0;

    virtual ~Traverser(){
    }
};

void Directory::doTraverse(const Path::AbsolutePath & path, Traverser & traverser){
    if (path.isFile()){
        traverser.traverseFile(*this, path.path());
    } else {
        string name = path.firstDirectory();
        if (name == "."){
            try{
                return doTraverse(path.removeFirstDirectory(), traverser);
            } catch (const UpDirectory & up){
                return doTraverse(up.path, traverser);
            }
        } else if (name == ".."){
            throw UpDirectory(path.removeFirstDirectory());
        }
        traverser.traverseDirectory(*this, path.firstDirectory());
        Util::ReferenceCount<Directory> directory = directories[path.firstDirectory()];
        if (directory != NULL){
            try{
                return directory->doTraverse(path.removeFirstDirectory(), traverser);
            } catch (const UpDirectory & up){
                return doTraverse(up.path, traverser);
            }
        }
    }
}

void Directory::traverse(const Path::AbsolutePath & path, Traverser & traverser){
    Path::AbsolutePath use = path;

    /* The path might contain .. paths that would go out of this directory
     * so just ignore those paths and use the rest of the path.
     */
    while (true){
        try{
            return doTraverse(use, traverser);
        } catch (const UpDirectory & up){
            use = up.path;
        }
    }
}

/*
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
*/

/* Might return NULL if the path can't be found */
Util::ReferenceCount<Descriptor> Directory::lookup(const Path::AbsolutePath & path){
    class FindIt: public Traverser {
    public:

        virtual void traverseFile(Directory & directory, const string & file){
            found = directory.files[file];
        }

        virtual void traverseDirectory(Directory & directory, const string & path){
        }

        Util::ReferenceCount<Descriptor> found;
    };

    FindIt find;
    traverse(path, find);
    return find.found;

#if 0
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
#endif
}

void Directory::addFile(const Path::AbsolutePath & path, const Util::ReferenceCount<Descriptor> & file){
    class AddPath: public Traverser {
    public:
        AddPath(const Util::ReferenceCount<Descriptor> & file):
        file(file){
        }

        const Util::ReferenceCount<Descriptor> & file;

        virtual void traverseFile(Directory & directory, const string & path){
            directory.files[path] = file;
        }

        virtual void traverseDirectory(Directory & directory, const string & path){
            if (directory.directories[path] == NULL){
                directory.directories[path] = Util::ReferenceCount<Directory>(new Directory());
            }
        }
    };

    AddPath adder(file);
    traverse(path, adder);
}
        
void Directory::removeFile(const Path::AbsolutePath & path){
    addFile(path, Util::ReferenceCount<Descriptor>(NULL));
}

}
