#include "input-source.h"

InputSource::InputSource():
keyboard(true),
joystick(0){
}
    
InputSource::InputSource(bool keyboard, int joystick):
keyboard(keyboard),
joystick(joystick){
}
    
InputSource::InputSource(const InputSource & copy):
keyboard(copy.keyboard),
joystick(copy.joystick){
}
    
InputSource & InputSource::operator=(const InputSource & copy){
    this->keyboard = copy.keyboard;
    this->joystick = copy.joystick;
    return *this;
}

InputSource::~InputSource(){
}
    
bool InputSource::useKeyboard() const {
    return keyboard;
}
    
int InputSource::getJoystick() const {
    return joystick;
}
