#ifndef _paintown_file_system_h
#define _paintown_file_system_h

#include "exceptions/exception.h"
#include <string>
#include <vector>
#include <stdint.h>

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

    /* like `find' but ignores case */
    AbsolutePath findInsensitive(const RelativePath & path);

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
}

#endif
