#ifdef USE_SDL
#include <SDL.h>
#endif
#include "events.h"
#include "exceptions/shutdown_exception.h"
#include "thread.h"
#include "funcs.h"

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

void EventManager::dispatch(Event type){
    switch (type){
        case CloseWindow : {
            throw ShutdownException();
        }
    }
}

}
