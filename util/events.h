#ifndef _paintown_events_h
#define _paintown_events_h

/* handles global events from the system such as
 *   window manager events (press X button)
 *   keyboard/mouse/joystick input (for some backends like SDL)
 */

#include <vector>

#ifdef USE_SDL
#include <SDL.h>
#endif

namespace Util{

class WaitThread;

class EventManager{
public:
    EventManager();
    virtual void run();
    virtual void waitForThread(WaitThread & thread);
    virtual ~EventManager();

#ifdef USE_SDL
    typedef SDLKey KeyType;
#else
    typedef int KeyType;
#endif

    inline const std::vector<KeyType> & getBufferedKeys() const {
        return keys;
    }
    
    void enableKeyBuffer();
    void disableKeyBuffer();

private:
    enum Event{
        CloseWindow,
        ResizeScreen,
        Key
    };

    virtual void dispatch(Event type, int arg1, int arg2);
    virtual void dispatch(Event type, int arg1);
    virtual void dispatch(Event type);

#ifdef USE_SDL
    virtual void runSDL();
#endif

    std::vector<KeyType> keys;
    bool bufferKeys;
};

}

#endif
