#include <string>
#include "system.h"
#include <stdint.h>

#ifndef WINDOWS
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#endif

#ifndef WINDOWS
static bool isReadable(const std::string & path){
    if (access(path.c_str(), R_OK) == 0){
        return true;
    } else {
        return false;
    }
}

bool System::isDirectory(const std::string & path){
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

bool System::readable(const std::string & path){
    return isReadable(path);
}

uint64_t System::currentMicroseconds(){
    struct timeval hold;
    gettimeofday(&hold, NULL);
    return hold.tv_sec * 1000 * 1000 + hold.tv_usec;
}

#endif
