#ifdef USE_SDL

#include <SDL.h>
#include "joystick.h"
#include "util/debug.h"
#include <string>

using std::string;

class ButtonMapping{
public:
    ButtonMapping(){
    }

    virtual ~ButtonMapping(){
    }

    virtual int toNative(int button) = 0;
    virtual int fromNative(int button) = 0;
    virtual Joystick::Key toKey(int button) = 0;
    virtual Joystick::Key axisMotionToKey(int axis, int motion) = 0;
};

class DefaultButtonMapping: public ButtonMapping {
public:
    int toNative(int button){
        return button;
    }

    int fromNative(int button){
        return button;
    }

    Joystick::Key toKey(int button){
        switch (button){
            case 0: return Joystick::Button1;
            case 1: return Joystick::Button2;
            case 2: return Joystick::Button3;
            case 3: return Joystick::Button4;
            case 4: return Joystick::Quit;
            default: return Joystick::Invalid;
        }
    }

    Joystick::Key axisMotionToKey(int axis, int motion){
        /* FIXME */
        /*
        if (axis == 0){
            if (motion < 0){
                return Joystick::Left;
            } else if (motion > 0){
                return Joystick::Right;
            }
        } else if (axis == 1){
            if (motion < 0){
                return Joystick::Up;
            } else if (motion > 0){
                return Joystick::Down;
            }
        }
        */

        return Joystick::Invalid;
    }
};

/* used when a ps3 controller is plugged into a usb port of a normal pc */
class Playstation3Controller: public ButtonMapping {
public:
    enum Buttons{
        Cross = 14,
        Circle = 13,
        Triangle = 12,
        Square = 15,
        Start = 3,
        Select = 0,
        Up = 4,
        Left = 7,
        Down = 6,
        Right = 5,
        Stick1 = 1,
        Stick2 = 2,
        L2 = 8,
        L1 = 10,
        R2 = 9,
        R1 = 11,
        /* the middle ps3 button */
        Ps3 = 16
    };

    int toNative(int button){
        switch (button){
            case 0: return Square;
            case 1: return Cross;
            case 2: return Circle;
            case 3: return Triangle;
            case 4: return Start;
        }

        return button;
    }

    int fromNative(int button){
        switch (button){
            case Square: return 0;
            case Cross: return 1;
            case Circle: return 2;
            case Triangle: return 3;
            case Start: return Start;
            default: return 5;
        }

        return button;
    }

    Joystick::Key toKey(int button){
        switch (button){
            case Square: return Joystick::Button1;
            case Cross: return Joystick::Button2;
            case Circle: return Joystick::Button3;
            case Triangle: return Joystick::Button4;
            case Start: return Joystick::Quit;
            case Up: return Joystick::Up;
            case Down: return Joystick::Down;
            case Left: return Joystick::Left;
            case Right: return Joystick::Right;
            default: return Joystick::Invalid;
        }
    }
    
    /* TODO */
    Joystick::Key axisMotionToKey(int axis, int motion){
        return Joystick::Invalid;
    }
};

/* used for the ps3 controller with psl1ght's SDL version */
class Ps3Controller: public ButtonMapping {
public:
    enum Buttons{
        Left = 0,
        Down = 1,
        Right = 2,
        Up = 3,
        Select = 7,
        Start = 4,
        Square = 8,
        Cross = 9,
        Circle = 10,
        Triangle = 11
    };

    int toNative(int button){
        switch (button){
            case 0: return Square;
            case 1: return Cross;
            case 2: return Circle;
            case 3: return Triangle;
            case 4: return Start;
        }
        return button;
    }

    int fromNative(int button){
        switch (button){
            case Square: return 0;
            case Cross: return 1;
            case Circle: return 2;
            case Triangle: return 3;
            case Start: return Start;
            default: return 5;
        }
        return button;
    }

    Joystick::Key toKey(int button){
        switch (button){
            case Square: return Joystick::Button1;
            case Cross: return Joystick::Button2;
            case Circle: return Joystick::Button3;
            case Triangle: return Joystick::Button4;
            case Start: return Joystick::Quit;
            case Up: return Joystick::Up;
            case Down: return Joystick::Down;
            case Left: return Joystick::Left;
            case Right: return Joystick::Right;
            default: return Joystick::Invalid;
        }
    }

