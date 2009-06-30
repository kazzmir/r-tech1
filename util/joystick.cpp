#include <stdlib.h>
#include "joystick.h"

/*
#ifdef LINUX
#include "linux_joystick.h"
#endif
*/

#include "allegro-joystick.h"

Joystick * Joystick::create(){
    return new AllegroJoystick();
/*
#ifdef LINUX
    return new LinuxJoystick();
#endif
    return NULL;
*/
}

Joystick::Joystick(){
}
    
Joystick::~Joystick(){
}
