#include "input-source.h"

InputSource::InputSource():
keyboard(true),
joystick(0){
}
    
InputSource::InputSource(bool keyboard, int joystick):
keyboard(keyboard),
joystick(joystick){
}

InputSource::~InputSource(){
}
    
bool InputSource::useKeyboard() const {
    return keyboard;
}
    
int InputSource::getJoystick() const {
    return joystick;
}
