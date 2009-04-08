#include <stdlib.h>
#include "joystick.h"

#ifdef LINUX
#include "linux_joystick.h"
#endif

Joystick * Joystick::create(){
#ifdef LINUX
    return new LinuxJoystick();
#endif
    return NULL;
}

Joystick::Joystick(){
}
    
Joystick::~Joystick(){
}
