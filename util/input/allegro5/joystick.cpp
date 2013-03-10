#ifdef USE_ALLEGRO5

#include "../joystick.h"
#include "joystick.h"
#include <allegro5/allegro.h>

Allegro5Joystick::Allegro5Joystick(int id):
id(id){
}

void Allegro5Joystick::poll(){
}

int Allegro5Joystick::getDeviceId() const {
    return id;
}

std::string Allegro5Joystick::getName() const {
    return al_get_joystick_name(al_get_joystick(id));
}

Allegro5Joystick::~Allegro5Joystick(){
}

int Joystick::numberOfJoysticks(){
    return al_get_num_joysticks();
}

#endif
