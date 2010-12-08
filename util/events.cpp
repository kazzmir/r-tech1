#ifdef USE_SDL
#include <SDL.h>
#endif
#include "bitmap.h"
#include "events.h"
#include "exceptions/shutdown_exception.h"
#include "configuration.h"
#include "debug.h"
#include "funcs.h"
#include "thread.h"
#include "input/keyboard.h"
#include "input/joystick.h"
#include "input/input-manager.h"

namespace Util{

EventManager::EventManager():
bufferKeys(false){
}

#ifdef USE_SDL
static void handleKeyDown(Keyboard & keyboard, const SDL_Event & event){
    keyboard.press(event.key.keysym.sym, event.key.keysym.unicode);
}

static void handleKeyUp(Keyboard & keyboard, const SDL_Event & event){
    keyboard.release(event.key.keysym.sym);
}

static void handleJoystickButtonUp(Joystick * joystick, const SDL_Event & event){
    int device = event.jbutton.which;
    int button = event.jbutton.button;
    if (device == joystick->getDeviceId()){
        joystick->releaseButton(button);
    }
}

static void handleJoystickButtonDown(Joystick * joystick, const SDL_Event & event){
    int device = event.jbutton.which;
    int button = event.jbutton.button;
    if (device == joystick->getDeviceId()){
        joystick->pressButton(button);
    }
}

static void handleJoystickAxis(Joystick * joystick, const SDL_Event & event){
    int device = event.jaxis.which;
    int axis = event.jaxis.axis;
    int value = event.jaxis.value;
    if (device == joystick->getDeviceId()){
        joystick->axisMotion(axis, value);
    }
}

void EventManager::runSDL(Keyboard & keyboard, Joystick * joystick){
    keyboard.poll();
    if (joystick){
        joystick->poll();
    }
    SDL_Event event;
    while (SDL_PollEvent(&event) == 1){
        switch (event.type){
            case SDL_QUIT : {
                dispatch(CloseWindow);
                break;
            }
            case SDL_KEYDOWN : {
                handleKeyDown(keyboard, event);
                 // dispatch(Key, event.key.keysym.sym);
                 break;
            }
            case SDL_KEYUP : {
                handleKeyUp(keyboard, event);
                break;
            }
            case SDL_JOYBUTTONDOWN: {
                if (joystick != NULL){
                    handleJoystickButtonDown(joystick, event);
                }
                break;
            }
            case SDL_JOYBUTTONUP: {
                if (joystick != NULL){
                    handleJoystickButtonUp(joystick, event);
                }
                break;
            }
            case SDL_JOYAXISMOTION: {
                if (joystick != NULL){
                    handleJoystickAxis(joystick, event);
                }
                break;
            }
            case SDL_VIDEORESIZE : {
                int width = event.resize.w;
                int height = event.resize.h;
                /* to keep the perspective correct
                 * 640/480 = 1.33333
                 */
                if (width > height){
                    height = (int)((double) width / 1.3333333333);
                } else {
                    width = (int)((double) height * 1.3333333333);
                }
                dispatch(ResizeScreen, width, height);
                break;
            }
            default : {
                break;
            }
        }
    }
}
#endif

#ifdef USE_ALLEGRO
void EventManager::runAllegro(Keyboard & keyboard, Joystick * joystick){
    keyboard.poll();
}
#endif

void EventManager::run(Keyboard & keyboard, Joystick * joystick){
#ifdef USE_SDL
    runSDL(keyboard, joystick);
#elif USE_ALLEGRO
    runAllegro(keyboard, joystick);
#endif
}

/* kill the program if the user requests */
void EventManager::waitForThread(WaitThread & thread){
    // Keyboard dummy;
    while (!thread.isRunning()){
        try{
            /* input manager will run the event manager */
            InputManager::poll();
            // run(dummy);
        } catch (const ShutdownException & death){
            thread.kill();
            throw death;
        }
        Util::rest(10);
    }
}

EventManager::~EventManager(){
}
    
void EventManager::enableKeyBuffer(){
    bufferKeys = true;
}

void EventManager::disableKeyBuffer(){
    bufferKeys = false;
}

void EventManager::dispatch(Event type, int arg1){
    switch (type){
        case Key : {
            if (bufferKeys){
                keys.push_back(KeyType(arg1));
            }
            break;
        }
        default : {
            break;
        }
    }
}

void EventManager::dispatch(Event type, int arg1, int arg2){
    switch (type){
        case ResizeScreen : {
            Global::debug(1) << "Resizing screen to " << arg1 << ", " << arg2 << std::endl;
            if (Bitmap::setGraphicsMode(0, arg1, arg2) == 0){
                Configuration::setScreenWidth(arg1);
                Configuration::setScreenHeight(arg2);
            }
            break;
        }
        default : break;
    }
}

void EventManager::dispatch(Event type){
    switch (type){
        case CloseWindow : {
            throw ShutdownException();
        }
        default : break;
    }
}

}
