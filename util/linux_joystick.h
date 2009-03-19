#include "joystick.h"

class LinuxJoystick: public Joystick {
public:
    virtual void poll();

    friend class Joystick;
protected:
    LinuxJoystick();
};
