#ifdef USE_SDL

#include <SDL.h>
#include "joystick.h"
#include "util/debug.h"
#include <string>
#include <vector>

#include <exception>

using std::string;
using std::vector;
using std::map;

class ButtonMapping{
public:
    ButtonMapping(){
    }

    virtual ~ButtonMapping(){
    }

    virtual int toNative(Joystick::Key key) = 0;
    virtual Joystick::Key toKey(int button) = 0;
    virtual void axisMotionEvents(int axis, int motion, vector<Joystick::Event> & events) = 0;
    virtual void hatMotionEvents(int motion, vector<Joystick::Event> & events) = 0;
};

class DefaultButtonMapping: public ButtonMapping {
public:
    int toNative(Joystick::Key key){
        switch (key){
            case Joystick::Button1: return 0;
            case Joystick::Button2: return 1;
            case Joystick::Button3: return 2;
            case Joystick::Button4: return 3;
            case Joystick::Quit: return 4;
            case Joystick::Button5: return 5;
            case Joystick::Button6: return 6;
            default: return -1;
        }
        return -1;
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
        bool up = false;
        bool down = false;
        bool left = false;
        bool right = false;
        if (axis == 0){
            if (motion < 0){
                left = true;
            } else if (motion > 0){
                right = true;
            }
        } else if (axis == 1){
            if (motion < 0){
                up = true;
            } else if (motion > 0){
                down = true;
            }
        }
        
        events.push_back(Joystick::Event(Joystick::Left, left));
        events.push_back(Joystick::Event(Joystick::Right, right));
        events.push_back(Joystick::Event(Joystick::Down, down));
        events.push_back(Joystick::Event(Joystick::Up, up));
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

    int toNative(Joystick::Key key){
        switch (key){
            case Joystick::Button1: return Square;
            case Joystick::Button2: return Cross;
            case Joystick::Button3: return Circle;
            case Joystick::Button4: return Triangle;
            case Joystick::Start: return Start;
            default: return -1;
        }

        return -1;
    }

    Joystick::Key toKey(int button){
        switch (button){
            case Square: return Joystick::Button1;
            case Cross: return Joystick::Button2;
            case Circle: return Joystick::Button3;
            case Triangle: return Joystick::Button4;
            case L1: return Joystick::Button5;
            case R1: return Joystick::Button6;
            case Start: return Joystick::Start;
            case Select: return Joystick::Quit;
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

    int toNative(Joystick::Key key){
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
    void axisMotionEvents(int axis, int motion, vector<Joystick::Event> & events){
        int tolerance = 10;
        if (axis == 0){
            if (motion == 0){
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

    int toNative(Joystick::Key key){
        switch (key){
            case Joystick::Button1: return Square;
            case Joystick::Button2: return Cross;
            case Joystick::Button3: return Circle;
            case Joystick::Button4: return Triangle;
            case Joystick::Start: return Start;
            default: return -1;
        }
        return -1;
    }

    Joystick::Key toKey(int button){
        switch (button){
            case Square: return Joystick::Button1;
            case Cross: return Joystick::Button2;
            case Circle: return Joystick::Button3;
            case Triangle: return Joystick::Button4;
            case L1: return Joystick::Button5;
            case R1: return Joystick::Button6;
            case Start: return Joystick::Start;
            case Select: return Joystick::Quit;
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
        Select = 10
    };

    int toNative(Joystick::Key key){
        return -1;
    }

    Joystick::Key toKey(int button){
        switch (button){
            case A: return Joystick::Button1;
            case B: return Joystick::Button2;
            case X: return Joystick::Button3;
            case Y: return Joystick::Button4;
            case L1: return Joystick::Button5;
            case R1: return Joystick::Button6;
            case Start: return Joystick::Start;
            case Select: return Joystick::Quit;
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

    int toNative(Joystick::Key key){
        return -1;
    }

    /* FIXME: need a start key */
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

    int toNative(Joystick::Key key){
        return -1;
    }

    Joystick::Key toKey(int button){
        switch (button){
            case A: return Joystick::Button1;
            case B: return Joystick::Button2;
            case X: return Joystick::Button3;
            case Y: return Joystick::Button4;
            case Z: return Joystick::Button5;
            case Start: return Joystick::Start;
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

class IControlPad: public ButtonMapping {
public:
    enum Buttons{
        A = 11,
        B = 13,
        X = 12,
        Y = 10,
        Start = 9,
        Select = 8,
        Up = 0,
        Down = 3,
        Left = 2,
        Right = 1,
        L1 = 4,
        R1 = 14
    };

    int toNative(Joystick::Key key){
        return -1;
    }

    Joystick::Key toKey(int button){
        switch (button){
            case A: return Joystick::Button1;
            case B: return Joystick::Button2;
            case X: return Joystick::Button3;
            case Y: return Joystick::Button4;
            case L1: return Joystick::Button5;
            case R1: return Joystick::Button6;
            case Up: return Joystick::Up;
            case Down: return Joystick::Down;
            case Left: return Joystick::Left;
            case Right: return Joystick::Right;
            case Start: return Joystick::Start;
            case Select: return Joystick::Quit;
        }
        return Joystick::Invalid;
    }
    
    void axisMotionEvents(int axis, int motion, vector<Joystick::Event> & events){
        // printf("axis %d motion %d\n", axis, motion);
    }
    
    virtual void hatMotionEvents(int motion, vector<Joystick::Event> & events){
        /*
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
        */
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
    if (name.find("iControlPad") != string::npos){
        return new IControlPad();
    }
    Global::debug(0) << "Unknown controller '" << name << "'. Using default mapping" << std::endl;
    return new DefaultButtonMapping();
}

void SDLJoystick::poll(){
    events.clear();
}

static bool read_button(SDL_Joystick * joystick, int button){
    return SDL_JoystickGetButton(joystick, button);
}

SDLJoystick::~SDLJoystick(){
    if (joystick){
        SDL_JoystickClose(joystick);
    }
}

#if 0
#include <io/pad.h>
#include <fstream>
void hack(){
    padInfo padinfo;
    int ok = ioPadGetInfo(&padinfo);
    if (ok == 0){
        std::ofstream out("/dev_hdd0/tmp/p.txt");
        out << "PS3 Pad Info" << std::endl;
        out << " max " << padinfo.max << std::endl;
        out << " connected " << padinfo.connected << std::endl;
        out << " status 0 " << (int) padinfo.status[0] << std::endl;
        out << " status 1 " << (int) padinfo.status[1] << std::endl;
        out << " status 2 " << (int) padinfo.status[2] << std::endl;
        out << " status 3 " << (int) padinfo.status[3] << std::endl;
        out << " status 4 " << (int) padinfo.status[4] << std::endl;
        out.close();
    } else {
        Global::debug(0) << "Could not get pad info" << std::endl;
    }
}
#endif

SDLJoystick::SDLJoystick(int id):
joystick(NULL){
    if (SDL_NumJoysticks() > id){
        joystick = SDL_JoystickOpen(id);
        if (joystick == NULL){
            Global::debug(0) << "Could not open joystick at index " << id << std::endl;
        } else {
            Global::debug(1) << "Opened joystick '" << SDL_JoystickName(id) << "'" << std::endl;
        }
        // printf("Opened joystick '%s'\n", SDL_JoystickName(4));
        buttonMapping = makeButtonMapping(SDL_JoystickName(id));

        readCustomButtons();
        readCustomAxes();
    }
}
    
Joystick::Key SDLJoystick::getKey(int button){
    if (customButton.find(button) != customButton.end()){
        return customButton[button];
    }
    return buttonMapping->toKey(button);
}

int SDLJoystick::getButton(Key key){
    for (std::map<int, Key>::iterator it = customButton.begin(); it != customButton.end(); it++){
        if (it->second == key){
            return it->first;
        }
    }

    return buttonMapping->toNative(key);
}
    
void SDLJoystick::pressButton(int button){
    const std::set<JoystickListener*> & listeners = getListeners();
    for (std::set<JoystickListener*>::const_iterator it = listeners.begin(); it != listeners.end(); it++){
        (*it)->pressButton(this, button);
    }

    // Global::debug(0) << "Pressed button " << button << std::endl;
    if (joystick){
        Key event = getKey(button);
        if (event != Invalid){
            events.push_back(Event(event, true));
        }
    }
}

void SDLJoystick::releaseButton(int button){
    const std::set<JoystickListener*> & listeners = getListeners();
    for (std::set<JoystickListener*>::const_iterator it = listeners.begin(); it != listeners.end(); it++){
        (*it)->releaseButton(this, button);
    }

    if (joystick){
        Key event = getKey(button);
        if (event != Invalid){
            events.push_back(Event(event, false));
        }
    }
}

void SDLJoystick::hatMotion(int motion){
    std::set<JoystickListener*> listeners = getListeners();
    for (std::set<JoystickListener*>::iterator it = listeners.begin(); it != listeners.end(); it++){
        (*it)->hatMotion(this, motion);
    }

    // Global::debug(0) << "Hat motion " << motion << std::endl;
    if (joystick){
        buttonMapping->hatMotionEvents(motion, events);
    }
}

void SDLJoystick::axisMotion(int axis, int motion){
    const std::set<JoystickListener*> & listeners = getListeners();
    for (std::set<JoystickListener*>::const_iterator it = listeners.begin(); it != listeners.end(); it++){
        /* Stick is always 0.
         * Motions should always fit inside a short, [-32767, 32767]
         */
        (*it)->axisMotion(this, 0, axis, (double) motion / 32768.0);
    }

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

string SDLJoystick::getName() const {
    if (joystick){
        return SDL_JoystickName(getDeviceId());
    }

    return "";
}

int SDLJoystick::getDeviceId() const {
    if (joystick){
        return SDL_JoystickIndex(joystick);
    }

    return -1;
}
    
std::map<int, std::map<int, double> > SDLJoystick::getCurrentAxisValues() const {
    map<int, map<int, double> > out;

    if (joystick){
        int axis = SDL_JoystickNumAxes(joystick);
        for (int i = 0; i < axis; i++){
            /* always use stick 0 */
            out[0][i] = (double) SDL_JoystickGetAxis(joystick, i) / 32768.0;
        }
    }

    return out;
}

int Joystick::numberOfJoysticks(){
    return SDL_NumJoysticks();
}

#endif
