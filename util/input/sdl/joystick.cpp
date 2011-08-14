#ifdef USE_SDL

#include <SDL.h>
#include "joystick.h"
#include "util/debug.h"
#include <string>
#include <vector>

using std::string;
using std::vector;

class ButtonMapping{
public:
    ButtonMapping(){
    }

    virtual ~ButtonMapping(){
    }

    virtual int toNative(int button) = 0;
    virtual int fromNative(int button) = 0;
    virtual Joystick::Key toKey(int button) = 0;
    virtual void axisMotionEvents(int axis, int motion, vector<Joystick::Event> & events) = 0;
    virtual void hatMotionEvents(int motion, vector<Joystick::Event> & events) = 0;
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
            case 5: return Joystick::Button5;
            case 6: return Joystick::Button6;
            default: return Joystick::Invalid;
        }
    }
    
    virtual void hatMotionEvents(int motion, vector<Joystick::Event> & events){
        bool up = false;
        bool down = false;
        bool left = false;
        bool right = false;
        switch (motion){
            case SDL_HAT_CENTERED: break;
            case SDL_HAT_UP: up = true; break;
            case SDL_HAT_RIGHT: right = true; break;
            case SDL_HAT_DOWN: down = true; break;
            case SDL_HAT_LEFT: left = true; break;
            case SDL_HAT_RIGHTUP: right = true; up = true; break;
            case SDL_HAT_RIGHTDOWN: right = true; down = true; break;
            case SDL_HAT_LEFTUP: left = true; up = true; break;
            case SDL_HAT_LEFTDOWN: left = true; down = true; break;
        }

        events.push_back(Joystick::Event(Joystick::Left, left));
        events.push_back(Joystick::Event(Joystick::Right, right));
        events.push_back(Joystick::Event(Joystick::Down, down));
        events.push_back(Joystick::Event(Joystick::Up, up));
    }

    void axisMotionEvents(int axis, int motion, vector<Joystick::Event> & events){
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
            case L1: return Joystick::Button5;
            case R1: return Joystick::Button6;
            case Start: return Joystick::Quit;
            case Up: return Joystick::Up;
            case Down: return Joystick::Down;
            case Left: return Joystick::Left;
            case Right: return Joystick::Right;
            default: return Joystick::Invalid;
        }
    }
    
    /* TODO */
    void axisMotionEvents(int axis, int motion, vector<Joystick::Event> & events){
    }

    virtual void hatMotionEvents(int motion, vector<Joystick::Event> & events){
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
            case Start: return Joystick::Quit;
        }
        return Joystick::Invalid;
    }
    
    /* axis 1. negative up, positive down
     * axis 0, negative left, positive right
     */
    void axisMotionEvents(int axis, int motion, vector<Joystick::Event> & events){
        int tolerance = 10;
        if (axis == 0){
            if (motion < -tolerance){
                events.push_back(Joystick::Event(Joystick::Left, true));
            } else if (motion > tolerance){
                events.push_back(Joystick::Event(Joystick::Right, true));
            } else {
                /* fake a release for left and right */
                events.push_back(Joystick::Event(Joystick::Left, false));
                events.push_back(Joystick::Event(Joystick::Right, false));
            }
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
    }
    
    virtual void hatMotionEvents(int motion, vector<Joystick::Event> & events){
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
        Triangle = 11,
        L1 = 13,
        R1 = 12,
        L2 = 15,
        R2 = 14,
        L3 = 6,
        R3 = 5
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
            case L1: return Joystick::Button5;
            case R1: return Joystick::Button6;
            case Start: return Joystick::Quit;
            case Up: return Joystick::Up;
            case Down: return Joystick::Down;
            case Left: return Joystick::Left;
            case Right: return Joystick::Right;
            default: return Joystick::Invalid;
        }
    }

    /* TODO */
    void axisMotionEvents(int axis, int motion, vector<Joystick::Event> & events){
    }
    
    virtual void hatMotionEvents(int motion, vector<Joystick::Event> & events){
    }
};

class XBox360Controller: public ButtonMapping {
public:
    enum Buttons{
        A = 0,
        B = 1,
        X = 2,
        Y = 3,
        L1 = 4,
        R1 = 5,
        Start = 6,
        Xbox = 7,
        L3 = 8,
        R3 = 9,
        select = 10
    };

