#ifndef _paintown_sound_h
#define _paintown_sound_h

#include <string>
#include "load_exception.h"

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

	Sound & operator=( const Sound & rhs );

	void play();
	void play( int volume, int pan );
	void playLoop();
        void stop();

	virtual ~Sound();

protected:

	void destroy();

	SAMPLE * my_sound;

	/* reference counting */
	int * own;
};

#endif
