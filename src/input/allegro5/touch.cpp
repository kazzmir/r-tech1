#ifdef USE_ALLEGRO5

#include "r-tech1/input/touch.h"
#include "r-tech1/pointer.h"
#include "r-tech1/debug.h"
#include <allegro5/allegro.h>

namespace DeviceInput{

class Allegro5Touch: public Touch {
public:
    Allegro5Touch();
    virtual void poll();
    virtual ~Allegro5Touch();

protected:
    ALLEGRO_EVENT_QUEUE * queue;
};

Allegro5Touch::Allegro5Touch():
queue(NULL){
    if (al_install_touch_input()){
        queue = al_create_event_queue();
        if (queue != NULL){
            al_register_event_source(queue, al_get_touch_input_event_source());
        }
    }
}

void Allegro5Touch::poll(){
    if (queue == NULL){
        return;
    }
    ALLEGRO_EVENT event;
    while (al_get_next_event(queue, &event)){
        switch (event.type){
            case ALLEGRO_EVENT_TOUCH_BEGIN: {
                Global::debug(0) << "Touch begin id (" << event.touch.id << ") x (" << (double) event.touch.x << ") y (" << (double) event.touch.y << ")" << std::endl;
                break;
            }
            case ALLEGRO_EVENT_TOUCH_END: {
                Global::debug(0) << "Touch end id (" << event.touch.id << ") x (" << (double) event.touch.x << ") y (" << (double) event.touch.y << ")" << std::endl;
                break;
            }
            case ALLEGRO_EVENT_TOUCH_MOVE: {
                Global::debug(0) << "Touch move id (" << event.touch.id << ") x (" << (double) event.touch.x << ") y (" << (double) event.touch.y << ")" << std::endl;
                break;
            }
            case ALLEGRO_EVENT_TOUCH_CANCEL: {
                Global::debug(0) << "Touch cancel id (" << event.touch.id << ") x (" << (double) event.touch.x << ") y (" << (double) event.touch.y << ")" << std::endl;
                break;
            }
        }
    }
}

Allegro5Touch::~Allegro5Touch(){
    al_destroy_event_queue(queue);
    queue = NULL;
}

Util::ReferenceCount<Touch> getTouchDevice(){
    return Util::ReferenceCount<Touch>(new Allegro5Touch());
}

}

#endif
