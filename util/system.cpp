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
#ifndef WII
    if (access(path.c_str(), R_OK) == 0){
        return true;
    } else {
        return false;
    }
#else
    /* FIXME */
    return true;
#endif
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
    
void System::makeDirectory(const std::string & path){
    mkdir(path.c_str(), 0777);
}

uint64_t System::currentMicroseconds(){
    struct timeval hold;
    gettimeofday(&hold, NULL);
    return hold.tv_sec * 1000 * 1000 + hold.tv_usec;
}

uint64_t System::getModificationTime(const std::string & path){
    struct stat data;
    if (stat(path.c_str(), &data) == 0){
        return data.st_mtime;
    }
    return 0;
}

static void * start_memory = 0;
unsigned long System::memoryUsage(){
    void * here = sbrk(0);
    /* hopefully the heap is growing up */
    return (char*) here - (char*) start_memory;
}

void System::startMemoryUsage(){
    start_memory = sbrk(0);
}

#endif
    
void System::makeAllDirectory(const std::string & path){
    unsigned int last = path.find('/');
    while (last != std::string::npos){
        std::string sofar = path.substr(0, last);
        if (sofar != ""){
            makeDirectory(sofar);
        }
        last = path.find('/', last + 1);
    }
    makeDirectory(path);
}
