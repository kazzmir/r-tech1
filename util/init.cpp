#ifdef USE_ALLEGRO
#include <allegro.h>
#ifdef ALLEGRO_WINDOWS
#include <winalleg.h>
#endif
#endif

#ifdef USE_ALLEGRO5
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#endif

#ifdef USE_SDL
#include <SDL.h>
#endif

#ifndef WINDOWS
#include <signal.h>
#include <string.h>
#include <unistd.h>
#endif

#ifdef LINUX
#include <execinfo.h>
#endif

/* don't be a boring tuna */
// #warning you are ugly

#include "globals.h"
#include "init.h"
#include "network/network.h"
#include "thread.h"
#include <time.h>

#include <ostream>
#include "sound/dumb/include/dumb.h"
#ifdef USE_ALLEGRO
#include "sound/dumb/include/aldumb.h"
#include "allegro/loadpng/loadpng.h"
#include "allegro/gif/algif.h"
#endif
#include "graphics/bitmap.h"
#include "funcs.h"
#include "file-system.h"
#include "font.h"
#include "sound/sound.h"
#include "configuration.h"
#include "sound/music.h"
#include "loading.h"
#include "input/keyboard.h"
#include "message-queue.h"

#ifdef WII
#include <fat.h>
#endif

using namespace std;

volatile int Global::speed_counter4 = 0;
bool Global::rateLimit = true;

/* enough seconds for 136 years */
volatile unsigned int Global::second_counter = 0;

/* the original engine was running at 90 ticks per second, but we dont
 * need to render that fast, so TICS_PER_SECOND is really fps and
 * LOGIC_MULTIPLIER will be used to adjust the speed counter to its
 * original value.
 */
int Global::TICS_PER_SECOND = 40;
// const double Global::LOGIC_MULTIPLIER = (double) 90 / (double) Global::TICS_PER_SECOND;

double Global::ticksPerSecond(int ticks){
    return (double) ticks / (double) TICS_PER_SECOND;
}
    
static volatile bool run_timer;
Util::Thread::Lock run_timer_lock;
Util::ThreadBoolean run_timer_guard(run_timer, run_timer_lock);
vector<Util::Thread::Id> running_timers;

#ifdef USE_ALLEGRO
const int Global::WINDOWED = GFX_AUTODETECT_WINDOWED;
const int Global::FULLSCREEN = GFX_AUTODETECT_FULLSCREEN;
#else
/* FIXME: use enums here or something */
const int Global::WINDOWED = 0;
const int Global::FULLSCREEN = 1;
#endif

/* game counter, controls FPS */
static void inc_speed_counter(){
    /* probably put input polling here, InputManager::poll(). no, don't do that.
     * polling is done in the standardLoop now.
     */
    Global::speed_counter4 += 1;
}
#ifdef USE_ALLEGRO
END_OF_FUNCTION(inc_speed_counter)
#endif

/* if you need to count seconds for some reason.. */
static void inc_second_counter() {
    Global::second_counter += 1;
}
#ifdef USE_ALLEGRO
END_OF_FUNCTION(inc_second_counter)
#endif

#if !defined(WINDOWS) && !defined(WII) && !defined(MINPSPW) && !defined(PS3) && !defined(NDS) && !defined(NACL) && !defined(XENON)
#ifdef LINUX
static void print_stack_trace(){
    /* use addr2line on these addresses to get a filename and line number */
    void *trace[128];
    int frames = backtrace(trace, 128);
    printf("Stack trace\n");
    for (int i = 0; i < frames; i++){
        printf(" %p\n", trace[i]);
    }
}
#endif

static void handleSigSegV(int i, siginfo_t * sig, void * data){
    const char * message = "Bug! Caught a memory violation. Shutting down..\n";
    int dont_care = write(1, message, 48);
    dont_care = dont_care;
#ifdef LINUX
    print_stack_trace();
#endif
    // Global::shutdown_message = "Bug! Caught a memory violation. Shutting down..";
    Graphics::setGfxModeText();
#ifdef USE_ALLEGRO
    allegro_exit();
#endif
#ifdef USE_SDL
    SDL_Quit();
#endif
    /* write to a log file or something because sigsegv shouldn't
     * normally happen.
     */
    exit(1);
}
#else
#endif

