#ifndef _paintown_sdl_joystick_h
#define _paintown_sdl_joystick_h

#ifdef USE_SDL

#include <SDL.h>
#include "../joystick.h"
#include "util/pointer.h"

class ButtonMapping;

class SDLJoystick: public Joystick {
public:
    virtual void poll();
    // virtual JoystickInput readAll();
    virtual int getDeviceId() const;
    virtual void pressButton(int button);
    virtual void releaseButton(int button);
    virtual void axisMotion(int axis, int motion);
    virtual void hatMotion(int motion);

    virtual ~SDLJoystick();

    friend class Joystick;
protected:
    /* convert buttons between what paintown wants and what sdl wants */
    int to_native_button(int button);
    int from_native_button(int button);

    SDLJoystick();
    SDL_Joystick * joystick;
    Util::ReferenceCount<ButtonMapping> buttonMapping;
};

#endif

#endif
