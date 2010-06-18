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

namespace Util{

EventManager::EventManager(){
}

#ifdef USE_SDL
void EventManager::runSDL(){
    SDL_Event event;
    while (SDL_PollEvent(&event) == 1){
        switch (event.type){
            case SDL_QUIT : {
                dispatch(CloseWindow);
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

void EventManager::run(){
#ifdef USE_SDL
    runSDL();
#endif
}

/* kill the program if the user requests */
void EventManager::waitForThread(WaitThread & thread){
    while (!thread.isRunning()){
        try{
            run();
        } catch (const ShutdownException & death){
            thread.kill();
            throw death;
        }
        Util::rest(10);
    }
}

EventManager::~EventManager(){
}

void EventManager::dispatch(Event type, int arg1, int arg2){
    switch (type){
        case ResizeScreen : {
            Global::debug(0) << "Resizing screen to " << arg1 << ", " << arg2 << std::endl;
            if (Bitmap::setGraphicsMode(0, arg1, arg2)){
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
