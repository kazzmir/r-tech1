#ifndef _paintown_system_h
#define _paintown_system_h

/* system utilities */

#include <string>

namespace System{
    bool isDirectory(const std::string & path);
    bool readableFile(const std::string & path);
    bool readable(const std::string & path);
}

#endif
