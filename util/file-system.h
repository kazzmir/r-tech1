#ifndef _paintown_file_system_h
#define _paintown_file_system_h

#include "exceptions/exception.h"
#include "pointer.h"
#include "thread.h"
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <fstream>
#include <stdint.h>

struct stat;
/* path utilities */
namespace Path{
    class Path{
    public:
        const std::string & path() const;
        const std::string getExtension() const;
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

    class InsensitivePath: public Path {
    public:
        InsensitivePath(const Path & what);
        bool operator==(const Path & path) const;
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

        RelativePath removeFirstDirectory() const;
        RelativePath firstDirectory() const;

        /* true if there are no directory parts to this path
         * foo is a file
         * bar/foo is not a file
         */
        bool isFile() const;

        /* a/ + b/ = a/b/ */
        RelativePath join(const RelativePath & path) const;
        RelativePath & operator=(const RelativePath & copy);

        bool operator==(const RelativePath & path) const;
        bool operator!=(const RelativePath & path) const;
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
        bool operator!=(const AbsolutePath & path) const;
        
        /* Remove a given path from the start of this path */
        virtual RelativePath remove(const AbsolutePath & path) const;

        virtual AbsolutePath getDirectory() const;
        virtual AbsolutePath getFilename() const;
        
        AbsolutePath join(const RelativePath & path) const;
    };

    std::string invertSlashes(std::string str);
    std::string sanitize(std::string path);

    /* remove extension. foo.txt -> foo */
    std::string removeExtension(const std::string & str);

    /* basename, just get the filename and remove the directory part */
    std::string stripDir(const std::string & str);
    
    /* dirname, just get the directory and remove the filename part */
    std::string stripFilename(const std::string & str);
}

namespace Storage{
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
        virtual void throwSelf() const {
            throw *this;
        }

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
        virtual void throwSelf() const {
            throw *this;
        }
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
        virtual void throwSelf() const {
            throw *this;
        }
    protected:
        virtual Exc::Base * copy() const {
            return new IllegalPath(*this);
        }
    };

    class Eof: public std::exception {
        public:
            Eof(){
            }

            virtual ~Eof() throw (){
            }
    };

    class EndianReader{
        public:
            EndianReader(std::ifstream & stream):
                stream(stream){
                }

            virtual ~EndianReader(){
            }

            virtual int8_t readByte1(){
                return convert(readBytes(sizeof(int8_t)));
            }

            virtual int16_t readByte2(){
                return convert(readBytes(sizeof(int16_t)));
            }

            virtual int32_t readByte4(){
                return convert(readBytes(sizeof(int32_t)));
            }

            virtual std::string readStringX(int length);
            virtual std::string readString2(int length);
            virtual void readBytes(uint8_t * out, int length);

            virtual void seekEnd(std::streamoff where);
            virtual void seek(std::streampos where);

            virtual int position();

        protected:
            virtual int32_t convert(const std::vector<uint8_t> & bytes) = 0;

            std::vector<uint8_t> readBytes(int length);
                
            std::ifstream & stream;
    };

    /* combines bytes b0 b1 b2 b3 as b0 + b1*2^8 + b2*2^16 + b3*2^24 */
    class LittleEndianReader: public EndianReader {
        public:
            LittleEndianReader(std::ifstream & stream):
                EndianReader(stream){
                }
        protected:
            virtual int32_t convert(const std::vector<uint8_t> & bytes){
                uint32_t out = 0;
                for (std::vector<uint8_t>::const_reverse_iterator it = bytes.rbegin(); it != bytes.rend(); it++){
                    out = (out << 8) + *it;
                }
                return out;
            }
    };

    /* combines bytes b0 b1 b2 b3 as b0*2^24 + b1*2^16 + b2*2^8 + b3 */
    class BigEndianReader: public EndianReader {
        public:
            BigEndianReader(std::ifstream & stream):
                EndianReader(stream){
                }
        protected:
            virtual int32_t convert(const std::vector<uint8_t> & bytes){
                uint32_t out = 0;
                for (std::vector<uint8_t>::const_iterator it = bytes.begin(); it != bytes.end(); it++){
                    out = (out << 8) + *it;
                }
                return out;
            }
    };

    /* Abstraction for files. Should be used instead of FILE, ifstream, SDL_RWOps, anything else */
    class File{
    public:
        enum Access{
            Read,
            Write,
            ReadWrite
        };

        File();
        virtual ~File();

        /* Returns the number of bytes read */
        virtual int readLine(char * output, int size) = 0;

        /* if the file is at eof and can't read anymore */
        virtual bool eof() = 0;

        /* if the file can still be read */
        virtual bool good() = 0;

        /* read one unsigned byte */
        virtual File & operator>>(unsigned char &) = 0;
    };
    
