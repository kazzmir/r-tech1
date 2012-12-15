#include <allegro.h>
#include <string>

#include "../../memory.h"
#include "util/exceptions/load_exception.h"

using namespace std;

/* allegro uses volume in the range of 0-255 */
static const int MAX_VOLUME = 255;

Sound::Sound():
own(NULL){
}

Sound::Sound(const char * data, int length):
own(NULL){
    loadFromMemory(data, length);
}

void Sound::loadFromMemory(const char * data, int length){
    PACKFILE_VTABLE table = Memory::makeTable();
    Memory::memory memory((unsigned char *) data, length);

    PACKFILE * pack = pack_fopen_vtable(&table, &memory);
    this->data.sample = load_wav_pf(pack);
    pack_fclose(pack);
    if (!this->data.sample){
        throw LoadException(__FILE__, __LINE__, "Could not load wav data");
    }
	
    own = new int;
    *own = 1;
}

Sound::Sound(const string & path):
own(NULL){
    data.sample = load_sample( path.c_str() );

    if ( !data.sample ){
        string xf( "Could not load " );
        xf += path;
        throw LoadException(__FILE__, __LINE__, xf);
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

void Sound::uninitialize(){
    /* anything needed? */
}

void Sound::stop(){
    if (data.sample){
        stop_sample(data.sample);
        data.voice = -1;
    }
}

void Sound::play(){
    if (data.sample){
        data.voice = play_sample(data.sample, (int)(scale(1.0) * MAX_VOLUME), 128, 1000, false);
    }
}

void Sound::play(double volume, int pan){
    if ( data.sample){
        if (pan > 255 ){
            pan = 255;
        } else if ( pan < 0 ){
            pan = 0;
        }
        int v = volume;
        if (v < 0){
            v = 0;
        } else if (v > 1){
            v = 1;
        }

        data.voice = play_sample( data.sample, (int)(scale(v) * MAX_VOLUME), pan, 1000, false );
    }
}

void Sound::playLoop(){
    if (data.sample){
        data.voice = play_sample(data.sample, 255, 128, 1000, true);
    }
}

void Sound::setVolume(double volume){
    if (data.voice != -1){
        voice_set_volume(data.voice, (int)(scale(volume) * MAX_VOLUME));
    }
}
