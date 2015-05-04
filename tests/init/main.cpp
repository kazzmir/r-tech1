#include <r-tech1/init.h>
#include <r-tech1/debug.h>

int main(int argc, char ** argv){
    
    Global::InitConditions conditions;
    
    Global::debug(0) << "Starting up r-tech1..." << std::endl;
    
    Global::init(conditions);
    
    Global::debug(0) << "Done! Exiting..." << std::endl;
    
    Global::close();
    
    return 0;
}
