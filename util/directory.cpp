#include "file-system.h"
#include <string>
#include <vector>
#include <map>

#ifndef USE_ALLEGRO
#include "sfl/sfl.h"
#include "sfl/sfldir.h"
#endif

using std::string;
using std::vector;
using std::map;

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
        
vector<Path::AbsolutePath> Directory::findFiles(const Path::AbsolutePath & dataPath, const std::string & find, bool caseInsensitive){
    vector<Path::AbsolutePath> out;

    class FindDirectory: public Traverser {
    public:
        FindDirectory():
        failed(false){
        }

        bool failed;
        Util::ReferenceCount<Directory> last;

        virtual void traverseFile(Directory & directory, const string & file){
        }

        virtual void traverseDirectory(Directory & directory, const string & path){
            if (directory.directories[path] != NULL){
                last = directory.directories[path];
            } else {
                failed = true;
            }
        }
    };

    FindDirectory lastDirectory;
    traverse(dataPath, lastDirectory);

    if (lastDirectory.failed || lastDirectory.last == NULL){
        return out;
    }

    vector<string> names = lastDirectory.last->filenames();
#ifndef USE_ALLEGRO
    for (vector<string>::iterator it = names.begin(); it != names.end(); it++){
        if (file_matches(it->c_str(), find.c_str())){
            out.push_back(dataPath.join(Path::RelativePath(*it)));
        }
    }
#endif

    return out;
}
        
vector<string> Directory::filenames() const {
    vector<string> out;

    for (map<string, Util::ReferenceCount<Descriptor> >::const_iterator it = files.begin(); it != files.end(); it++){
        out.push_back(it->first);
    }

    for (map<string, Util::ReferenceCount<Directory> >::const_iterator it = directories.begin(); it != directories.end(); it++){
        out.push_back(it->first);
    }
    
    return out;
}

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
