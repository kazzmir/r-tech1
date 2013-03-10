#ifdef USE_ALLEGRO5

#include "../joystick.h"
#include "joystick.h"
#include "util/debug.h"
#include <allegro5/allegro.h>

Allegro5Joystick::Allegro5Joystick(int id):
id(id){
    queue = al_create_event_queue();
    if (al_is_joystick_installed()){
        al_register_event_source(queue, al_get_joystick_event_source());
    }
}
    
void Allegro5Joystick::axis(int stick, int axis, float position){
    Global::debug(0) << "stick " << stick << " axis " << axis << " position " << position << std::endl;
}
    
void Allegro5Joystick::buttonDown(int button){
    Global::debug(0) << "Button down " << button << std::endl;
}

void Allegro5Joystick::buttonUp(int button){
    Global::debug(0) << "Button up " << button << std::endl;
}

void Allegro5Joystick::poll(){
    events.clear();
    ALLEGRO_EVENT event;
    while (al_get_next_event(queue, &event)){
        switch (event.type){
            case ALLEGRO_EVENT_JOYSTICK_AXIS: {
                if (event.joystick.id == al_get_joystick(id)){
                    axis(event.joystick.stick, event.joystick.axis, event.joystick.pos);
                }
                break;
            }
            case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN: {
                if (event.joystick.id == al_get_joystick(id)){
                    buttonDown(event.joystick.button);
                }
                break;
            }
            case ALLEGRO_EVENT_JOYSTICK_BUTTON_UP: {
                if (event.joystick.id == al_get_joystick(id)){
                    buttonUp(event.joystick.button);
                }
                break;
            }
        }
    }
}

int Allegro5Joystick::getDeviceId() const {
    return id;
}

std::string Allegro5Joystick::getName() const {
    return al_get_joystick_name(al_get_joystick(id));
}

Allegro5Joystick::~Allegro5Joystick(){
    al_destroy_event_queue(queue);
    queue = NULL;
}

int Joystick::numberOfJoysticks(){
    return al_get_num_joysticks();
}

#endif