/* catch a socket being closed prematurely on unix */
#if !defined(WINDOWS) && !defined(WII) && !defined(MINPSPW) && !defined(PS3) && !defined(NDS) && !defined(NACL) && !defined(XENON)
static void handleSigPipe( int i, siginfo_t * sig, void * data ){
}

/*
static void handleSigUsr1( int i, siginfo_t * sig, void * data ){
	pthread_exit( NULL );
}
*/
#endif

static void registerSignals(){
#if !defined(WINDOWS) && !defined(WII) && !defined(MINPSPW) && !defined(PS3) && !defined(NDS) && !defined(NACL) && !defined(XENON)
	struct sigaction action;
	memset( &action, 0, sizeof(struct sigaction) );
	action.sa_sigaction = handleSigPipe;
	sigaction( SIGPIPE, &action, NULL );

	memset( &action, 0, sizeof(struct sigaction) );
	action.sa_sigaction = handleSigSegV;
	sigaction( SIGSEGV, &action, NULL );

	/*
	action.sa_sigaction = handleSigUsr1;
	sigaction( SIGUSR1, &action, NULL );
	*/
#endif
}

/* should probably call the janitor here or something */
static void close_paintown(){
    Music::pause();
    Graphics::setGfxModeText();
#ifdef USE_ALLEGRO
    allegro_exit();
#endif
    exit(0);
}

namespace Global{
    extern int do_shutdown;
}

static void close_window(){
    /* when do_shutdown is 1 the game will attempt to throw ShutdownException
     * wherever it is. If the game is stuck or the code doesn't throw
     * ShutdownException then when the user tries to close the window
     * twice we just forcifully shutdown.
     */
    Global::do_shutdown += 1;
    if (Global::do_shutdown == 2){
        close_paintown();
    }
}
#ifdef USE_ALLEGRO
END_OF_FUNCTION(close_window)
#endif

#ifdef USE_ALLEGRO5
struct TimerInfo{
    TimerInfo(void (*x)(), ALLEGRO_TIMER * y):
        tick(x), timer(y){}

    void (*tick)();
    ALLEGRO_TIMER * timer;
};

static void * do_timer(void * info){
    TimerInfo * timerInfo = (TimerInfo*) info;

    ALLEGRO_EVENT_SOURCE * source = al_get_timer_event_source(timerInfo->timer);
    ALLEGRO_EVENT_QUEUE * queue = al_create_event_queue();
    al_register_event_source(queue, source);
    while (run_timer_guard.get()){
        ALLEGRO_EVENT event;
        al_wait_for_event(queue, &event);
        timerInfo->tick();
    }

    al_destroy_event_queue(queue);
    al_destroy_timer(timerInfo->timer);

    delete timerInfo;
    return NULL;
}

static Util::Thread::Id start_timer(void (*func)(), int frequency){
    ALLEGRO_TIMER * timer = al_create_timer(ALLEGRO_BPS_TO_SECS(frequency));
    if (timer == NULL){
        Global::debug(0) << "Could not create timer" << endl;
    }
    al_start_timer(timer);
    TimerInfo * info = new TimerInfo(func, timer);
    Util::Thread::Id thread;
    Util::Thread::createThread(&thread, NULL, (Util::Thread::ThreadFunction) do_timer, (void*) info);
    return thread;
}

static void startTimers(){
    run_timer_guard.set(true);
    running_timers.push_back(start_timer(inc_speed_counter, Global::TICS_PER_SECOND));
    running_timers.push_back(start_timer(inc_second_counter, 1));
}

static void initSystem(Global::stream_type & out){
    out << "Allegro5 initialize " << (al_init() ? "Ok" : "Failed") << endl;
    uint32_t version = al_get_allegro_version();
    int major = version >> 24;
    int minor = (version >> 16) & 255;
    int revision = (version >> 8) & 255;
    int release = version & 255;
    out << "Allegro5 version " << major << "." << minor << "." << revision << "." << release << endl;
    out << "Init image: " << (al_init_image_addon() ? "Ok" : "Failed") << endl;
    out << "Init primitives " << (al_init_primitives_addon() ? "Ok" : "Failed") << endl;
    out << "Init keyboard " << (al_install_keyboard() ? "Ok" : "Failed") << endl;
    al_set_app_name("Paintown");
}
#endif

#ifdef USE_ALLEGRO
static void startTimers(){
    install_int_ex(inc_speed_counter, BPS_TO_TIMER(Global::TICS_PER_SECOND));
    install_int_ex(inc_second_counter, BPS_TO_TIMER(1));
}

