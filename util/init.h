#ifndef _paintown_init_h
#define _paintown_init_h

/* global vars */

#define GFX_X 640
#define GFX_Y 480

namespace Global{
    extern volatile int speed_counter4;
    extern volatile unsigned int second_counter;

    // extern const double LOGIC_MULTIPLIER;
    extern int TICS_PER_SECOND;

    extern const int WINDOWED;
    extern const int FULLSCREEN;

    extern bool rateLimit;

    /* pass WINDOWED or FULLSCREEN in. FIXME: replace with an enum */
    bool init(int gfx);
    void close();
    bool initNoGraphics();

    /* Returns a consistent number of ticks per second regardless of what
     * TICS_PER_SECOND is.
     */
    double ticksPerSecond(int ticks);

    /* Updates TICS_PER_SECOND */
    void setTicksPerSecond(int ticks);
}

/*
#define GFX_X 320
#define GFX_Y 240
*/

#endif