    int toNative(int button){
        return 0;
    }

    int fromNative(int button){
    	return 0;
    }
    
    Joystick::Key toKey(int button){
        switch (button){
            case A: return Joystick::Button1;
            case B: return Joystick::Button2;
            case X: return Joystick::Button3;
            case Y: return Joystick::Button4;
            case L1: return Joystick::Button5;
            case R1: return Joystick::Button6;
            case Start: return Joystick::Quit;
        }
        return Joystick::Invalid;
    }
    
    void axisMotionEvents(int axis, int motion, vector<Joystick::Event> & events){
    /* axis 6 and 7 are the hats. sdl passes them as hat events */
    #if 0
        if (axis == 6){
            if (motion < 0){
                events.push_back(Joystick::Event(Joystick::Left, true));
            } else if (motion > 0){
                events.push_back(Joystick::Event(Joystick::Right, true));
            } else if (motion == 0){
                /* fake a release for left and right */
                events.push_back(Joystick::Event(Joystick::Left, false));
                events.push_back(Joystick::Event(Joystick::Right, false));
            }
        } else if (axis == 7){
            if (motion < 0){
                events.push_back(Joystick::Event(Joystick::Up, true));
            } else if (motion > 0){
                events.push_back(Joystick::Event(Joystick::Down, true));
            } else if (motion == 0){
                events.push_back(Joystick::Event(Joystick::Up, false));
                events.push_back(Joystick::Event(Joystick::Down, false));
            }
        }
        #endif
    }
    
    virtual void hatMotionEvents(int motion, vector<Joystick::Event> & events){
        bool up = false;
        bool down = false;
        bool left = false;
        bool right = false;
        switch (motion){
            case SDL_HAT_CENTERED: break;
            case SDL_HAT_UP: up = true; break;
            case SDL_HAT_RIGHT: right = true; break;
            case SDL_HAT_DOWN: down = true; break;
            case SDL_HAT_LEFT: left = true; break;
            case SDL_HAT_RIGHTUP: right = true; up = true; break;
            case SDL_HAT_RIGHTDOWN: right = true; down = true; break;
            case SDL_HAT_LEFTUP: left = true; up = true; break;
            case SDL_HAT_LEFTDOWN: left = true; down = true; break;
        }

        events.push_back(Joystick::Event(Joystick::Left, left));
        events.push_back(Joystick::Event(Joystick::Right, right));
        events.push_back(Joystick::Event(Joystick::Down, down));
        events.push_back(Joystick::Event(Joystick::Up, up));
    }
};

class Wiimote: public ButtonMapping {
public:
    enum Buttons{
        A = 0,
        B = 1,
        Button1 = 2,
        Button2 = 3,
        Minus = 4,
        Plus = 5,
        Home = 6
    };

    int toNative(int button){
        return 0;
    }

    int fromNative(int button){
    	return 0;
    }
    
    Joystick::Key toKey(int button){
        switch (button){
            case A: return Joystick::Button1;
            case B: return Joystick::Button2;
            case Button1: return Joystick::Button3;
            case Button2: return Joystick::Button4;
            case Minus: return Joystick::Button5;
            case Plus: return Joystick::Button6;
            case Home: return Joystick::Quit;
        }
        return Joystick::Invalid;
    }
    
    void axisMotionEvents(int axis, int motion, vector<Joystick::Event> & events){
    }
    
    virtual void hatMotionEvents(int motion, vector<Joystick::Event> & events){
        /* rotate all the directions 90 degrees */
        bool up = false; // right
        bool down = false; // left
        bool left = false; // up
        bool right = false; // down
        switch (motion){
            case SDL_HAT_CENTERED: break;
            case SDL_HAT_UP: up = true; break;
            case SDL_HAT_RIGHT: right = true; break;
            case SDL_HAT_DOWN: down = true; break;
            case SDL_HAT_LEFT: left = true; break;
            case SDL_HAT_RIGHTUP: up = true; right = true; break;
            case SDL_HAT_RIGHTDOWN: down = true; right = true; break;
            case SDL_HAT_LEFTUP: up = true; left = true; break;
            case SDL_HAT_LEFTDOWN: down = true; left = true; break;
        }

        events.push_back(Joystick::Event(Joystick::Left, left));
        events.push_back(Joystick::Event(Joystick::Right, right));
        events.push_back(Joystick::Event(Joystick::Down, down));
        events.push_back(Joystick::Event(Joystick::Up, up));
    }
};