static void initSystem(Global::stream_type & out){
    out << "Allegro version: " << ALLEGRO_VERSION_STR << endl;
    out << "Allegro init: " <<allegro_init()<<endl;
    out << "Install timer: " <<install_timer()<<endl;

    /* png */
    loadpng_init();
    algif_init();

    out<<"Install keyboard: "<<install_keyboard()<<endl;
    /* do we need the mouse?? */
    // out<<"Install mouse: "<<install_mouse()<<endl;
    out<<"Install joystick: "<<install_joystick(JOY_TYPE_AUTODETECT)<<endl;
    /* 16 bit color depth */
    set_color_depth(16);

    LOCK_VARIABLE( speed_counter4 );
    LOCK_VARIABLE( second_counter );
    LOCK_FUNCTION( (void *)inc_speed_counter );
    LOCK_FUNCTION( (void *)inc_second_counter );
    
    /* keep running in the background */
    set_display_switch_mode(SWITCH_BACKGROUND);

    /* close window when the X is pressed */
    LOCK_FUNCTION(close_window);
    set_close_button_callback(close_window);
}

#endif
#ifdef USE_SDL
    
// static pthread_t events;

struct TimerInfo{
    TimerInfo(void (*x)(), int y):
        tick(x), frequency(y){}

    void (*tick)();
    int frequency;
};

static void * do_timer(void * arg){
    TimerInfo info = *(TimerInfo *) arg;
    uint32_t delay = (uint32_t)(1000.0 / (double) info.frequency);

    /* assuming SDL_GetTicks() starts at 0, this should last for about 50 days
     * before overflowing. overflow should work out fine. Assuming activate occurs
     * when the difference between now and ticks is at least 6, the following will happen.
     * ticks      now        now-ticks
     * 4294967294 4294967294 0
     * 4294967294 4294967295 1
     * 4294967294 0          2
     * 4294967294 1          3
     * 4294967294 2          4
     * 4294967294 3          5
     * 4294967294 4          6
     * Activate
     * 3          5          2
     * 3          6          3
     * 3          7          4
     * 3          8          5
     * 3          9          6
     * Activate
     *
     * Can 'now' ever be much larger than 'ticks' due to overflow?
     * It doesn't seem like it.
     */
    uint32_t ticks = SDL_GetTicks();

    while (run_timer_guard.get()){
        uint32_t now = SDL_GetTicks();
        while (now - ticks >= delay){
            // Global::debug(0) << "Tick!" << endl;
            info.tick();
            ticks += delay;
        }
        SDL_Delay(1);
    }

    delete (TimerInfo *) arg;

    return NULL;
}

static Util::Thread::Id start_timer(void (*func)(), int frequency){
    TimerInfo * speed = new TimerInfo(func, frequency);
	/*
    speed.tick = func;
    speed.frequency = frequency;
*/
    Util::Thread::Id thread;
    Util::Thread::createThread(&thread, NULL, (Util::Thread::ThreadFunction) do_timer, (void*) speed);
    return thread;
}

/*
static void doSDLQuit(){
    SDL_Event quit;
    quit.type = SDL_QUIT;
    SDL_PushEvent(&quit);
    Global::debug(0) << "Waiting for SDL event handler to finish" << endl;
    pthread_join(events, NULL);
    SDL_Quit();
}
*/

static void startTimers(){
    run_timer_guard.set(true);
    running_timers.push_back(start_timer(inc_speed_counter, Global::TICS_PER_SECOND));
    running_timers.push_back(start_timer(inc_second_counter, 1));
}
    
static void initSystem(Global::stream_type & out){
#ifdef ANDROID
    /* opengles2 is the default renderer but it doesn't work */
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles");
#endif

    out << "SDL Init: ";
    int ok = SDL_Init(SDL_INIT_VIDEO |
                      SDL_INIT_AUDIO |
                      SDL_INIT_TIMER |
                      SDL_INIT_JOYSTICK |
                      SDL_INIT_NOPARACHUTE);
    if (ok == 0){
        out << "Ok" << endl;
    } else {
	out << "Failed (" << ok << ") - " << SDL_GetError() << endl;
	exit(ok);
    }

    /* Just do SDL thread init
#ifdef MINPSPW
    pthread_init();
#endif
*/

    try{
        SDL_Surface * icon = SDL_LoadBMP(Storage::instance().find(Filesystem::RelativePath("menu/icon.bmp")).path().c_str());
        if (icon != NULL){
            SDL_WM_SetIcon(icon, NULL);
        }
    } catch (const Filesystem::NotFound & failed){
        Global::debug(0) << "Could not find window icon: " << failed.getTrace() << endl;
    }

    SDL_WM_SetCaption("Paintown", NULL);

    SDL_EnableUNICODE(1);
    SDL_JoystickEventState(1);

    atexit(SDL_Quit);
    // atexit(doSDLQuit);
}
#endif

