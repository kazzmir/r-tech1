#ifndef _paintown_joystick
#define _paintown_joystick

class Joystick{
public:
    virtual void poll() = 0;

    static Joystick * create();

protected:

    Joystick();
};

#endif
