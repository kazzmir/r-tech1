#ifdef USE_SDL
#include <SDL.h>
#endif
#include "bitmap.h"
#include "events.h"
#include "exceptions/shutdown_exception.h"
#include "configuration.h"
#include "globals.h"
#include "funcs.h"
#include "thread.h"
#include "input/keyboard.h"

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

void EventManager::runSDL(Keyboard & keyboard){
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

void EventManager::runAllegro(Keyboard & keyboard){
    keyboard.poll();
}

void EventManager::run(Keyboard & keyboard){
#ifdef USE_SDL
    runSDL(keyboard);
#elif USE_ALLEGRO
    runAllegro(keyboard);
#endif
}

/* kill the program if the user requests */
void EventManager::waitForThread(WaitThread & thread){
    Keyboard dummy;
    while (!thread.isRunning()){
        try{
            run(dummy);
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
