#include <allegro.h>
#include <string>

#include "../memory.h"
#include "../sound.h"
#include "configuration.h"
#include "../load_exception.h"

using namespace std;

Sound::Sound():
own(NULL){
}

Sound::Sound(const char * data, int length):
own(NULL){
    PACKFILE_VTABLE table = Memory::makeTable();
    Memory::memory memory((unsigned char *) data, length);

    PACKFILE * pack = pack_fopen_vtable(&table, &memory);
    this->data.sample = load_wav_pf(pack);
    pack_fclose(pack);
    if (!this->data.sample){
        throw LoadException("Could not load wav data");
    }
	
    own = new int;
    *own = 1;
}

Sound::Sound(const string & path) throw( LoadException ):
own(NULL){
    data.sample = load_sample( path.c_str() );

    if ( !data.sample ){
        string xf( "Could not load " );
        xf += path;
        throw LoadException(xf);
    }

    own = new int;
    *own = 1;
}

void Sound::destroy(){
    if ( own ){
        *own -= 1;
        if ( *own == 0 ){
            delete own;
            destroy_sample(data.sample);
            own = NULL;
        }
    }
}

void Sound::initialize(){
    /* default for alsa is 8, so reserve a few more */
    reserve_voices(16, -1);
    /* is calling this function a good idea? */
    set_volume_per_voice(0);
    // out<<"Install sound: "<<install_sound( DIGI_AUTODETECT, MIDI_NONE, "" )<<endl;
    install_sound(DIGI_AUTODETECT, MIDI_NONE, "");
}
        
void Sound::stop(){
    if (data.sample){
        stop_sample(data.sample);
    }
}

int scaleVolume(int v){
    return (int)(v * (int) Configuration::getSoundVolume() / 100.0);
}

void Sound::play(){
    if (data.sample){
        play_sample(data.sample, scaleVolume(255), 128, 1000, false);
    }
}

void Sound::play(int volume, int pan){
    if ( data.sample){
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

        play_sample( data.sample, scaleVolume(v), p, 1000, false );
    }
}
	
void Sound::playLoop(){
    if ( data.sample ){
        play_sample( data.sample, 255, 128, 1000, true );
    }
}
