#ifdef MINPSPW

#include "joystick.h"

void PSPJoystick::poll(){
    events.clear();
    sceCtrlPeekBufferPositive(&joystick, 1); 
}
    
int PSPJoystick::getDeviceId() const {
    /* FIXME */
    return 0;
}

JoystickInput PSPJoystick::readAll(){
    JoystickInput input;
    if(joystick.Buttons != 0){ 
        if(joystick.Buttons & PSP_CTRL_START){
            //input.button7 = true;
        } else if(joystick.Buttons & PSP_CTRL_SELECT){
            //input.button8 = true;
        } else if(joystick.Buttons & PSP_CTRL_SQUARE){
            input.button1 = true;
        } else if(joystick.Buttons & PSP_CTRL_CROSS){
            input.button2 = true;
        } else if(joystick.Buttons & PSP_CTRL_TRIANGLE){
            input.button3 = true;
        } else if(joystick.Buttons & PSP_CTRL_CIRCLE){
            input.button4 = true;
        } else if(joystick.Buttons & PSP_CTRL_RTRIGGER){
        } else if(joystick.Buttons & PSP_CTRL_LTRIGGER){
        } else if(joystick.Buttons & PSP_CTRL_LEFT){
            input.left = true;
        } else if(joystick.Buttons & PSP_CTRL_RIGHT){
            input.right = true;
        } else if(joystick.Buttons & PSP_CTRL_DOWN){
            input.down = true;
        } else if(joystick.Buttons & PSP_CTRL_UP){
            input.up = true;
        }
    }

    return input;
}

void PSPJoystick::pressButton(int button){
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

Joystick::Event PSPJoystick::getKey(int button){
    /*
    switch (key){
	case (joystick.Buttons & PSP_CTRL_SQUARE):
	    return Button1; break;
	case (joystick.Buttons & PSP_CTRL_CROSS):
	    return Button2; break;
	case (joystick.Buttons & PSP_CTRL_TRIANGLE):
	    return Button3; break;
	case (joystick.Buttons & PSP_CTRL_CIRCLE):
	    return Button4; break;
	default: break;
    }
    
    return Invalid;
    */
}

PSPJoystick::~PSPJoystick(){
    // no cleanup required
}

PSPJoystick::PSPJoystick(){
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
}

#endif
