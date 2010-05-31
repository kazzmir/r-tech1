#include "music-player.h"
#include "globals.h"
#include <iostream>
#include "configuration.h"
#include "funcs.h"
#include "system.h"

#include "dumb/include/dumb.h"

#ifdef USE_ALLEGRO
#include "dumb/include/aldumb.h"

#ifdef _WIN32
/* what do we need winalleg for?
 * reason: ...
 */
#include <winalleg.h>
#endif

#endif

#ifdef USE_SDL
#include "sdl/mixer/SDL_mixer.h"
#endif

namespace Util{

static double scaleVolume(double start){
    return start * Configuration::getMusicVolume() / 100;
}
    
MusicPlayer::MusicPlayer(){
}

MusicPlayer::~MusicPlayer(){
}

static const char * typeToExtension( int i ){
    switch (i){
        case 0 : return ".xm";
        case 1 : return ".s3m";
        case 2 : return ".it";
        case 3 : return ".mod";
        default : return "";
    }
}

DUH * DumbPlayer::loadDumbFile(const char * path){
    DUH * what;
    for (int i = 0; i < 4; i++){
        /* the order of trying xm/s3m/it/mod matters because mod could be
         * confused with one of the other formats, so load it last.
         */
        switch (i){
            case 0 : {
                 what =  dumb_load_xm_quick(path);
                 break;
            }
            case 1 : {
                what = dumb_load_s3m_quick(path);
                break;
            }
            case 2 : {
                what = dumb_load_it_quick(path);
                break;
            }
            case 3 : {
                what = dumb_load_mod_quick(path);
                break;
            }
        }
        
        if (what != NULL){
            Global::debug(0) << "Loaded " << path << " type " << typeToExtension(i) << "(" << i << ")" << std::endl;
            return what;
        }
    }
    return NULL;
}

#ifdef USE_ALLEGRO

DumbPlayer::DumbPlayer(const char * path):
volume(1.0){
    music_file = loadDumbFile(path);
    if (music_file != NULL){
        int buf = 1 << 11;
        player = al_start_duh(music_file, 2, 0, scaleVolume(volume), buf, FREQUENCY);
    }
}

void DumbPlayer::play(){
    al_resume_duh(this->player);
}

void DumbPlayer::poll(){
    if (al_poll_duh(this->player) != 0){
    }
}

void DumbPlayer::pause(){
    al_pause_duh(this->player);
}

void DumbPlayer::setVolume(double volume){
    this->volume = volume;
    al_duh_set_volume(player, scaleVolume(volume));
}

DumbPlayer::~DumbPlayer(){
    al_stop_duh(player);
    unload_duh(music_file);
}

#endif
    

#ifdef USE_SDL
DumbPlayer::DumbPlayer(const char * path):
volume(1.0){
    music_file = loadDumbFile(path);
    if (music_file == NULL){
        /* FIXME */
        throw std::exception();
    }
    
    int n_channels = 2;
    int position = 0;
    renderer = duh_start_sigrenderer(music_file, 0, n_channels, position);
    if (!renderer){
        Global::debug(0) << "Could not create renderer" << std::endl;
        throw std::exception();
    }

    // Mix_HookMusic(mixer, this);

    /*
    AL_DUH_PLAYER *dp;

    if (!duh)
        return NULL;

    dp = malloc(sizeof(*dp));
    if (!dp)
        return NULL;

    dp->flags = ADP_PLAYING;
    dp->bufsize = bufsize;
    dp->freq = freq;

    dp->stream = play_audio_stream(bufsize, 16, n_channels - 1, freq, 255, 128);

    if (!dp->stream) {
        free(dp);
        return NULL;
    }

    voice_set_priority(dp->stream->voice, 255);

    dp->sigrenderer = duh_start_sigrenderer(duh, 0, n_channels, pos);

    if (!dp->sigrenderer) {
        stop_audio_stream(dp->stream);
        free(dp);
        return NULL;
    }

    dp->volume = volume;
    dp->silentcount = 0;

    return dp;
    */
}

void DumbPlayer::render(Uint8 * stream, int length){
    int n;
    // Global::debug(0) << "Dumb render in " << (void*) stream << " buffer " << length << std::endl;
    double delta = 1.0;
    delta = 65536.0 / 22050;
    // delta = 1.0;
    // delta = 0.5;
    /*
    static int ok = 0;
    ok += 1;
    if (ok < 2){
        // memset(stream, 0, length);
        return;
    }
    ok = 0;
    */
    n = 0;
    n = duh_render(renderer, 16, 0, volume, delta, length / 4, stream);
    // Global::debug(0) << "Rendered " << n << " bits" << " at " << System::currentMicroseconds() << std::endl; 
    // int n_channels = duh_sigrenderer_get_n_channels(dp->sigrenderer);
    // n *= n_channels;
    /*
    int size = length * n_channels;
        */
#if 0
    Uint16 * s16 = (Uint16*) stream;
    int saw = 0;
    int f = 15000;
    for (; n < length; n += 2){
        /*
        if (n < length / 2){
            s16[n] = 32000;
        } else {
            s16[n] = -32000;
        }
        */
        s16[n] = f;
        saw += 1;
        if (saw > 200){
            f = -f;
            saw = 0;
        }
        // s16[n] = 10000 + n * 5;
        /*
        switch (n%4){
            case 0 : s16[n] = 0; break;
            case 1 : s16[n] = 16000; break;
            case 2 : s16[n] = 32000; break;
            case 3 : s16[n] = -32000; break;
        }
        */
        /*
        switch(rnd(2)){
            case 0 : s16[n] = 32000; break;
            case 1 : s16[n] = -32000;
        }
        */
        // s16[n] = rnd(65536) - 65536/2;
        // stream[n] = 0xa0;
    }
#endif

    if (n == 0){
        // Global::debug(0) << "Sound finished?" << std::endl;
    }
}

struct DumbPlayerInfo{
    MusicPlayer * who;
};

void DumbPlayer::mixer(void * player_, Uint8 * stream, int length){
    DumbPlayerInfo * info = (DumbPlayerInfo *) player_;
    DumbPlayer * player = (DumbPlayer *) info->who;
    player->render(stream, length);
}

void DumbPlayer::pause(){
}

void DumbPlayer::play(){
    DumbPlayerInfo * me = new DumbPlayerInfo;
    me->who = this;
    Mix_HookMusic(mixer, me);
}

void DumbPlayer::poll(){
}

void DumbPlayer::setVolume(double volume){
}

DumbPlayer::~DumbPlayer(){
    Mix_HookMusic(NULL, NULL);
}

#endif

}
