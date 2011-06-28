#ifdef USE_SDL

#include <SDL.h>
#include "joystick.h"

void SDLJoystick::poll(){
    events.clear();
}

static bool read_button(SDL_Joystick * joystick, int button){
    return SDL_JoystickGetButton(joystick, button);
}

#ifdef PS3
static int to_native_button(int button){
    switch (button){
    	case 0: return 8;
	case 1: return 9;
	case 2: return 10;
	case 3: return 11;
	case 4: return 4;
    }
    return button;
}

static int from_native_button(int button){
    switch (button){
    	case 8: return 0;
	case 9: return 1;
	case 10: return 2;
	case 11: return 3;
	case 4: return 4;
	default: return 5;
    }
    return button;
}
#else
static int to_native_button(int button){
    return button;
}

static int from_native_button(int button){
    return button;
}
#endif

JoystickInput SDLJoystick::readAll(){
    JoystickInput input;
    if (joystick){
        int buttons = SDL_JoystickNumButtons(joystick);
        switch (buttons > 5 ? 5 : buttons){
            case 5: input.quit = read_button(joystick, to_native_button(4));
            case 4: input.button4 = read_button(joystick, to_native_button(3));
            case 3: input.button3 = read_button(joystick, to_native_button(2));
            case 2: input.button2 = read_button(joystick, to_native_button(1));
            case 1: input.button1 = read_button(joystick, to_native_button(0));
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
    }
}
    
void SDLJoystick::pressButton(int button){
    if (joystick){
        Event event = Invalid;
        switch (from_native_button(button)){
            case 0: event = Button1; break;
            case 1: event = Button2; break;
            case 2: event = Button3; break;
            case 3: event = Button4; break;
            case 4: event = Quit; break;
            default: break;
        }
        events.push_back(event);
    }
}

void SDLJoystick::releaseButton(int button){
}

void SDLJoystick::axisMotion(int axis, int motion){
    if (joystick){
        if (axis == 0){
            if (motion < 0){
                events.push_back(Left);
            } else if (motion > 0){
                events.push_back(Right);
            }
        } else if (axis == 1){
            if (motion < 0){
                events.push_back(Up);
            } else if (motion > 0){
                events.push_back(Down);
            }
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
