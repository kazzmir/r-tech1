#include "music-player.h"
#include "globals.h"
#include <iostream>
#include "configuration.h"
#include "sound.h"

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
}

void DumbPlayer::render(Uint8 * stream, int length){
    double delta = 65536.0 / Sound::FREQUENCY;

    /* a frame is 32 bits, 16 bit samples with 2 channels = 2 * 16,
     * so we have to divide the number of 'dumb samples' by the
     * size of each frame, 32 bits = 4 bytes
     */
    int n = duh_render(renderer, 16, 0, volume, delta, length / 4, stream);
    
    if (n == 0){
        Global::debug(0) << "Sound finished?" << std::endl;
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
    this->volume = volume;
}

DumbPlayer::~DumbPlayer(){
    DumbPlayerInfo * info = (DumbPlayerInfo*) Mix_GetMusicHookData();
    Mix_HookMusic(NULL, NULL);
    if (info != NULL){
        delete info;
    }
    /* I'm pretty sure if we get this far then there is no chance
     * that our mixer function will still be active.
     */
    duh_end_sigrenderer(renderer);
    unload_duh(music_file);
}

#endif

}