    /* TODO */
    Joystick::Key axisMotionToKey(int axis, int motion){
        return Joystick::Invalid;
    }
};

ButtonMapping * makeButtonMapping(string name){
#ifdef PS3
    return new Ps3Controller();
#endif
    if (name == "Sony PLAYSTATION(R)3 Controller"){
        return new Playstation3Controller();
    }
    return new DefaultButtonMapping();
}

void SDLJoystick::poll(){
    events.clear();
}

static bool read_button(SDL_Joystick * joystick, int button){
    return SDL_JoystickGetButton(joystick, button);
}

JoystickInput SDLJoystick::readAll(){
    JoystickInput input;
    return input;
    if (joystick){
        int buttons = SDL_JoystickNumButtons(joystick);
        switch (buttons > 5 ? 5 : buttons){
            case 5: input.quit = read_button(joystick, buttonMapping->toNative(4));
            case 4: input.button4 = read_button(joystick, buttonMapping->toNative(3));
            case 3: input.button3 = read_button(joystick, buttonMapping->toNative(2));
            case 2: input.button2 = read_button(joystick, buttonMapping->toNative(1));
            case 1: input.button1 = read_button(joystick, buttonMapping->toNative(0));
            case 0: {
                break;
            }
        }
    }

    int axis = SDL_JoystickNumAxes(joystick);
    if (axis > 0){
        int position = SDL_JoystickGetAxis(joystick, 0);
        if (position < 0){
            input.left = true;
        } else if (position > 0){
            input.right = true;
        }
    }

    if (axis > 1){
        int position = SDL_JoystickGetAxis(joystick, 1);
        if (position < 0){
            input.up = true;
        } else if (position > 0){
            input.down = true;
        }
    }

    int hats = SDL_JoystickNumHats(joystick);
    if (hats > 0){
        int hat = SDL_JoystickGetHat(joystick, 0);
        if ((hat & SDL_HAT_UP) == SDL_HAT_UP){
            input.up = true;
        }
        if ((hat & SDL_HAT_DOWN) == SDL_HAT_DOWN){
            input.down = true;
        }
        if ((hat & SDL_HAT_LEFT) == SDL_HAT_LEFT){
            input.left = true;
        }
        if ((hat & SDL_HAT_RIGHT) == SDL_HAT_RIGHT){
            input.right = true;
        }
        if ((hat & SDL_HAT_RIGHTUP) == SDL_HAT_RIGHTUP){
            input.right = true;
            input.up = true;
        }
        if ((hat & SDL_HAT_RIGHTDOWN) == SDL_HAT_RIGHTDOWN){
            input.right = true;
            input.down = true;
        }
        if ((hat & SDL_HAT_LEFTDOWN) == SDL_HAT_LEFTDOWN){
            input.left = true;
            input.down = true;
        }
        if ((hat & SDL_HAT_LEFTUP) == SDL_HAT_LEFTUP){
            input.left = true;
            input.up = true;
        }
    }

    return input;
}

SDLJoystick::~SDLJoystick(){
    if (joystick){
        SDL_JoystickClose(joystick);
    }
}

SDLJoystick::SDLJoystick():
joystick(NULL){
    if (SDL_NumJoysticks() > 0){
        joystick = SDL_JoystickOpen(0);
        Global::debug(1) << "Opened joystick '" << SDL_JoystickName(0) << "'" << std::endl;
        buttonMapping = makeButtonMapping(SDL_JoystickName(0));
    }
}
    
void SDLJoystick::pressButton(int button){
    // Global::debug(0) << "Pressed button " << button << std::endl;
    if (joystick){
        Key event = buttonMapping->toKey(button);
        if (event != Invalid){
            events.push_back(Event(event, true));
        }
    }
}

void SDLJoystick::releaseButton(int button){
    if (joystick){
        Key event = buttonMapping->toKey(button);
        if (event != Invalid){
            events.push_back(Event(event, false));
        }
    }
}

void SDLJoystick::axisMotion(int axis, int motion){
    if (joystick){
        Key move = buttonMapping->axisMotionToKey(axis, motion);
        if (move != Invalid){
            events.push_back(Event(move, true));
        }
    }
}

int SDLJoystick::getDeviceId() const {
    if (joystick){
        return SDL_JoystickIndex(joystick);
    }

    return -1;
}

#endif
