#ifndef _paintown_file_system_h
#define _paintown_file_system_h

#include <exception>
#include <string>

namespace Filesystem{
    class NotFound: public std::exception {
    public:
        NotFound(const std::string & file);
        virtual ~NotFound() throw();
    };

    /* given a relative path like sounds/arrow.png, prepend the proper
     * data path to it to give data/sounds/arrow.png
     */
    std::string find(const std::string & path) throw (NotFound);

    /* remove the data path from a string
     * data/sounds/arrow.png -> sounds/arrow.png
     */
    std::string cleanse(const std::string & path);
}

#endif
