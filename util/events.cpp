#ifdef USE_SDL
#include <SDL.h>
#endif
#ifdef USE_ALLEGRO5
#include <allegro5/allegro.h>
#endif
#include <vector>
#include "bitmap.h"
#include "events.h"
#include "exceptions/shutdown_exception.h"
#include "configuration.h"
#include "debug.h"
#include "funcs.h"
#include "thread.h"
#include "init.h"
#include "parameter.h"
#include "input/keyboard.h"
#include "input/joystick.h"
#include "input/input-manager.h"
#include "input/input-source.h"

namespace Util{

EventManager::EventManager():
bufferKeys(false){
#ifdef USE_ALLEGRO5
    queue = al_create_event_queue();
    if (al_is_keyboard_installed()){
        al_register_event_source(queue, al_get_keyboard_event_source());
    }
    if (Graphics::the_display != NULL){
        al_register_event_source(queue, al_get_display_event_source(Graphics::the_display));
    }
#endif
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

static void handleJoystickHat(Joystick * joystick, const SDL_Event & event){
    int device = event.jhat.which;
    int motion = event.jhat.value;
    if (device == joystick->getDeviceId()){
        joystick->hatMotion(motion);
    }

#if 0
    /* should up/down control left/right -- flip these values? */
#if WII
    const int axis_up_down = 0;
    const int axis_left_right = 1;
    const int up = 1;
    const int down = -1;
    const int left = -1;
    const int right = 1;
#else
    const int axis_up_down = 1;
    const int axis_left_right = 0;
    const int up = -1;
    const int down = 1;
    const int left = -1;
    const int right = 1;
#endif

    switch (motion){
        case SDL_HAT_CENTERED: break;
        case SDL_HAT_UP: joystick->axisMotion(axis_up_down, up); break;
        case SDL_HAT_DOWN: joystick->axisMotion(axis_up_down, down); break;
        case SDL_HAT_RIGHT: joystick->axisMotion(axis_left_right, right); break;
        case SDL_HAT_LEFT: joystick->axisMotion(axis_left_right, left); break;
        default: break;
    }
#endif
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
            case SDL_JOYHATMOTION : {
                if (joystick != NULL){
                    handleJoystickHat(joystick, event);
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
                double ratio = (double) GFX_X / (double) GFX_Y;
                if (width > height){
                    height = (int)((double) width / ratio);
                } else {
                    width = (int)((double) height * ratio);
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

#ifdef USE_ALLEGRO5
static void handleKeyDown(Keyboard & keyboard, const ALLEGRO_EVENT & event){
    keyboard.press(event.keyboard.keycode, event.keyboard.unichar);
}

static void handleKeyUp(Keyboard & keyboard, const ALLEGRO_EVENT & event){
    keyboard.release(event.keyboard.keycode);
}

static void handleResize(const ALLEGRO_EVENT & event){
    double width = event.display.width;
    double height = event.display.height;
    if (width < GFX_X){
        width = GFX_X;
    }
    if (height < GFX_Y){
        height = GFX_Y;
    }
    /* to keep the perspective correct
     * 640/480 = 1.33333
     */
    double ratio = (double) GFX_X / (double) GFX_Y;
    if (width > height){
        height = width / ratio;
    } else {
        width = height * ratio;
    }
    
    ALLEGRO_DISPLAY * display = event.display.source;
    al_acknowledge_resize(display);
    al_resize_display(display, (int) width, (int) height);
    ALLEGRO_TRANSFORM transformation;
    al_identity_transform(&transformation);
    // al_scale_transform(&transformation, (double) al_get_display_width(display) / (double) GFX_X, (double) al_get_display_height(display) / (double) GFX_Y);
    al_scale_transform(&transformation, (double) width / (double) GFX_X, (double) height / (double) GFX_Y);
    al_set_target_bitmap(Graphics::getScreenBuffer().getData()->getBitmap());
    al_use_transform(&transformation);
}

void EventManager::runAllegro5(Keyboard & keyboard, Joystick * joystick){
    keyboard.poll();

    ALLEGRO_EVENT event;
    while (al_get_next_event(queue, &event)){
        switch (event.type){
            /*
            case ALLEGRO_EVENT_KEY_DOWN: {
                Global::debug(0) << "Key down " << event.keyboard.keycode << std::endl;
                handleKeyDown(keyboard, event);
                break;
            }
            */
            case ALLEGRO_EVENT_DISPLAY_RESIZE: {
                handleResize(event);
                break;
            }
            case ALLEGRO_EVENT_KEY_UP: {
                handleKeyUp(keyboard, event);
                break;
            }
            case ALLEGRO_EVENT_KEY_CHAR : {
                // Global::debug(0) << "Key char " << event.keyboard.keycode << " unicode " << event.keyboard.unichar << std::endl;
                handleKeyDown(keyboard, event);
                break;
            }
        }
    }

    /*
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
            case SDL_JOYHATMOTION : {
                if (joystick != NULL){
                    handleJoystickHat(joystick, event);
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
                / * to keep the perspective correct
                 * 640/480 = 1.33333
                 * /
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
    */
}
#endif

void EventManager::run(Keyboard & keyboard, Joystick * joystick){
#ifdef USE_SDL
    runSDL(keyboard, joystick);
#elif USE_ALLEGRO
    runAllegro(keyboard, joystick);
#elif USE_ALLEGRO5
    runAllegro5(keyboard, joystick);
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
#ifdef USE_ALLEGRO5
    al_destroy_event_queue(queue);
#endif
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
            if (Graphics::setGraphicsMode(0, arg1, arg2) == 0){
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

class LoopDone: public std::exception {
public:
    LoopDone(){
    }

    ~LoopDone() throw () {
    }
};

Logic::~Logic(){
}

Draw::Draw():
frames(0),
second_counter(Global::second_counter),
fps(0){
}

void Draw::drawFirst(const Graphics::Bitmap & screen){
}

Draw::~Draw(){
}
    
double Draw::getFps() const {
    return fps;
}

void Draw::updateFrames(){
    if (second_counter != Global::second_counter){
        int difference = Global::second_counter - second_counter;
        double alpha = 0.2;
        /* unlikely, but just in case */
        if (difference == 0){
            difference = 1;
        }
        fps = (alpha * fps) + ((1 - alpha) * (double) frames / difference);
        // fps[fps_index] = (double) frames / (double) difference;
        // fps_index = (fps_index+1) % max_fps_index;
        second_counter = Global::second_counter;
        frames = 0;
    }

    frames += 1;
}

static void changeScreenMode(){
    Configuration::setFullscreen(!Configuration::getFullscreen());
    int gfx = (Configuration::getFullscreen() ? Global::FULLSCREEN : Global::WINDOWED);
    Graphics::setGraphicsMode(gfx, Global::getScreenWidth(), Global::getScreenHeight());
}

static void checkFullscreen(){
    InputMap<int> input;
    input.set(Keyboard::Key_F11, 0, true, 5);
    std::vector<InputMap<int>::InputEvent> events = InputManager::getEvents(input, InputSource());

    for (std::vector<InputMap<int>::InputEvent>::iterator it = events.begin(); it != events.end(); it++){
        InputMap<int>::InputEvent event = *it;

        if (!event.enabled){
            continue;
        }

        if (event.out == 5){
            changeScreenMode();
        }
    }
}

static void doStandardLoop(Logic & logic, Draw & draw){
    const Graphics::Bitmap & screen = *Graphics::screenParameter.current();
    draw.drawFirst(screen);
    Global::speed_counter4 = 0;
    double runCounter = 0;
    try{
        while (!logic.done()){
            if (Global::speed_counter4 > 0){
                // Global::debug(0) << "Speed counter " << Global::speed_counter4 << std::endl;
                runCounter += logic.ticks(Global::speed_counter4);
                Global::speed_counter4 = 0;
                bool need_draw = false;
                while (runCounter >= 1.0){
                    need_draw = true;
                    InputManager::poll();
                    checkFullscreen();
                    runCounter -= 1;
                    logic.run();

                    if (Global::shutdown()){
                        throw ShutdownException();
                    }

                    if (logic.done()){
                        /* quit the loop immediately */
                        throw LoopDone();
                    }
                }

                if (need_draw){
                    draw.updateFrames();
                    draw.draw(screen);
                }
            }

            while (Global::speed_counter4 == 0){
                /* if the fps is limited then don't keep redrawing */
                if (Global::rateLimit){
                    rest(1);
                } else {
                    draw.updateFrames();
                    draw.draw(screen);
                }
            }
        }
    } catch (const LoopDone & done){
    }
}

void standardLoop(Logic & logic, Draw & draw){
    /* if a screen already exists (because we have nested standardLoops) then
     * leave this parameter alone, otherwise set a new parameter.
     */
    /*
    if (Parameter<Graphics::Bitmap*>::current() == NULL){
        doStandardLoop(logic, draw);
    } else {
        doStandardLoop(logic, draw);
    }
    */
    doStandardLoop(logic, draw);
}

}
