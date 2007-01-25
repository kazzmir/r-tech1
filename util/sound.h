#ifndef _pain_sound_h
#define _pain_sound_h

#include <string>
#include "load_exception.h"

struct SAMPLE;

using namespace std;

/* a sound! */
class Sound{
public:
	Sound( const string & path ) throw( LoadException );
	Sound( const Sound & copy );

	void play();

	virtual ~Sound();

protected:

	SAMPLE * my_sound;
	int * own;

};

#endif
