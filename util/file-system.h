#ifndef _paintown_file_system_h
#define _paintown_file_system_h

#include "exceptions/exception.h"
#include <string>
#include <vector>

namespace Filesystem{
    /* sorry for the crappy abbreviation, but can't collide with the
     * Exception class here
     */
    namespace Exc = ::Exception;

    class Exception: public Exc::Base {
    public:
        Exception(const std::string & where, int line, const std::string & file);
        Exception(const std::string & where, int line, const Exc::Base & nested, const std::string & file);
        Exception(const Exception & copy);
        virtual ~Exception() throw ();

    protected:
        virtual const std::string getReason() const;

        virtual Exc::Base * copy() const {
            return new Exception(*this);
        }

    private:
        std::string reason;
    };

    class NotFound: public Exception {
    public:
        NotFound(const std::string & where, int line, const std::string & file);
        NotFound(const std::string & where, int line, const Exc::Base & nested, const std::string & file);
        virtual ~NotFound() throw();
        NotFound(const NotFound & copy);
    protected:
        virtual Exc::Base * copy() const {
            return new NotFound(*this);
        }
    };

    class IllegalPath: public Exception {
    public:
        IllegalPath(const std::string & where, int line, const std::string & file);
        IllegalPath(const std::string & where, int line, const Exc::Base & nested, const std::string & file);
        virtual ~IllegalPath() throw();
        IllegalPath(const IllegalPath & copy);
    protected:
        virtual Exc::Base * copy() const {
            return new IllegalPath(*this);
        }
    };

    class Path{
    public:
        const std::string & path() const;
        bool isEmpty() const;
        
        virtual ~Path();

    protected:
        Path();
        Path(const std::string & path);
        Path(const Path & path);

        virtual inline void setPath(const std::string & s){
            mypath = s;
        }

        std::string mypath;
    };

    /* relative path should not have the leading data directory on it, just
     * the path within the paintown system.
     */
    class RelativePath: public Path {
    public:
        explicit RelativePath();
        explicit RelativePath(const std::string & path);
        RelativePath(const RelativePath & path);

        bool operator<(const RelativePath & path) const;
        
        virtual RelativePath getDirectory() const;
        virtual RelativePath getFilename() const;

        /* a/ + b/ = a/b/ */
        RelativePath join(const RelativePath & path) const;
        RelativePath & operator=(const RelativePath & copy);

        bool operator==(const RelativePath & path) const;
    };

    /* absolute paths should have the entire filesystem path on it */
    class AbsolutePath: public Path {
    public:
        explicit AbsolutePath();
        explicit AbsolutePath(const std::string & path);
        AbsolutePath(const AbsolutePath & path);
        AbsolutePath & operator=(const AbsolutePath & copy);

        bool operator<(const AbsolutePath & path) const;
        bool operator==(const AbsolutePath & path) const;
        
        virtual AbsolutePath getDirectory() const;
        virtual AbsolutePath getFilename() const;
        
        AbsolutePath join(const RelativePath & path) const;
    };

    /* given a relative path like sounds/arrow.png, prepend the proper
     * data path to it to give data/sounds/arrow.png
     */
    AbsolutePath find(const RelativePath & path);

    /* whether the file exists at all */
    bool exists(const RelativePath & path);
    bool exists(const AbsolutePath & path);

    /* remove the data path from a string
     * data/sounds/arrow.png -> sounds/arrow.png
     */
    RelativePath cleanse(const AbsolutePath & path);

    /* returns all the directories starting with the given path.
     * will look in the main data directory, the user directory, and
     * the current working directory.
     */
    std::vector<AbsolutePath> findDirectories(const RelativePath & path);

    /* basename, just get the filename and remove the directory part */
    std::string stripDir(const std::string & str);

    /* remove extension. foo.txt -> foo */
    std::string removeExtension(const std::string & str);

    /* user specific directory to hold persistent data */
    AbsolutePath userDirectory();

    /* user specific path to store the configuration file */
    AbsolutePath configFile();

    /* search a directory for some files matching pattern `find' */
    std::vector<AbsolutePath> getFiles(const AbsolutePath & dataPath, const std::string & find, bool caseInsensitive = false);

    /* same as getFiles but search directories recursively */
    std::vector<AbsolutePath> getFilesRecursive(const AbsolutePath & dataPath, const std::string & find, bool caseInsensitive = false);

    std::string invertSlashes(const std::string & str);
    std::string sanitize(std::string path);
}

#endif
