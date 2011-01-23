#ifndef _paintown_psp_joystick_h
#define _paintown_psp_joystick_h

#ifdef MINPSPW

#include "../joystick.h"

#include <pspctrl.h>

class PSPJoystick: public Joystick {
public:
    virtual void poll();
    virtual JoystickInput readAll();
    virtual int getDeviceId() const;
    virtual void pressButton(int button);
    virtual void releaseButton(int button);
    virtual void axisMotion(int axis, int motion);

    virtual ~PSPJoystick();

    friend class Joystick;
protected:
    
    Joystick::Event getKey(int button);
    PSPJoystick();
    // joystick
    SceCtrlData joystick;
};

#endif

#endif
