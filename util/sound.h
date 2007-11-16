#ifndef _pain_sound_h
#define _pain_sound_h

#include <string>
#include "load_exception.h"

struct SAMPLE;

using namespace std;

/* a sound! */
class Sound{
public:
	Sound();
	Sound( const string & path ) throw( LoadException );
	Sound( const Sound & copy );

	Sound & operator=( const Sound & rhs );

	void play();
	void play( int volume, int pan );
	void playLoop();

	virtual ~Sound();

protected:

	void destroy();

	SAMPLE * my_sound;

	/* reference counting */
	int * own;

};

#endif
