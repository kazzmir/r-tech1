#include "music-player.h"
#include "globals.h"
#include <iostream>

#ifdef USE_ALLEGRO
#include "dumb/include/aldumb.h"

#ifdef _WIN32
#include <winalleg.h>
#endif

#endif

namespace Util{
    
MusicPlayer::MusicPlayer(){
}

MusicPlayer::~MusicPlayer(){
}

#ifdef USE_ALLEGRO

static const char * typeToExtension( int i ){
    switch (i){
        case 0 : return ".xm";
        case 1 : return ".s3m";
        case 2 : return ".it";
        case 3 : return ".mod";
        default : return "";
    }
}

DumbPlayer::DumbPlayer(const char * path):
volume(1.0){
    for (int i = 0; i < 4; i++){
        /* the order of trying xm/s3m/it/mod matters because mod could be
         * confused with one of the other formats, so load it last.
         */
        switch (i){
            case 0 : {
                 music_file = dumb_load_xm_quick(path);
                 break;
            }
            case 1 : {
                music_file = dumb_load_s3m_quick(path);
                break;
            }
            case 2 : {
                music_file = dumb_load_it_quick(path);
                break;
            }
            case 3 : {
                music_file = dumb_load_mod_quick(path);
                break;
            }
        }

        if ( music_file != NULL ){
            Global::debug(0) << "Loaded " << path << " type " << typeToExtension( i ) << "(" << i << ")" << std::endl;
            break;
        }
    }

    if (music_file){
        int buf = 1 << 11;
        player = al_start_duh(music_file, 2, 0, volume, buf, 22050);
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

void DumbPlayer::setVolume(int volume){
    al_duh_set_volume(player, volume);
}

DumbPlayer::~DumbPlayer(){
    al_stop_duh(player);
    unload_duh(music_file);
}

#endif

}
