#ifndef _paintown_file_system_h
#define _paintown_file_system_h

#include <exception>
#include <string>
#include <vector>

namespace Filesystem{

    class Exception: public std::exception {
    public:
        Exception(const std::string & file);
        virtual ~Exception() throw ();

        const std::string & getReason() const {
            return reason;
        }
    private:
        std::string reason;
    };

    class NotFound: public Exception {
    public:
        NotFound(const std::string & file);
        virtual ~NotFound() throw();
    };

    class IllegalPath: public Exception {
    public:
        IllegalPath(const std::string & file);
        virtual ~IllegalPath() throw();
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
        
        virtual RelativePath getDirectory() const;
        virtual RelativePath getFilename() const;

        /* a/ + b/ = a/b/ */
        RelativePath join(const RelativePath & path) const;
        RelativePath & operator=(const RelativePath & copy);
    };

    /* absolute paths should have the entire filesystem path on it */
    class AbsolutePath: public Path {
    public:
        explicit AbsolutePath();
        explicit AbsolutePath(const std::string & path);
        AbsolutePath(const AbsolutePath & path);
        AbsolutePath & operator=(const AbsolutePath & copy);

        bool operator==(const AbsolutePath & path) const;
        
        virtual AbsolutePath getDirectory() const;
        virtual AbsolutePath getFilename() const;
        
        AbsolutePath join(const RelativePath & path) const;
    };

    /* given a relative path like sounds/arrow.png, prepend the proper
     * data path to it to give data/sounds/arrow.png
     */
    AbsolutePath find(const RelativePath & path);

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
}

#endif
