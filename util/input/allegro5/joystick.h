#ifndef _paintown_allegro5_joystick_h
#define _paintown_allegro5_joystick_h

#include "../joystick.h"
#include <string>

class Allegro5Joystick: public Joystick {
public:
    virtual void poll();
    virtual int getDeviceId() const;
    virtual std::string getName() const;

    virtual ~Allegro5Joystick();

    friend class Joystick;
protected:
    Allegro5Joystick(int id);
    int id;
};

#endif