class GamecubePad: public ButtonMapping {
public:
    enum Buttons{
        A = 0,
        B = 1,
        X = 2,
        Y = 3,
        Z = 4,
        Start = 7
    };

    int toNative(int button){
        return 0;
    }

    int fromNative(int button){
    	return 0;
    }
    
    Joystick::Key toKey(int button){
        switch (button){
            case A: return Joystick::Button1;
            case B: return Joystick::Button2;
            case X: return Joystick::Button3;
            case Y: return Joystick::Button4;
            case Z: return Joystick::Button5;
            case Start: return Joystick::Quit;
        }
        return Joystick::Invalid;
    }
    
    void axisMotionEvents(int axis, int motion, vector<Joystick::Event> & events){
        // printf("axis %d motion %d\n", axis, motion);
    }
    
    virtual void hatMotionEvents(int motion, vector<Joystick::Event> & events){
        bool up = false;
        bool down = false;
        bool left = false;
        bool right = false;
        switch (motion){
            case SDL_HAT_CENTERED: break;
            case SDL_HAT_UP: up = true; break;
            case SDL_HAT_RIGHT: right = true; break;
            case SDL_HAT_DOWN: down = true; break;
            case SDL_HAT_LEFT: left = true; break;
            case SDL_HAT_RIGHTUP: right = true; up = true; break;
            case SDL_HAT_RIGHTDOWN: right = true; down = true; break;
            case SDL_HAT_LEFTUP: left = true; up = true; break;
            case SDL_HAT_LEFTDOWN: left = true; down = true; break;
        }

        events.push_back(Joystick::Event(Joystick::Left, left));
        events.push_back(Joystick::Event(Joystick::Right, right));
        events.push_back(Joystick::Event(Joystick::Down, down));
        events.push_back(Joystick::Event(Joystick::Up, up));
    }

};

ButtonMapping * makeButtonMapping(string name){
#ifdef PS3
    return new Ps3Controller();
#endif
    if (name == "Sony PLAYSTATION(R)3 Controller"){
        return new Playstation3Controller();
    }
    if (name.find("Logitech(R) Precision(TM) Gamepad") != string::npos){
        return new LogitechPrecision();
    }
    if (name == "Microsoft X-Box 360 pad"){
    	return new XBox360Controller();
    }
    if (name.find("Wiimote") != string::npos){
        return new Wiimote();
    }
    if (name.find("Gamecube") != string::npos){
        return new GamecubePad();
    }
    return new DefaultButtonMapping();
}

void SDLJoystick::poll(){
    events.clear();
}

static bool read_button(SDL_Joystick * joystick, int button){
    return SDL_JoystickGetButton(joystick, button);
}

/*
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
*/

SDLJoystick::~SDLJoystick(){
    if (joystick){
        SDL_JoystickClose(joystick);
    }
}

SDLJoystick::SDLJoystick():
joystick(NULL){
    /* TODO: don't always open joystick 0, try to support all of them */
    if (SDL_NumJoysticks() > 0){
        Global::debug(1) << "Opened joystick '" << SDL_JoystickName(0) << "'" << std::endl;
        joystick = SDL_JoystickOpen(0);
        // printf("Opened joystick '%s'\n", SDL_JoystickName(4));
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

void SDLJoystick::hatMotion(int motion){
    if (joystick){
        buttonMapping->hatMotionEvents(motion, events);
    }
}

void SDLJoystick::axisMotion(int axis, int motion){
    // Global::debug(0) << "Axis motion on " << axis << " motion " << motion << std::endl;
    if (joystick){
        buttonMapping->axisMotionEvents(axis, motion, events);
        /*
        Event move = buttonMapping->axisMotionToEvent(axis, motion);
        if (move.key != Invalid){
            events.push_back(move);
        }
        */
    }
}

int SDLJoystick::getDeviceId() const {
    if (joystick){
        return SDL_JoystickIndex(joystick);
    }

    return -1;
}

#endif
