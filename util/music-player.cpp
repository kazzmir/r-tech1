#ifdef USE_ALLEGRO
#include <allegro.h>
#endif

#include "music-player.h"
#include "globals.h"
#include "util/debug.h"
#include <iostream>
#include "configuration.h"
#include "sound.h"
#include "dumb/include/dumb.h"
#include "gme/Music_Emu.h"
#include "exceptions/exception.h"
#include <sstream>

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

#ifdef HAVE_MP3_MPG123
#include <mpg123.h>
#endif

#ifdef HAVE_MP3_MAD
#include <mad.h>
#endif

#ifdef USE_SDL
#include "sdl/mixer/SDL_mixer.h"
#endif

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
    return start;
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

static int ALLEGRO_MONO = 0;
static int ALLEGRO_STEREO = 1;

DumbPlayer::DumbPlayer(const char * path){
    music_file = loadDumbFile(path);
    if (music_file != NULL){
        int buf = 1 << 11;
        player = al_start_duh(music_file, 2, 0, scaleVolume(volume), buf, Sound::FREQUENCY);
    } else {
        std::ostringstream error;
        error << "Could not load DUMB file " << path;
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

    stream = play_audio_stream(GME_BUFFER_SIZE, 16, ALLEGRO_STEREO, Sound::FREQUENCY, 255, 128);
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
            buffer[i] *= volume;
            buffer[i] += 0x8000;
        }

        free_audio_stream_buffer(stream);
    }
}

void GMEPlayer::pause(){
    voice_stop(stream->voice);
}

