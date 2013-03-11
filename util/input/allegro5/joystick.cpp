#ifdef USE_ALLEGRO5

#include "../joystick.h"
#include "joystick.h"
#include "util/debug.h"
#include <allegro5/allegro.h>

using std::vector;
using std::string;

class ButtonMapping{
public:
    ButtonMapping(){
    }

    virtual ~ButtonMapping(){
    }

    virtual Joystick::Key toKey(int button) = 0;
    virtual void axisMotionEvents(int stick, int axis, float position, vector<Joystick::Event> & events) = 0;
    virtual void hatMotionEvents(int motion, vector<Joystick::Event> & events) = 0;
};

class DefaultMapping: public ButtonMapping {
public:
    DefaultMapping(){
    }
    
    virtual Joystick::Key toKey(int button){
        switch (button){
            case 0: return Joystick::Button1;
            case 1: return Joystick::Button2;
            case 2: return Joystick::Button3;
            case 3: return Joystick::Button4;
            case 4: return Joystick::Button5;
            case 5: return Joystick::Button6;
            case 6: return Joystick::Start;
            case 7: return Joystick::Quit;
            case 8: return Joystick::Up;
            case 9: return Joystick::Down;
            case 10: return Joystick::Left;
            case 11: return Joystick::Right;
        }
        return Joystick::Invalid;
    }
    
    virtual void axisMotionEvents(int stick, int axis, float position, vector<Joystick::Event> & events){
    }

    virtual void hatMotionEvents(int motion, vector<Joystick::Event> & events){
    }

    virtual ~DefaultMapping(){
    }
};

class LogitechPrecision: public ButtonMapping {
public:
    enum Buttons{
        Button1 = 0,
        Button2 = 1,
        Button3 = 2,
        Button4 = 3,
        Start = 8,
        Select = 9,
        R2 = 7,
        R1 = 5,
        L2 = 6,
        L1 = 4
    };

    int toNative(int button){
        return -1;
    }

    int fromNative(int button){
        return -1;
    }
    
    Joystick::Key toKey(int button){
        switch (button){
            case Button1: return Joystick::Button1;
            case Button2: return Joystick::Button2;
            case Button3: return Joystick::Button3;
            case Button4: return Joystick::Button4;
            case L1: return Joystick::Button5;
            case R1: return Joystick::Button6;
            case Start: return Joystick::Start;
            case Select: return Joystick::Quit;
        }
        return Joystick::Invalid;
    }
    
    /* axis 1. negative up, positive down
     * axis 0, negative left, positive right
     */
    void axisMotionEvents(int stick, int axis, float position, vector<Joystick::Event> & events){
        int tolerance = 10;

        const int LeftRightAxis = 0;
        const int UpDownAxis = 1;
        switch (stick){
            case 0: {
                switch (axis){
                    case UpDownAxis: {
                        if (position == -1){
                            events.push_back(Joystick::Event(Joystick::Up, true));
                            events.push_back(Joystick::Event(Joystick::Down, false));
                        } else if (position == 1){
                            events.push_back(Joystick::Event(Joystick::Up, false));
                            events.push_back(Joystick::Event(Joystick::Down, true));
                        } else if (position == 0){
                            events.push_back(Joystick::Event(Joystick::Up, false));
                            events.push_back(Joystick::Event(Joystick::Down, false));
                        }
                        break;
                    }
                    case LeftRightAxis: {
                        if (position == -1){
                            events.push_back(Joystick::Event(Joystick::Left, true));
                            events.push_back(Joystick::Event(Joystick::Right, false));
                        } else if (position == 1){
                            events.push_back(Joystick::Event(Joystick::Left, false));
                            events.push_back(Joystick::Event(Joystick::Right, true));
                        } else if (position == 0){
                            events.push_back(Joystick::Event(Joystick::Left, false));
                            events.push_back(Joystick::Event(Joystick::Right, false));
                        }
                        break;
                    }
                    default: {
                    }
                }
            }
            default: {
                break;
            }
        }
#if 0
        if (axis == 0){
            if (position == 0){
                events.push_back(Joystick::Event(Joystick::Left, false));
                events.push_back(Joystick::Event(Joystick::Right, false));
            } else if (motion == -32768){
                events.push_back(Joystick::Event(Joystick::Left, true));
            } else if (motion == 32767){
                events.push_back(Joystick::Event(Joystick::Right, true));
            } else if (motion == 128){
                events.push_back(Joystick::Event(Joystick::Up, false));
                events.push_back(Joystick::Event(Joystick::Down, false));
            } else if (motion == 1){
                events.push_back(Joystick::Event(Joystick::Up, true));
            } else if (motion == 255){
                events.push_back(Joystick::Event(Joystick::Down, true));
            }
            /*
            if (motion < -tolerance){
                events.push_back(Joystick::Event(Joystick::Left, true));
            } else if (motion > tolerance){
                events.push_back(Joystick::Event(Joystick::Right, true));
            } else {
                / * fake a release for left and right * /
                events.push_back(Joystick::Event(Joystick::Left, false));
                events.push_back(Joystick::Event(Joystick::Right, false));
            }
            */
        } else if (axis == 1){
            if (motion < -tolerance){
                events.push_back(Joystick::Event(Joystick::Up, true));
            } else if (motion > tolerance){
                events.push_back(Joystick::Event(Joystick::Down, true));
            } else {
                events.push_back(Joystick::Event(Joystick::Up, false));
                events.push_back(Joystick::Event(Joystick::Down, false));
            }
        }
#endif
    }
    
    virtual void hatMotionEvents(int motion, vector<Joystick::Event> & events){
    }
};

static Util::ReferenceCount<ButtonMapping> createMapping(ALLEGRO_JOYSTICK * joystick){
    string name = al_get_joystick_name(joystick);
    if (name.find("Logitech(R) Precision(TM) Gamepad") != string::npos){
        return Util::ReferenceCount<ButtonMapping>(new LogitechPrecision());
    }
    return Util::ReferenceCount<ButtonMapping>(new DefaultMapping());
}

Allegro5Joystick::Allegro5Joystick(int id):
id(id){
    queue = al_create_event_queue();
    if (al_is_joystick_installed()){
        al_register_event_source(queue, al_get_joystick_event_source());
    }
    buttons = createMapping(al_get_joystick(id));
}
    
void Allegro5Joystick::axis(int stick, int axis, float position){
    Global::debug(0) << "stick " << stick << " axis " << axis << " position " << position << std::endl;
        
    buttons->axisMotionEvents(stick, axis, position, events);
}
    
void Allegro5Joystick::buttonDown(int button){
    Global::debug(0) << "Button down " << button << std::endl;

    Key event = buttons->toKey(button);
    if (event != Invalid){
        events.push_back(Event(event, false));
    }
}

void Allegro5Joystick::buttonUp(int button){
    Global::debug(0) << "Button up " << button << std::endl;

    Key event = buttons->toKey(button);
    if (event != Invalid){
        events.push_back(Event(event, true));
    }
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
