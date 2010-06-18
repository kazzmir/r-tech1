#ifndef _paintown_events_h
#define _paintown_events_h

/* handles global events from the system such as
 *   window manager events (press X button)
 *   keyboard/mouse/joystick input (for some backends like SDL)
 */

namespace Util{

class WaitThread;

class EventManager{
public:
    EventManager();
    virtual void run();
    virtual void waitForThread(WaitThread & thread);
    virtual ~EventManager();

private:
    enum Event{
        CloseWindow,
        ResizeScreen
    };

    void dispatch(Event type, int arg1, int arg2);
    virtual void dispatch(Event type);

#ifdef USE_SDL
    virtual void runSDL();
#endif
};

}

#endif