/* mostly used for testing purposes */
bool Global::initNoGraphics(){
    /* copy/pasting the init code isn't ideal, maybe fix it later */
    Global::stream_type & out = Global::debug(0);
    out << "-- BEGIN init --" << endl;
    out << "Data path is " << Util::getDataPath2().path() << endl;
    out << "Paintown version " << Global::getVersionString() << endl;
    out << "Build date " << __DATE__ << " " << __TIME__ << endl;

#ifdef WII
    /* <WinterMute> fatInitDefault will set working dir to argv[0] passed by launcher,
     * or root of first device mounted
     */
    out << "Fat init " << (fatInitDefault() == true ? "Ok" : "Failed") << endl;
#endif
    /*
    char buffer[512];
    if (getcwd(buffer, 512) != 0){
        printf("Working directory '%s'\n", buffer);
    }
    */

    if (!Storage::instance().exists(Util::getDataPath2())){
        Global::debug(0) << "Cannot find data path '" << Util::getDataPath2().path() << "'! Either use the -d switch to specify the data directory or find the data directory and move it to that path" << endl;
        return false;
    }

    /* do implementation specific setup */
    initSystem(out);

    dumb_register_stdfiles();
    
    // Sound::initialize();

    // Filesystem::initialize();

    /*
    Graphics::SCALE_X = GFX_X;
    Graphics::SCALE_Y = GFX_Y;
    */

    Configuration::loadConfigurations();
    const int sx = Configuration::getScreenWidth();
    const int sy = Configuration::getScreenHeight();
    Graphics::Bitmap::setFakeGraphicsMode(sx, sy);
       
    /* music */
    atexit(&dumb_exit);

    out << "Initialize random number generator" << endl;
    /* initialize random number generator */
    srand(time(NULL));

    registerSignals();

#ifdef HAVE_NETWORKING
    out << "Initialize network" << endl;
    Network::init();
    atexit(Network::closeAll);
#endif

    /* this mutex is used to show the loading screen while the game loads */
    // Util::Thread::initializeLock(&Loader::loading_screen_mutex);

    out << "-- END init --" << endl;

    return true;
}

static void closeTimers(){
    run_timer_guard.set(false);
    for (vector<Util::Thread::Id>::iterator it = running_timers.begin(); it != running_timers.end(); it++){
        Util::Thread::Id timer = *it;
        Util::Thread::joinThread(timer);
    }
    running_timers.clear();
}

void Global::close(){
    closeTimers();
}

#ifdef PS3
extern "C" int SDL_JoystickInit();
static void ps3JoystickHack(){
    /* FIXME: hack for the ps3. at the start of the program only 1 joystick is enabled
     * even if more than 1 is connected, so we force another call to JoystickInit
     * to pick up all joysticks.
     */
    SDL_JoystickInit();
}
#endif

#if defined(USE_SDL) && defined(MACOSX)
#include <CoreFoundation/CoreFoundation.h>
#endif

static void maybeSetWorkingDirectory(){
#if defined(USE_SDL) && defined(MACOSX)
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    if (mainBundle != NULL){
        CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
        char path[PATH_MAX];
        if (CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX)){
            chdir(path);
        } else {
            Global::debug(0) << "Could not set working directory to Resources" << std::endl;
        }
        CFRelease(resourcesURL);
    }
#endif
}

/* All xenon stuff goes here */
#ifdef XENON

#ifdef DEBUG
#include <network/network.h>
#include <threads/gdb.h>
#endif

#include <threads/threads.h>
#include <xenos/xenos.h>
#include <diskio/ata.h>
#include <libfat/fat.h>
#include <xenon_sound/sound.h>

