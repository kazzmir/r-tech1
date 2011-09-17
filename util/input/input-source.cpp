#include "input-source.h"

InputSource::InputSource():
keyboard(0),
joystick(0){
}
    
InputSource::InputSource(int keyboard, int joystick):
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
    return keyboard >= 0;
}

bool InputSource::useJoystick() const {
    return joystick >= 0;
}

int InputSource::getKeyboard() const{
    return keyboard;
}
    
int InputSource::getJoystick() const {
    return joystick;
}
