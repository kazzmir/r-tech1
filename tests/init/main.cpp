#include <r-tech1/init.h>
#include <r-tech1/debug.h>
#include <r-tech1/file-system.h>
#include <r-tech1/graphics/bitmap.h>

Filesystem::AbsolutePath Filesystem::configFile(){
    return Filesystem::AbsolutePath("config");
}

Filesystem::AbsolutePath Filesystem::userDirectory(){
    return Filesystem::AbsolutePath("/tmp");
}

int main(int argc, char ** argv){
    
    Global::InitConditions conditions;
    
    Global::debug(0) << "Starting up r-tech1..." << std::endl;
    
    Global::init(conditions);
    
    Global::debug(0) << "Done! Exiting..." << std::endl;
    
    Graphics::Bitmap::shutdown();
    Global::close();
    
    return 0;
}