static void xenon_init(){
    xenos_init(VIDEO_MODE_AUTO);
    console_init();
    xenon_make_it_faster(XENON_SPEED_FULL);
    usb_init();
    usb_do_poll();
    xenon_ata_init();
    xenon_atapi_init();
    fatInitDefault();
    xenon_sound_init();
    threading_init();

#ifdef DEBUG
    network_init();
    gdb_init();
#endif
}
#endif

bool Global::init(int gfx){
    /* Can xenon_init be moved lower? Probably.. */
#ifdef XENON
    xenon_init();
#endif

    Global::stream_type & out = Global::debug(0);
    out << "-- BEGIN init --" << endl;
    out << "Data path is " << Util::getDataPath2().path() << endl;
    out << "Paintown version " << Global::getVersionString() << endl;
    out << "Build date " << __DATE__ << " " << __TIME__ << endl;

    maybeSetWorkingDirectory();


#ifdef WII
    /* <WinterMute> fatInitDefault will set working dir to argv[0] passed by launcher,
     * or root of first device mounted
     */
    out << "Fat init " << (fatInitDefault() == 0 ? "Ok" : "Failed") << endl;
#endif
    /*
    char buffer[512];
    if (getcwd(buffer, 512) != 0){
        printf("Working directory '%s'\n", buffer);
    }
    */

#ifndef NACL
    /* do implementation specific setup */
    initSystem(out);
#endif

    dumb_register_stdfiles();
    
    Sound::initialize();

    // Filesystem::initialize();

    /*
    Graphics::SCALE_X = GFX_X;
    Graphics::SCALE_Y = GFX_Y;
    */

    Configuration::loadConfigurations();
    const int sx = Configuration::getScreenWidth();
    const int sy = Configuration::getScreenHeight();
    if (gfx == -1){
        gfx = Configuration::getFullscreen() ? Global::FULLSCREEN : Global::WINDOWED;
    } else {
        Configuration::setFullscreen(gfx == Global::FULLSCREEN);
    }
    
    /* set up the screen */
    int gfxCode = Graphics::setGraphicsMode(gfx, sx, sy);
    if (gfxCode == 0){
        out << "Set graphics mode: Ok" << endl;
    } else {
        out << "Set graphics mode: Failed! (" << gfxCode << ")" << endl;
        return false;
    }
    
    /* music */
    atexit(&dumb_exit);

    out << "Initialize random number generator" << endl;
    /* initialize random number generator */
    srand(time(NULL));

    registerSignals();

#ifdef HAVE_NETWORKING
    out << "Initialize network" << endl;
    Network::init();
    atexit(Network::closeAll);
#endif

    /* this mutex is used to show the loading screen while the game loads */
    Util::Thread::initializeLock(&Global::messageLock);

    Util::Thread::initializeLock(&run_timer_lock);
    run_timer = true;
    Global::TICS_PER_SECOND = Configuration::getFps();
    startTimers();

    out << "-- END init --" << endl;

    /*
    const Font & font = Font::getDefaultFont();
    // font.setSize(30, 30);
    Bitmap temp(font.textLength("Loading") + 1, font.getHeight("Loading") + 1);
    font.printf(0, 0, Bitmap::makeColor(255, 255, 255), temp, "Loading", 0);
    temp.BlitToScreen(sx / 2, sy / 2);
    */
    Graphics::Bitmap white(*Graphics::getScreenBuffer());
    /* for nacl which takes a while to run exists(), we just want
     * to show some progress
     */
    white.fill(Graphics::makeColor(128, 128, 128));
    white.BlitToScreen();
    if (!Storage::instance().exists(Util::getDataPath2())){
        white.fill(Graphics::makeColor(255, 0, 0));
        white.BlitToScreen();
        Global::debug(0) << "Cannot find data path '" << Util::getDataPath2().path() << "'! Either use the -d switch to specify the data directory or find the data directory and move it to that path" << endl;
        Util::restSeconds(1);
        return false;
    } else {
        white.fill(Graphics::makeColor(255, 255, 255));
        white.BlitToScreen();
    }

#ifdef PS3
    // ps3JoystickHack();
#endif

    return true;
}

/* Restarts the timers */
void Global::setTicksPerSecond(int ticks){
    if (ticks < 1){
        ticks = 1;
    }
    if (ticks > 90){
        ticks = 90;
    }
    if (ticks != TICS_PER_SECOND){
        TICS_PER_SECOND = ticks;
        closeTimers();
        startTimers();
    }
}

