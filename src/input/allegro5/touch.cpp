#ifdef USE_ALLEGRO5

#include "r-tech1/input/touch.h"
#include "r-tech1/pointer.h"
#include <allegro5/allegro.h>

namespace Input{

class Allegro5Touch: public Touch {
public:
    Allegro5Touch();
    virtual ~Allegro5Touch();

protected:
    ALLEGRO_EVENT_QUEUE * queue;
};

Allegro5Touch::Allegro5Touch():
queue(NULL){
    queue = al_create_event_queue();
    al_register_event_source(queue, al_get_touch_input_event_source());
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
