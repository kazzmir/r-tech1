#ifndef _paintown_file_system_h
#define _paintown_file_system_h

#include <exception>
#include <string>
#include <vector>

namespace Filesystem{
    class NotFound: public std::exception {
    public:
        NotFound(const std::string & file);
        virtual ~NotFound() throw();

        const std::string & getReason() const {
            return reason;
        }

    private:
        std::string reason;
    };

    /* given a relative path like sounds/arrow.png, prepend the proper
     * data path to it to give data/sounds/arrow.png
     */
    std::string find(const std::string & path) throw (NotFound);

    /* remove the data path from a string
     * data/sounds/arrow.png -> sounds/arrow.png
     */
    std::string cleanse(const std::string & path);

    /* returns all the directories starting with the given path.
     * will look in the main data directory, the user directory, and
     * the current working directory.
     */
    std::vector<std::string> findDirectories(const std::string & path);
}

#endif