void GMEPlayer::setVolume(double volume){
    this->volume = volume;
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


#ifdef HAVE_MP3_MPG123

static const int MPG123_BUFFER_SIZE = 1 << 11;
Mp3Player::Mp3Player(const char * path):
stream(NULL),
mp3(NULL){
    /* Initialize */
    if (mpg123_init() != MPG123_OK){
	throw MusicException(__FILE__, __LINE__, "Could not initialize mpg123");
    }
    try{
        mp3 = mpg123_new(NULL, NULL);
        if (mp3 == NULL){
            throw MusicException(__FILE__,__LINE__, "Could not allocate mpg handle");
        }
        mpg123_format_none(mp3);
        int error = mpg123_format(mp3, Sound::FREQUENCY, MPG123_STEREO, MPG123_ENC_SIGNED_16);
        if (error != MPG123_OK){
            Global::debug(0) << "Could not set format for mpg123 handle" << std::endl;
        }
        error = mpg123_open(mp3, (char*) path);
        if (error == -1){
            std::ostringstream error;
            error << "Could not open mpg123 file " << path << " error code " << error;
            throw MusicException(__FILE__,__LINE__, error.str());
        }
        // Global::debug(0) << "mpg support " << mpg123_format_support(mp3, Sound::FREQUENCY, MPG123_ENC_SIGNED_16) << std::endl;

        double base, really, rva;
        mpg123_getvolume(mp3, &base, &really, &rva);
        // Global::debug(0) << "mpg volume base " << base << " really " << really << " rva " << rva << std::endl;
        base_volume = base;

        long rate;
        int channels, encoding;
        mpg123_getformat(mp3, &rate, &channels, &encoding);
        // Global::debug(0) << path << " rate " << rate << " channels " << channels << " encoding " << encoding << std::endl;
        stream = play_audio_stream(MPG123_BUFFER_SIZE, 16, ALLEGRO_STEREO, rate, 255, 128);
        voice_set_priority(stream->voice, 255);
    } catch (const MusicException & fail){
        mpg123_exit();
        throw;
    }
}

void Mp3Player::play(){
    voice_start(stream->voice);
}

void Mp3Player::poll(){
    short * buffer = (short*) get_audio_stream_buffer(stream);
    if (buffer){
        /* buffer * 4 for 16 bits per sample * 2 samples for stereo */
        size_t out = 0;
        mpg123_read(mp3, (unsigned char *) buffer, MPG123_BUFFER_SIZE * 4, &out);
        // Global::debug(0) << "Decoded " << out << std::endl;
        for (int i = 0; i < MPG123_BUFFER_SIZE * 2; i++){
            buffer[i] *= volume;
            buffer[i] += 0x8000;
        }
        free_audio_stream_buffer(stream);
    }
}

void Mp3Player::pause(){
    /* TODO */
}

void Mp3Player::setVolume(double volume){
    this->volume = volume;
    // mpg123_volume(mp3, volume * base_volume / 5000);
    mpg123_volume(mp3, 0.0001);
}

Mp3Player::~Mp3Player(){
    mpg123_close(mp3);
    mpg123_exit();
    stop_audio_stream(stream);
}

#endif /* MP3_MPG123 */

#ifdef HAVE_MP3_MAD
    /* TODO */
Mp3Player::Mp3Player(const char * path):
stream(NULL){
    /* TODO */
}

void Mp3Player::play(){
    /* TODO */
}

void Mp3Player::poll(){
    /* TODO */
}

void Mp3Player::pause(){
    /* TODO */
}

void Mp3Player::setVolume(double volume){
    /* TODO */
}

Mp3Player::~Mp3Player(){
    /* TODO */
}
#endif /* MP3_MAD */

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

/* volume should be in the range 0.0 to 1.0 */
void DumbPlayer::setVolume(double volume){
    this->volume = volume;
    /* Mix_VolumeMusic only handles volume for formats it handles natively.
     * for custom formats (like all these players) we have to handle volume
     * ourselves.
     */
    // Mix_VolumeMusic(MIX_MAX_VOLUME * volume);
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

    /* scale for volume */
    for (int i = 0; i < length / 2; i++){
        short & sample = ((short *) stream)[i];
        sample *= volume;
    }

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
    this->volume = volume;
    // Mix_VolumeMusic(volume * MIX_MAX_VOLUME);
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
    this->volume = volume;
    Mix_VolumeMusic(volume * MIX_MAX_VOLUME);
}

OggPlayer::~OggPlayer(){
    Mix_FreeMusic(music);
}

#endif /* OGG */

#ifdef HAVE_MP3_MPG123


struct Mp3Info{
    Mp3Player * mp3;
};

Mp3Player::Mp3Player(const char * path):
mp3(NULL){
    /* Initialize */
    if (mpg123_init() != MPG123_OK){
	throw MusicException(__FILE__, __LINE__, "Could not initialize mpg123");
    }
    try{
        mp3 = mpg123_new(NULL, NULL);
        if (mp3 == NULL){
            throw MusicException(__FILE__,__LINE__, "Could not allocate mpg handle");
        }
        mpg123_format_none(mp3);
        int error = mpg123_format(mp3, Sound::FREQUENCY, MPG123_STEREO, MPG123_ENC_SIGNED_16);
        if (error != MPG123_OK){
            Global::debug(0) << "Could not set format for mpg123 handle" << std::endl;
        }
        error = mpg123_open(mp3, (char*) path);
        if (error == -1){
            std::ostringstream error;
            error << "Could not open mpg123 file " << path << " error code " << error;
            throw MusicException(__FILE__,__LINE__, error.str());
        }
        // Global::debug(0) << "mpg support " << mpg123_format_support(mp3, Sound::FREQUENCY, MPG123_ENC_SIGNED_16) << std::endl;

        double base, really, rva;
        mpg123_getvolume(mp3, &base, &really, &rva);
        // Global::debug(0) << "mpg volume base " << base << " really " << really << " rva " << rva << std::endl;
        base_volume = base;

        long rate;
        int channels, encoding;
        mpg123_getformat(mp3, &rate, &channels, &encoding);
        // Global::debug(0) << path << " rate " << rate << " channels " << channels << " encoding " << encoding << std::endl;
    } catch (const MusicException & fail){
        mpg123_exit();
        throw;
    }
}

void Mp3Player::mixer(void * arg, Uint8 * stream, int length){
    Mp3Info * info = (Mp3Info *) arg;
    Mp3Player * player = (Mp3Player *) info->mp3;
    mpg123_read(player->mp3, stream, length, NULL);
}

void Mp3Player::render(Uint8 * stream, int length){
}

void Mp3Player::play(){
    Mp3Info * me = new Mp3Info;
    me->mp3 = this;
    Mix_HookMusic(mixer, me);
}

void Mp3Player::poll(){
    /* TODO */
}

void Mp3Player::pause(){
    /* TODO */
}

void Mp3Player::setVolume(double volume){
    mpg123_volume(mp3, volume * base_volume / 5000);
}

Mp3Player::~Mp3Player(){
    Mix_HookMusic(NULL, NULL);
    if (mp3 != NULL){
        mpg123_close(mp3);
        mpg123_delete(mp3);
        mp3 = NULL;
    }
    mpg123_exit();
}

#endif /* MP3_MPG123 */

#ifdef HAVE_MP3_MAD
        /* TODO */
Mp3Player::Mp3Player(const char * path){
    music = Mix_LoadMUS(path);
    if (music == NULL){
        throw MusicException(__FILE__, __LINE__, "Could not load MP3 file");
    }
}

void Mp3Player::play(){
    Mix_PlayMusic(music, -1);
}

void Mp3Player::poll(){
    /* TODO */
}

void Mp3Player::pause(){
    Mix_PauseMusic();
}

void Mp3Player::setVolume(double volume){
    this->volume = volume;
    Mix_VolumeMusic(volume * MIX_MAX_VOLUME);
}

Mp3Player::~Mp3Player(){
    Mix_FreeMusic(music);
}
#endif /* MP3_MAD */

#endif /* SDL */

#ifdef USE_ALLEGRO5
GMEPlayer::GMEPlayer(const char * path){
    /* TODO */
}

void GMEPlayer::play(){
    /* TODO */
}

void GMEPlayer::poll(){
    /* TODO */
}

void GMEPlayer::pause(){
    /* TODO */
}

void GMEPlayer::setVolume(double volume){
    /* TODO */
}

GMEPlayer::~GMEPlayer(){
    /* TODO */
}

#ifdef HAVE_OGG
OggPlayer::OggPlayer(const char * path){
    /* TODO */
}

void OggPlayer::play(){
    /* TODO */
}

void OggPlayer::poll(){
    /* TODO */
}

void OggPlayer::pause(){
    /* TODO */
}

void OggPlayer::setVolume(double volume){
    /* TODO */
}

OggPlayer::~OggPlayer(){
    /* TODO */
}

#endif

#ifdef HAVE_MP3_MPG123

Mp3Player::Mp3Player(const char * path):
mp3(NULL){
    /* Initialize */
    if (mpg123_init() != MPG123_OK){
	throw MusicException(__FILE__, __LINE__, "Could not initialize mpg123");
    }
    int error = mpg123_open(mp3, path);
    if (mp3 == NULL){
	throw MusicException(__FILE__,__LINE__, "Problem loading file.");
    }
}

void Mp3Player::play(){
    /* TODO */
}

void Mp3Player::poll(){
    /* TODO */
}

void Mp3Player::pause(){
    /* TODO */
}

void Mp3Player::setVolume(double volume){
    mpg123_volume(mp3, volume);
}

Mp3Player::~Mp3Player(){
    mpg123_close(mp3);
    mpg123_exit();
}

#endif /* MP3_MPG123 */

#ifdef HAVE_MP3_MAD
        /* TODO */
Mp3Player::Mp3Player(const char * path){
    /* TODO */
}

void Mp3Player::play(){
    /* TODO */
}

void Mp3Player::poll(){
    /* TODO */
}

void Mp3Player::pause(){
    /* TODO */
}

void Mp3Player::setVolume(double volume){
    /* TODO */
}

Mp3Player::~Mp3Player(){
    /* TODO */
}
#endif /* MP3_MAD */

DumbPlayer::DumbPlayer(const char * path){
    /* TODO */
}

void DumbPlayer::play(){
    /* TODO */
}

void DumbPlayer::poll(){
    /* TODO */
}

void DumbPlayer::pause(){
    /* TODO */
}

void DumbPlayer::setVolume(double volume){
    /* TODO */
}

DumbPlayer::~DumbPlayer(){
    /* TODO */
}

#endif

}