    class ZipContainer;
    class ZipFile: public File {
    public:
        ZipFile(const Path::AbsolutePath & path, const Util::ReferenceCount<ZipContainer> & container);
        virtual ~ZipFile();
        virtual bool eof();
        virtual bool good();
        virtual File & operator>>(unsigned char &);
        
        virtual int readLine(char * output, int size);

    protected:
        const Path::AbsolutePath path;
        const Util::ReferenceCount<ZipContainer> zip;
    };

    class NormalFile: public File {
    public:
        NormalFile(const Path::AbsolutePath & path, Access mode = Read);

        virtual int readLine(char * output, int size);
        virtual bool eof();
        virtual bool good();
        virtual File & operator>>(unsigned char &);

        virtual ~NormalFile();

    protected:
        const Path::AbsolutePath path;
        std::fstream in;
    };

    class StringFile: public File {
    public:
        StringFile(const std::string & start);
        virtual int readLine(char * output, int size);
        virtual bool eof();
        virtual bool good();
        virtual File & operator>>(unsigned char &);
        virtual ~StringFile();

    protected:
        std::string data;
        std::istringstream stream;
    };

    class System{
    public:
        System();
        virtual ~System();

        typedef Path::AbsolutePath AbsolutePath;
        typedef Path::RelativePath RelativePath;

        virtual AbsolutePath find(const RelativePath & path) = 0;
        virtual RelativePath cleanse(const AbsolutePath & path) = 0;
        virtual bool exists(const RelativePath & path) = 0;
        virtual bool exists(const AbsolutePath & path) = 0;
        virtual std::vector<AbsolutePath> getFilesRecursive(const AbsolutePath & dataPath, const std::string & find, bool caseInsensitive = false) = 0;

        /* search for a pattern of a single file within a directory */
        virtual std::vector<AbsolutePath> getFiles(const AbsolutePath & dataPath, const std::string & find, bool caseInsensitive = false) = 0;

        virtual void addOverlay(const AbsolutePath & container, const AbsolutePath & where);

        /* search for some path which may contain wildcards in a directory */
        virtual std::vector<AbsolutePath> getFiles(const AbsolutePath & dataPath, const RelativePath & find, bool caseInsensitive);
        virtual AbsolutePath configFile() = 0;
        virtual AbsolutePath userDirectory() = 0;
        virtual std::vector<AbsolutePath> findDirectories(const RelativePath & path) = 0;
        virtual AbsolutePath findInsensitive(const RelativePath & path) = 0;
        virtual AbsolutePath lookupInsensitive(const AbsolutePath & directory, const RelativePath & path) = 0;

        virtual Util::ReferenceCount<File> open(const AbsolutePath & path, File::Access mode = File::Read);
    protected:
        void overlayFile(const AbsolutePath & where, Util::ReferenceCount<ZipContainer> zip);
        std::map<AbsolutePath, Util::ReferenceCount<ZipContainer> > overlays;
    };

    System & instance();
    bool hasInstance();
    System & setInstance(const Util::ReferenceCount<System> & what);
}

/*
 * class Filesystem
 * class NetworkStorage
 * class ZipStorage
 */
class Filesystem: public Storage::System {
public:
    Filesystem(const Path::AbsolutePath & dataPath);

    typedef Path::AbsolutePath AbsolutePath;
    typedef Path::RelativePath RelativePath;
    typedef Path::InsensitivePath InsensitivePath;
    typedef Storage::Exception Exception;
    typedef Storage::NotFound NotFound;

    /* given a relative path like sounds/arrow.png, prepend the proper
     * data path to it to give data/sounds/arrow.png
     */
    AbsolutePath find(const RelativePath & path);

    /* like `find' but ignores case */
    AbsolutePath findInsensitive(const RelativePath & path);

    /* findInsensitive but starts in the given absolute directory path */
    AbsolutePath lookupInsensitive(const AbsolutePath & directory, const RelativePath & path);

    // void initialize();

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

    /* user specific directory to hold persistent data */
    AbsolutePath userDirectory();

    /* user specific path to store the configuration file */
    AbsolutePath configFile();

    using Storage::System::getFiles;
    /* search a directory for some files matching pattern `find' */
    std::vector<AbsolutePath> getFiles(const AbsolutePath & dataPath, const std::string & find, bool caseInsensitive = false);

    /* same as getFiles but search directories recursively */
    std::vector<AbsolutePath> getFilesRecursive(const AbsolutePath & dataPath, const std::string & find, bool caseInsensitive = false);

protected:
    AbsolutePath lookup(const RelativePath path);
    std::vector<AbsolutePath> findDirectoriesIn(const AbsolutePath & path);
    std::vector<AbsolutePath> getAllDirectories(const AbsolutePath & path);

protected:
    Util::Thread::LockObject lock;
    AbsolutePath dataPath;
};

#endif
