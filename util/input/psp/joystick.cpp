#ifdef MINPSPW

// Assumes SDL
#include <SDL.h>

#include "joystick.h"
#include "util/debug.h"

void PSPJoystick::poll(){
    events.clear();
    sceCtrlPeekBufferPositive(&joystick, 1);
    doKeys();
}
    
int PSPJoystick::getDeviceId() const {
    /* Only one available on psp, return that */
    return 0;
}

JoystickInput PSPJoystick::readAll(){
    return buffer;
}

void PSPJoystick::pressButton(int button){
    /* NOTE This can easily be done in doKeys instead of feeding it back into SDL to run this */
    Event event = Invalid;
    switch (button){
	case 0: event = Button1; break;
	case 1: event = Button2; break;
	case 2: event = Button3; break;
	case 3: event = Button4; break;
	case 4: event = Quit; break;
    }
    events.push_back(event);
}

void PSPJoystick::releaseButton(int button){
}

void PSPJoystick::axisMotion(int axis, int motion){
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

void PSPJoystick::doKeys(){
    if(joystick.Buttons != 0){ 
	
	/*
	if(!(buffer.button7) && (joystick.Buttons & PSP_CTRL_START)){
            
        } else if((buffer.button7) && !(joystick.Buttons & PSP_CTRL_START)){
            
        } */
        
        if(!(buffer.quit) && (joystick.Buttons & PSP_CTRL_SELECT)){
            buffer.quit = true;
	    // Create SDL joy button event
	    SDL_Event event;
	    // Default to 0
	    event.jbutton.which = 0;
	    event.jbutton.button = 4;
	    event.jbutton.type = SDL_JOYBUTTONDOWN;
	    event.jbutton.state = SDL_PRESSED;
	    SDL_PushEvent(&event);
        } else if((buffer.quit) && !(joystick.Buttons & PSP_CTRL_SELECT)){
            buffer.quit = false;
	    // Create SDL joy button event
	    SDL_Event event;
	    // Default to 0
	    event.jbutton.which = 0;
	    event.jbutton.button = 4;
	    event.jbutton.type = SDL_JOYBUTTONUP;
	    event.jbutton.state = SDL_RELEASED;
	    SDL_PushEvent(&event);
        } 
        
        if(!(buffer.button1) && (joystick.Buttons & PSP_CTRL_SQUARE)){
            buffer.button1 = true;
	    // Create SDL joy button event
	    SDL_Event event;
	    // Default to 0
	    event.jbutton.which = 0;
	    event.jbutton.button = 0;
	    event.jbutton.type = SDL_JOYBUTTONDOWN;
	    event.jbutton.state = SDL_PRESSED;
	    SDL_PushEvent(&event);
        } else if((buffer.button1) && !(joystick.Buttons & PSP_CTRL_SQUARE)){
            buffer.button1 = false;
	    // Create SDL joy button event
	    SDL_Event event;
	    // Default to 0
	    event.jbutton.which = 0;
	    event.jbutton.button = 0;
	    event.jbutton.type = SDL_JOYBUTTONUP;
	    event.jbutton.state = SDL_RELEASED;
	    SDL_PushEvent(&event);
        }
        
        if(!(buffer.button2) && (joystick.Buttons & PSP_CTRL_CROSS)){
            buffer.button2 = true;
	    // Create SDL joy button event
	    SDL_Event event;
	    // Default to 0
	    event.jbutton.which = 0;
	    event.jbutton.button = 1;
	    event.jbutton.type = SDL_JOYBUTTONDOWN;
	    event.jbutton.state = SDL_PRESSED;
	    SDL_PushEvent(&event);
        } else if((buffer.button2) && !(joystick.Buttons & PSP_CTRL_CROSS)){
            buffer.button2 = false;
	    // Create SDL joy button event
	    SDL_Event event;
	    // Default to 0
	    event.jbutton.which = 0;
	    event.jbutton.button = 1;
	    event.jbutton.type = SDL_JOYBUTTONUP;
	    event.jbutton.state = SDL_RELEASED;
	    SDL_PushEvent(&event);
        }
        
        if(!(buffer.button3) && (joystick.Buttons & PSP_CTRL_TRIANGLE)){
            buffer.button3 = true;
	    // Create SDL joy button event
	    SDL_Event event;
	    // Default to 0
	    event.jbutton.which = 0;
	    event.jbutton.button = 2;
	    event.jbutton.type = SDL_JOYBUTTONDOWN;
	    event.jbutton.state = SDL_PRESSED;
	    SDL_PushEvent(&event);
        } else if((buffer.button3) && !(joystick.Buttons & PSP_CTRL_TRIANGLE)){
            buffer.button3 = false;
	    // Create SDL joy button event
	    SDL_Event event;
	    // Default to 0
	    event.jbutton.which = 0;
	    event.jbutton.button = 2;
	    event.jbutton.type = SDL_JOYBUTTONUP;
	    event.jbutton.state = SDL_RELEASED;
	    SDL_PushEvent(&event);
        }
        
        if(!(buffer.button4) && (joystick.Buttons & PSP_CTRL_CIRCLE)){
            buffer.button4 = true;
	    // Create SDL joy button event
	    SDL_Event event;
	    // Default to 0
	    event.jbutton.which = 0;
	    event.jbutton.button = 3;
	    event.jbutton.type = SDL_JOYBUTTONDOWN;
	    event.jbutton.state = SDL_PRESSED;
	    SDL_PushEvent(&event);
        } else if((buffer.button4) && !(joystick.Buttons & PSP_CTRL_CIRCLE)){
            buffer.button4 = false;
	    // Create SDL joy button event
	    SDL_Event event;
	    // Default to 0
	    event.jbutton.which = 0;
	    event.jbutton.button = 3;
	    event.jbutton.type = SDL_JOYBUTTONUP;
	    event.jbutton.state = SDL_RELEASED;
	    SDL_PushEvent(&event);
        }
        /*
        if(!(buffer.button5) && (joystick.Buttons & PSP_CTRL_RTRIGGER)){
        } else if((buffer.button5) && !(joystick.Buttons & PSP_CTRL_RTRIGGER)){
        }
        
        if(!(buffer.button6) && (joystick.Buttons & PSP_CTRL_LTRIGGER)){
        } else if((buffer.button6) && !(joystick.Buttons & PSP_CTRL_LTRIGGER)){
        }
        */
	
        if(!(buffer.left) && (joystick.Buttons & PSP_CTRL_LEFT)){
            buffer.left = true;
	    events.push_back(Left);
#if 0
	    // Create SDL joy button event
	    SDL_Event event;
	    // Default to 0
	    event.jaxis.which = 0;
	    event.jaxis.type = SDL_JOYAXISMOTION;
	    event.jaxis.axis = 0;
	    event.jaxis.value = -1;
	    SDL_PushEvent(&event);
#endif
        } else if((buffer.left) && !(joystick.Buttons & PSP_CTRL_LEFT)){
            buffer.left = false;
#if 0
	    // Create SDL joy button event
	    SDL_Event event;
	    // Default to 0
	    event.jaxis.which = 0;
	    event.jaxis.type = SDL_JOYAXISMOTION;
	    event.jaxis.axis = 0;
	    event.jaxis.value = 0;
	    SDL_PushEvent(&event);
#endif   
        }
        
        if(!(buffer.right) && (joystick.Buttons & PSP_CTRL_RIGHT)){
            buffer.right = true;
	    events.push_back(Right);
#if 0
	    // Create SDL joy button event
	    SDL_Event event;
	    // Default to 0
	    event.jaxis.which = 0;
	    event.jaxis.type = SDL_JOYAXISMOTION;
	    event.jaxis.axis = 0;
	    event.jaxis.value = 1;
	    SDL_PushEvent(&event);
#endif
        } else if((buffer.right) && !(joystick.Buttons & PSP_CTRL_RIGHT)){
            buffer.right = false;
#if 0
	    // Create SDL joy button event
	    SDL_Event event;
	    // Default to 0
	    event.jaxis.which = 0;
	    event.jaxis.type = SDL_JOYAXISMOTION;
	    event.jaxis.axis = 0;
	    event.jaxis.value = 0;
	    SDL_PushEvent(&event);
#endif
        }
        
        if(!(buffer.down) && (joystick.Buttons & PSP_CTRL_DOWN)){
            buffer.down = true;
	    events.push_back(Down);
#if 0
	    // Create SDL joy button event
	    SDL_Event event;
	    // Default to 0
	    event.jaxis.which = 0;
	    event.jaxis.type = SDL_JOYAXISMOTION;
	    event.jaxis.axis = 1;
	    event.jaxis.value = -1;
	    SDL_PushEvent(&event);
#endif
        } else if((buffer.down) && !(joystick.Buttons & PSP_CTRL_DOWN)){
            buffer.down = false;
#if 0
	    // Create SDL joy button event
	    SDL_Event event;
	    // Default to 0
	    event.jaxis.which = 0;
	    event.jaxis.type = SDL_JOYAXISMOTION;
	    event.jaxis.axis = 1;
	    event.jaxis.value = 0;
	    SDL_PushEvent(&event);
#endif
        }
        
        if(!(buffer.up) && (joystick.Buttons & PSP_CTRL_UP)){
            buffer.up = true;
	    events.push_back(Up);
#if 0
	    // Create SDL joy button event
	    SDL_Event event;
	    // Default to 0
	    event.jaxis.which = 0;
	    event.jaxis.type = SDL_JOYAXISMOTION;
	    event.jaxis.axis = 1;
	    event.jaxis.value = 1;
	    SDL_PushEvent(&event);
#endif
        } else if((buffer.up) && !(joystick.Buttons & PSP_CTRL_UP)){
            buffer.up = false;
#if 0
	    // Create SDL joy button event
	    SDL_Event event;
	    // Default to 0
	    event.jaxis.which = 0;
	    event.jaxis.type = SDL_JOYAXISMOTION;
	    event.jaxis.axis = 1;
	    event.jaxis.value = 0;
	    SDL_PushEvent(&event);
#endif
        }
        
    }
}

PSPJoystick::~PSPJoystick(){
    // no cleanup required
}

PSPJoystick::PSPJoystick(){
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
}

#endif
