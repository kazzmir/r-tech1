#ifdef USE_ALLEGRO5

#include "r-tech1/input/touch.h"
#include "r-tech1/pointer.h"
#include "r-tech1/debug.h"
#include <allegro5/allegro.h>
#include <map>

namespace DeviceInput{

class TouchTrack{
public:
    TouchTrack(int id, double x, double y):
    id(id),
    x(x),
    y(y),
    last_x(x),
    last_y(y){
    }

    void update(double x, double y){
        this->last_x = this->x;
        this->last_y = this->y;
        this->x = x;
        this->y = y;
    }

    int id;
    double x;
    double y;
    double last_x;
    double last_y;
};

class Allegro5Touch: public Touch {
public:
    Allegro5Touch();
    virtual void poll();
    virtual ~Allegro5Touch();

    void handleTouch(const Util::ReferenceCount<TouchTrack> & touch, std::map<Key, bool> & buttons, bool press);
    void generateEvents(std::map<Key, bool> & new_buttons);

    bool inZone(Key key, double x, double y);

protected:
    ALLEGRO_EVENT_QUEUE * queue;
    std::map<int, Util::ReferenceCount<TouchTrack> > touches;
    std::map<Key, bool> buttons;
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
    
bool Allegro5Touch::inZone(Key key, double x, double y){
    _xdebug << "Check for zone " << key << " in " << x << ", " << y << std::endl;
    switch (key){
        case Up: {
            return
                x >= 0 && x <= 20 &&
                y >= 0 && y <= 20;
        }
        case Down: {
            return
                x >= 0 && x <= 20 &&
                y >= 20 && y <= 40;
        }
        default: {
            return false;
        }
    }
    return false;
}

void Allegro5Touch::handleTouch(const Util::ReferenceCount<TouchTrack> & touch, std::map<Key, bool> & new_buttons, bool press){
    if (inZone(Up, touch->x, touch->y)){
        new_buttons[Up] = press;
    }
    if (inZone(Down, touch->x, touch->y)){
        new_buttons[Down] = press;
    }
}

/* Generate a 'pressed' event if the current state was not pressd and the new state is pressed.
 * Similarly, generate a 'release' event if the current state was pressed and the new state was not pressed
 *
 */
void Allegro5Touch::generateEvents(std::map<Key, bool> & new_buttons){
    if (buttons[Up] ^ new_buttons[Up]){
        events.push_back(Event(Up, new_buttons[Up]));
    }
}

void Allegro5Touch::poll(){
    if (queue == NULL){
        return;
    }
    events.clear();
    std::map<Key, bool> new_buttons;
    ALLEGRO_EVENT event;
    while (al_get_next_event(queue, &event)){
        switch (event.type){
            case ALLEGRO_EVENT_TOUCH_BEGIN: {
                // Global::debug(0) << "Touch begin id (" << event.touch.id << ") x (" << (double) event.touch.x << ") y (" << (double) event.touch.y << ")" << std::endl;
                int id = event.touch.id;
                double x = event.touch.x;
                double y = event.touch.y;

                touches[id] = Util::ReferenceCount<TouchTrack>(new TouchTrack(id, x, y));
                handleTouch(touches[id], new_buttons, true);

                break;
            }
            case ALLEGRO_EVENT_TOUCH_END: {
                int id = event.touch.id;
                double x = event.touch.x;
                double y = event.touch.y;

                handleTouch(touches[id], new_buttons, false);
                touches[id] = NULL;

                // Global::debug(0) << "Touch end id (" << event.touch.id << ") x (" << (double) event.touch.x << ") y (" << (double) event.touch.y << ")" << std::endl;
                break;
            }
            case ALLEGRO_EVENT_TOUCH_MOVE: {
                int id = event.touch.id;
                double x = event.touch.x;
                double y = event.touch.y;

                if (touches[id] != NULL){
                    touches[id]->update(x, y);
                } else {
                    touches[id] = Util::ReferenceCount<TouchTrack>(new TouchTrack(id, x, y));
                }
                
                handleTouch(touches[id], new_buttons, true);

                // Global::debug(0) << "Touch move id (" << event.touch.id << ") x (" << (double) event.touch.x << ") y (" << (double) event.touch.y << ")" << std::endl;
                break;
            }
            case ALLEGRO_EVENT_TOUCH_CANCEL: {
                // Global::debug(0) << "Touch cancel id (" << event.touch.id << ") x (" << (double) event.touch.x << ") y (" << (double) event.touch.y << ")" << std::endl;
                int id = event.touch.id;
                handleTouch(touches[id], new_buttons, false);
                touches[id] = NULL;
                break;
            }
        }
    }

    generateEvents(new_buttons);
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
