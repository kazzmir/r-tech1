#include <stdlib.h>
#include "util/configuration.h"
#include "joystick.h"

/*
#ifdef LINUX
#include "linux_joystick.h"
#endif
*/

#ifdef USE_ALLEGRO
#include "allegro/allegro-joystick.h"
#endif
#ifdef USE_ALLEGRO5
#include "allegro5/joystick.h"
#endif
#ifdef USE_SDL
#ifdef WII
#include "wii/joystick.h"
#include "sdl/joystick.h"
#elif MINPSPW
#include "psp/joystick.h"
#else
#include "sdl/joystick.h"
#endif
#endif
    
JoystickListener::JoystickListener(){
}

JoystickListener::~JoystickListener(){
}

Joystick * Joystick::create(int i){
#ifdef USE_ALLEGRO
    return new AllegroJoystick();
#endif
#ifdef USE_SDL
#ifdef WII
    return new SDLJoystick(i);
    // return new WiiJoystick();
#elif MINPSPW
    return new PSPJoystick();
#else
    return new SDLJoystick(i);
#endif
#endif
#ifdef USE_ALLEGRO5
    return new Allegro5Joystick(i);
#endif

    /* TODO: support allegro5 joystick */
/*
#ifdef LINUX
    return new LinuxJoystick();
#endif
    return NULL;
*/
    return NULL;
}

Joystick::Joystick(){
}
    
Joystick::~Joystick(){
}
    
const char * Joystick::keyToName(Key key){
    switch (key){
        case Invalid: return "Invalid";
        case Up: return "Up";
        case Down: return "Down";
        case Left: return "Left";
        case Right: return "Right";
        case Button1: return "Button1";
        case Button2: return "Button2";
        case Button3: return "Button3";
        case Button4: return "Button4";
	case Button5: return "Button5";
	case Button6: return "Button6";
	case Start: return "Start";
        case Quit: return "Quit";
    }
    return "Unknown";
}

bool Joystick::pressed() const {
    return events.size() > 0;
}
    
void Joystick::pressButton(int button){
}

void Joystick::releaseButton(int button){
}

void Joystick::axisMotion(int axis, int motion){
}

void Joystick::hatMotion(int motion){
}
    
void Joystick::setCustomButton(int button, Key key){
    Configuration::setCustomButton(key, getDeviceId(), getName(), button);

    customButton[button] = key;

    /* Can only have one unique button/axis */
    if (customAxis.find(key) != customAxis.end()){
        customAxis.erase(key);
    }
}

void Joystick::setCustomAxis(Key key, int stick, int axis, double low, double high){
    Configuration::setCustomAxis(key, getDeviceId(), getName(), stick, axis, low, high);

    Axis & use = customAxis[key];
    use.stick = stick;
    use.axis = axis;
    use.low = low;
    use.high = high;
    use.on = false;

    /* Can only have one unique button/axis */
    if (customButton.find(key) != customButton.end()){
        customButton.erase(key);
    }
}
    
bool Joystick::getAxis(Key key, int & stick, int & axis, double & low, double & high) const {
    std::map<Key, Axis>::const_iterator find = customAxis.find(key);
    if (find != customAxis.end()){
        const Axis & use = find->second;
        stick = use.stick;
        axis = use.axis;
        low = use.low;
        high = use.high;
        return true;
    } else {
        return false;
    }
}
    
void Joystick::addListener(JoystickListener * listener){
    listeners.insert(listener);
}

void Joystick::removeListener(JoystickListener * listener){
    listeners.erase(listener);
}
    
std::set<JoystickListener*> Joystick::listeners;
std::set<JoystickListener*> Joystick::getListeners(){
    return listeners;
}
