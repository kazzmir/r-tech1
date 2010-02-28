#include <allegro.h>
#include <string>

#include "memory.h"
#include "sound.h"
#include "configuration.h"
#include "load_exception.h"

using namespace std;

Sound::Sound():
my_sound(NULL),
own(NULL){
}

Sound::Sound(const char * data, int length):
my_sound(NULL),
own(NULL){
    PACKFILE_VTABLE table = Memory::makeTable();
    Memory::memory memory((unsigned char *) data, length);

    PACKFILE * pack = pack_fopen_vtable(&table, &memory);
    my_sound = load_wav_pf(pack);
    pack_fclose(pack);
    if (!my_sound){
        throw LoadException("Could not load wav data");
    }
	
    own = new int;
    *own = 1;
}

Sound::Sound(const string & path) throw( LoadException ):
my_sound(NULL ),
own(NULL){
    my_sound = load_sample( path.c_str() );

    if ( !my_sound ){
        string xf( "Could not load " );
        xf += path;
        throw LoadException(xf);
    }

    own = new int;
    *own = 1;
}

Sound::Sound( const Sound & copy ):
my_sound( NULL ),
own( NULL ){
    own = copy.own;
    if ( own ){
        *own += 1;
    }
    my_sound = copy.my_sound;
}

void Sound::destroy(){
    if ( own ){
        *own -= 1;
        if ( *own == 0 ){
            delete own;
            destroy_sample( my_sound );
            own = NULL;
        }
    }
}

Sound & Sound::operator=( const Sound & rhs ){
    if ( own ){
        destroy();
    }
    own = rhs.own;
    if ( own ){
        *own += 1;
    }
    my_sound = rhs.my_sound;

    return *this;
}
        
void Sound::stop(){
    if (my_sound){
        stop_sample(my_sound);
    }
}

int scaleVolume(int v){
    return (int)(v * (int) Configuration::getSoundVolume() / 100.0);
}

void Sound::play(){
    if (my_sound){
        play_sample(my_sound, scaleVolume(255), 128, 1000, false);
    }
}

void Sound::play(int volume, int pan){
    if ( my_sound ){
        int p = pan;
        if ( p > 255 ){
            p = 255;
        } else if ( p < 0 ){
            p = 0;
        }
        int v = volume;
        if (v < 0){
            v = 0;
        } else if (v > 255){
            v = 255;
        }

        play_sample( my_sound, scaleVolume(v), p, 1000, false );
    }
}
	
void Sound::playLoop(){
    if ( my_sound ){
        play_sample( my_sound, 255, 128, 1000, true );
    }
}

Sound::~Sound(){
    destroy();
}
