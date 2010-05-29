#ifdef USE_SDL
#include <SDL.h>
#endif
#include "events.h"
#include "shutdown_exception.h"

namespace Util{

EventManager::EventManager(){
}

#ifdef USE_SDL
void EventManager::runSDL(){
    SDL_Event event;
    if (SDL_PollEvent(&event) == 1){
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
