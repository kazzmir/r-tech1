#ifndef _paintown_sound_h
#define _paintown_sound_h

#include <string>
#include "load_exception.h"

#ifdef USE_SDL
#include "sdl/sound.h"
#endif
#ifdef USE_ALLEGRO
#include "allegro/sound.h"
#endif

struct SAMPLE;

/* a sound! */
class Sound{
public:
	Sound();
        /* create from wav file (riff header + pcm) */
        Sound(const char * data, int length);
        /* load from path */
	Sound(const std::string & path) throw (LoadException);
	Sound(const Sound & copy);

        /* do any global initialization necessary */
        static void initialize();

	Sound & operator=( const Sound & rhs );

	void play();
	void play(double volume, int pan);
	void playLoop();
        void stop();

	virtual ~Sound();

        /* global frequency to use */
        // static const int FREQUENCY = 22050;
        static int FREQUENCY;

protected:

	void destroy();

	// SAMPLE * my_sound;
        SoundData data;

	/* reference counting */
	int * own;
};

#endif
