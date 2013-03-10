#ifndef _paintown_allegro5_joystick_h
#define _paintown_allegro5_joystick_h

#include "../joystick.h"
#include "util/pointer.h"
#include <string>

struct ALLEGRO_EVENT_QUEUE;

class ButtonMapping;
class Allegro5Joystick: public Joystick {
public:
    virtual void poll();
    virtual int getDeviceId() const;
    virtual std::string getName() const;

    virtual ~Allegro5Joystick();

    friend class Joystick;

protected:
    void axis(int stick, int axis, float position);
    void buttonDown(int button);
    void buttonUp(int button);

    Allegro5Joystick(int id);
    int id;
    ALLEGRO_EVENT_QUEUE * queue;
    Util::ReferenceCount<ButtonMapping> buttons;
};

#endif
