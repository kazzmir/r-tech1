#include <string>
#include "system.h"

#ifndef WINDOWS
#include <unistd.h>
#include <sys/stat.h>
#endif

#ifndef WINDOWS
static bool isReadable(const std::string & path){
    if (access(path.c_str(), R_OK) == 0){
        return true;
    } else {
        return false;
    }
}

static bool isDirectory(const std::string & path){
    struct stat info;
    if (stat(path.c_str(), &info) == 0){
        if (S_ISDIR(info.st_mode) == 1){
            return true;
        } else {
            return false;
        }
    }
    return false;
}

bool System::readableFile(const std::string & path){
    return isReadable(path) && ! isDirectory(path);
}
#endif
