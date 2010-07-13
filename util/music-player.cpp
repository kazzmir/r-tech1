#ifdef USE_ALLEGRO
#include <allegro.h>
#endif
#include "music-player.h"
#include "globals.h"
#include <iostream>
#include "configuration.h"
#include "sound.h"

#include "dumb/include/dumb.h"
#include "gme/Music_Emu.h"

#ifdef USE_ALLEGRO
#include "dumb/include/aldumb.h"
#include "ogg/logg.h"

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

#include "exceptions/exception.h"
#include <sstream>

namespace Util{

class MusicException: public Exception::Base {
public:
    MusicException(const std::string & file, int line, const std::string & reason):
        Exception::Base(file, line),
        reason(reason){
        }

    MusicException(const MusicException & copy):
    Exception::Base(copy),
    reason(copy.reason){
    }

    virtual ~MusicException() throw(){
    }

protected:
    virtual const std::string getReason() const {
        return reason;
    }

    virtual Exception::Base * copy() const {
        return new MusicException(*this);
    }

    std::string reason;
};

static double scaleVolume(double start){
    return start * Configuration::getMusicVolume() / 100;
}
    
MusicPlayer::MusicPlayer():
volume(1.0){
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

DumbPlayer::DumbPlayer(const char * path){
    music_file = loadDumbFile(path);
    if (music_file != NULL){
        int buf = 1 << 11;
        player = al_start_duh(music_file, 2, 0, scaleVolume(volume), buf, Sound::FREQUENCY);
    } else {
        std::ostringstream error;
        error << "Could not DUMB file load " << path;
        throw MusicException(__FILE__, __LINE__, error.str());
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

static const int GME_BUFFER_SIZE = 1 << 11;
GMEPlayer::GMEPlayer(const char * path){
    gme_err_t fail = gme_open_file(path, &emulator, Sound::FREQUENCY);
    if (fail != NULL){
        Global::debug(0) << "GME load error for " << path << ": " << fail << std::endl;
        throw MusicException(__FILE__, __LINE__, "Could not load GME file");
    }
    emulator->start_track(0);
    Global::debug(0) << "Loaded GME file " << path << std::endl;

    stream = play_audio_stream(GME_BUFFER_SIZE, 16, 1, Sound::FREQUENCY, 255, 128);
    voice_set_priority(stream->voice, 255);
}

void GMEPlayer::play(){
    voice_start(stream->voice);
}

void GMEPlayer::poll(){
    short * buffer = (short*) get_audio_stream_buffer(stream);
    if (buffer){
        /* size in 16-bit samples is buffer size * channels */
        emulator->play(GME_BUFFER_SIZE * 2, buffer);
        if (emulator->track_ended()){
            gme_info_t * info;
            gme_track_info(emulator, &info, 0);
            int intro = info->intro_length;
            emulator->start_track(0);
            // Global::debug(0) << "Seeking " << intro << "ms. Track length " << info->length << "ms" << std::endl;
            /* skip past the intro if there is a loop */
            if (info->loop_length != 0){
                emulator->seek(intro);
            }
        }

        /* allegro wants unsigned data but gme produces signed so to convert
         * signed samples to unsigned samples we have to raise each value
         * by half the maximum value of a short (0xffff+1)/2 = 0x8000
         */
        for (int i = 0; i < GME_BUFFER_SIZE * 2; i++){
            buffer[i] += 0x8000;
        }

        free_audio_stream_buffer(stream);
    }
}

void GMEPlayer::pause(){
    voice_stop(stream->voice);
}

void GMEPlayer::setVolume(double volume){
    /* FIXME */
}

GMEPlayer::~GMEPlayer(){
    delete emulator;
    stop_audio_stream(stream);
}

#ifdef HAVE_OGG
OggPlayer::OggPlayer(const char * path){
    stream = logg_get_stream(path, scaleVolume(volume) * 255, 128, 1);
    if (stream == NULL){
        throw MusicException(__FILE__, __LINE__, "Could not load ogg file");
    }
}

void OggPlayer::play(){
}

void OggPlayer::poll(){
    logg_update_stream(stream);
}

void OggPlayer::pause(){
}

void OggPlayer::setVolume(double volume){
}

OggPlayer::~OggPlayer(){
    logg_destroy_stream(stream);
}
#endif /* OGG */

#endif /* ALlEGRO */

#ifdef USE_SDL
DumbPlayer::DumbPlayer(const char * path){
    music_file = loadDumbFile(path);
    if (music_file == NULL){
        std::ostringstream error;
        error << "Could not load DUMB file " << path;
        throw MusicException(__FILE__, __LINE__, error.str());
    }
    
    int n_channels = 2;
    int position = 0;
    renderer = duh_start_sigrenderer(music_file, 0, n_channels, position);
    if (!renderer){
        Global::debug(0) << "Could not create renderer" << std::endl;
        throw Exception::Base(__FILE__, __LINE__);
    }
}

void DumbPlayer::render(Uint8 * stream, int length){
    double delta = 65536.0 / Sound::FREQUENCY;

    /* a frame is 32 bits, 16 bit samples with 2 channels = 2 * 16,
     * so we have to divide the number of 'dumb samples' by the
     * size of each frame, 32 bits = 4 bytes
     */
    /* FIXME: use global music volume to scale the output here */
    int n = duh_render(renderer, 16, 0, volume, delta, length / 4, stream);
    
    if (n == 0){
        Global::debug(0) << "Sound finished?" << std::endl;
    }

    /*
    short large = 0;
    for (int i = 0; i < length / 2; i++){
        short z = ((short *) stream)[i];
        if (z < 0){
            z = -z;
        }
        if (z > large){
            large = z;
        }
    }
    Global::debug(0) << "Largest amplitude " << large << std::endl;
    */
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
    Mix_HookMusic(NULL, NULL);
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

struct GMEInfo{
    GMEPlayer * player;
};

GMEPlayer::GMEPlayer(const char * path){
    gme_err_t fail = gme_open_file(path, &emulator, Sound::FREQUENCY);
    if (fail != NULL){
        Global::debug(0) << "GME load error for " << path << ": " << fail << std::endl;
        throw MusicException(__FILE__, __LINE__, "Could not load GME file");
    }
    emulator->start_track(0);
    Global::debug(0) << "Loaded GME file " << path << std::endl;
}

void GMEPlayer::mixer(void * arg, Uint8 * stream, int length){
    GMEInfo * info = (GMEInfo*) arg;
    info->player->render(stream, length);
}

void GMEPlayer::render(Uint8 * stream, int length){
    /* length/2 to convert bytes to short */
    emulator->play(length / 2, (short*) stream);

    /*
    short large = 0;
    short small = 0;
    for (int i = 0; i < length / 2; i++){
        // ((short *) stream)[i] *= 2;
        short z = ((short *) stream)[i];
        if (z < small){
            small = z;
        }
        if (z > large){
            large = z;
        }
    }
    Global::debug(0) << "Largest " << large << " Smallest " << small << std::endl;
    */
}

void GMEPlayer::play(){
    GMEInfo * me = new GMEInfo;
    me->player = this;
    Mix_HookMusic(mixer, me);
}

void GMEPlayer::poll(){
}

void GMEPlayer::pause(){
}

void GMEPlayer::setVolume(double volume){
}

GMEPlayer::~GMEPlayer(){
    GMEInfo * info = (GMEInfo*) Mix_GetMusicHookData();
    Mix_HookMusic(NULL, NULL);
    if (info != NULL){
        delete info;
    }
    delete emulator;
}

#ifdef HAVE_OGG

OggPlayer::OggPlayer(const char * path){
    music = Mix_LoadMUS(path);
    if (music == NULL){
        throw MusicException(__FILE__, __LINE__, "Could not load OGG file");
    }
}

void OggPlayer::play(){
    Mix_PlayMusic(music, -1);
}

void OggPlayer::poll(){
}

void OggPlayer::pause(){
    Mix_PauseMusic();
}

void OggPlayer::setVolume(double volume){
    Mix_VolumeMusic(volume * MIX_MAX_VOLUME);
}

OggPlayer::~OggPlayer(){
    Mix_FreeMusic(music);
}

#endif /* OGG */

#endif /* SDL */

}
