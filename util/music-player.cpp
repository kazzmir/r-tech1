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

#ifdef USE_ALLEGRO5
#include <allegro5/allegro_audio.h>
#endif

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

#ifdef USE_ALLEGRO5
const int DUMB_SAMPLES = 1024;
MusicRenderer::MusicRenderer(){
    stream = al_create_audio_stream(2, DUMB_SAMPLES, Sound::FREQUENCY, ALLEGRO_AUDIO_DEPTH_INT16,  ALLEGRO_CHANNEL_CONF_2);
    if (!stream){
        throw MusicException(__FILE__, __LINE__, "Could not create allegro5 audio stream");
    }
    queue = al_create_event_queue();
    al_register_event_source(queue, al_get_audio_stream_event_source(stream));
}

void MusicRenderer::play(MusicPlayer & player){
    al_attach_audio_stream_to_mixer(stream, al_get_default_mixer());
}

void MusicRenderer::pause(){
    al_detach_audio_stream(stream);
}

MusicRenderer::~MusicRenderer(){
    al_destroy_audio_stream(stream);
    al_destroy_event_queue(queue);
}

void MusicRenderer::poll(MusicPlayer & player){
    ALLEGRO_EVENT event;
    while (al_get_next_event(queue, &event)){
        if (event.type == ALLEGRO_EVENT_AUDIO_STREAM_FRAGMENT) {
            ALLEGRO_AUDIO_STREAM * stream = (ALLEGRO_AUDIO_STREAM *) event.any.source;
            void * data = al_get_audio_stream_fragment(stream);
            if (data != NULL){
                player.render(data, al_get_audio_stream_length(stream));
                al_set_audio_stream_fragment(stream, data);
            }
        }
    }
}
#elif USE_SDL
MusicRenderer::MusicRenderer(){
}

void MusicRenderer::mixer(void * arg, Uint8 * stream, int bytes){
    MusicPlayer * player = (MusicPlayer*) arg;
    player->render(stream, bytes / 4);
}

void MusicRenderer::play(MusicPlayer & player){
    Mix_HookMusic(mixer, &player);
}

void MusicRenderer::pause(){
    Mix_HookMusic(NULL, NULL);
}

void MusicRenderer::poll(MusicPlayer & player){
}

MusicRenderer::~MusicRenderer(){
    Mix_HookMusic(NULL, NULL);
}
#elif USE_ALLEGRO
int BUFFER_SIZE = 1 << 11;
static int ALLEGRO_MONO = 0;
static int ALLEGRO_STEREO = 1;
MusicRenderer::MusicRenderer(){
    stream = play_audio_stream(BUFFER_SIZE, 16, ALLEGRO_STEREO, Sound::FREQUENCY, 255, 128);
    voice_set_priority(stream->voice, 255);
}

void MusicRenderer::play(MusicPlayer & player){
    voice_start(stream->voice);
}

void MusicRenderer::pause(){
    voice_stop(stream->voice);
}

void MusicRenderer::poll(MusicPlayer & player){
    short * buffer = (short*) get_audio_stream_buffer(stream);
    if (buffer){
        player.render(buffer, BUFFER_SIZE);

        /* allegro wants unsigned data but gme produces signed so to convert
         * signed samples to unsigned samples we have to raise each value
         * by half the maximum value of a short (0xffff+1)/2 = 0x8000
         */
        for (int i = 0; i < BUFFER_SIZE * 2; i++){
            buffer[i] += 0x8000;
        }

        free_audio_stream_buffer(stream);
    }
}

MusicRenderer::~MusicRenderer(){
    stop_audio_stream(stream);
}
#endif
    
MusicPlayer::MusicPlayer():
volume(1.0),
out(new MusicRenderer()){
}

MusicPlayer::~MusicPlayer(){
}

void MusicPlayer::play(){
    out->play(*this);
}

void MusicPlayer::pause(){
    out->pause();
}

void MusicPlayer::poll(){
    out->poll(*this);
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

#ifdef USE_ALLEGRO
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

void OggPlayer::render(void * stream, int length){
}

void OggPlayer::pause(){
}

void OggPlayer::setVolume(double volume){
}

OggPlayer::~OggPlayer(){
    logg_destroy_stream(stream);
}
#endif /* OGG */

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

#endif /* ALLEGRO */

#ifdef USE_SDL
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

void OggPlayer::render(void * data, int length){
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
#ifdef HAVE_MP3_MAD
        /* TODO */
Mp3Player::Mp3Player(const char * path){
    music = Mix_LoadMUS(path);
    if (music == NULL){
        throw MusicException(__FILE__, __LINE__, "Could not load MP3 file");
    }
}

void Mp3Player::play(){
    Mix_PlayMusic(music, 0);
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
    
void OggPlayer::render(void * stream, int length){
}

void OggPlayer::setVolume(double volume){
    /* TODO */
}

OggPlayer::~OggPlayer(){
    /* TODO */
}

#endif
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

#endif

/* expects each sample to be 4 bytes, 2 bytes per sample * 2 channels */
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

void DumbPlayer::render(void * data, int samples){
    double delta = 65536.0 / Sound::FREQUENCY;
    /* FIXME: use global music volume to scale the output here */
    int n = duh_render(renderer, 16, 0, volume, delta, samples, data);
}

void DumbPlayer::setVolume(double volume){
    this->volume = volume;
}

DumbPlayer::~DumbPlayer(){
    duh_end_sigrenderer(renderer);
    unload_duh(music_file);
}

DUH * DumbPlayer::loadDumbFile(const char * path){
    DUH * what;
    for (int i = 0; i < 4; i++){
        /* the order of trying xm/s3m/it/mod matters because mod could be
         * confused with one of the other formats, so load it last.
         */
        switch (i){
            case 0 : {
                what = dumb_load_xm_quick(path);
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

GMEPlayer::GMEPlayer(const char * path):
emulator(NULL){
    gme_err_t fail = gme_open_file(path, &emulator, Sound::FREQUENCY);
    if (fail != NULL){
        Global::debug(0) << "GME load error for " << path << ": " << fail << std::endl;
        throw MusicException(__FILE__, __LINE__, "Could not load GME file");
    }
    emulator->start_track(0);
    Global::debug(0) << "Loaded GME file " << path << std::endl;
}
    
void GMEPlayer::render(void * stream, int length){
    /* length/2 to convert bytes to short */
    emulator->play(length * 2, (short*) stream);

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

    /* scale for volume */
    for (int i = 0; i < length * 2; i++){
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

void GMEPlayer::setVolume(double volume){
    this->volume = volume;
}

GMEPlayer::~GMEPlayer(){
    delete emulator;
}

#ifdef HAVE_MP3_MPG123
/* initialize the mpg123 library and open up an mp3 file for reading */
static void initializeMpg123(mpg123_handle ** mp3, const char * path){
    /* Initialize */
    if (mpg123_init() != MPG123_OK){
	throw MusicException(__FILE__, __LINE__, "Could not initialize mpg123");
    }
    try{
        *mp3 = mpg123_new(NULL, NULL);
        if (*mp3 == NULL){
            throw MusicException(__FILE__,__LINE__, "Could not allocate mpg handle");
        }
        mpg123_format_none(*mp3);

        /* allegro wants unsigned samples but mpg123 can't actually provide unsigned
         * samples even though it has an enum for it, MPG123_ENC_UNSIGNED_16. this
         * was rectified in 1.13.0 or something, but for now signed samples are ok.
         */
        int error = mpg123_format(*mp3, Sound::FREQUENCY, MPG123_STEREO, MPG123_ENC_SIGNED_16);
        if (error != MPG123_OK){
            Global::debug(0) << "Could not set format for mpg123 handle" << std::endl;
        }
        
        /* FIXME workaround for libmpg issues with "generic" decoder frequency not being set */
        error = mpg123_open(*mp3, (char*) path);
        if (error == -1){
            std::ostringstream error;
            error << "Could not open mpg123 file " << path << " error code " << error;
            throw MusicException(__FILE__,__LINE__, error.str());
        }
        
        /* reading a frame is the only surefire way to get mpg123 to set the
         * sampling_frequency which it needs to set the decoder a few lines below
         */
        size_t dont_care;
        unsigned char tempBuffer[4096];
        error = mpg123_read(*mp3, tempBuffer, sizeof(tempBuffer), &dont_care);
	if (!(error == MPG123_OK || error == MPG123_NEW_FORMAT)){
            std::ostringstream error;
            error << "Could not read mpg123 file " << path << " error code " << error;
            throw MusicException(__FILE__,__LINE__, error.str());
        }
	mpg123_close(*mp3);
	
        /* stream has progressed a little bit so reset it by opening it again */
	error = mpg123_open(*mp3, (char*) path);
        if (error == -1){
            std::ostringstream error;
            error << "Could not open mpg123 file " << path << " error code " << error;
            throw MusicException(__FILE__,__LINE__, error.str());
        }
        /* FIXME end */

        /* some of the native decoders aren't stable in older versions of mpg123
         * so just use generic for now. 1.13.1 should work better
         */
        error = mpg123_decoder(*mp3, "generic");
        if (error != MPG123_OK){
            std::ostringstream error;
            error << "Could not use 'generic' mpg123 decoder for " << path << " error code " << error;
            throw MusicException(__FILE__,__LINE__, error.str());
        }
        // Global::debug(0) << "mpg support " << mpg123_format_support(mp3, Sound::FREQUENCY, MPG123_ENC_SIGNED_16) << std::endl;

        /*
        double base, really, rva;
        mpg123_getvolume(*mp3, &base, &really, &rva);
        // Global::debug(0) << "mpg volume base " << base << " really " << really << " rva " << rva << std::endl;
        base_volume = base;

        long rate;
        int channels, encoding;
        mpg123_getformat(*mp3, &rate, &channels, &encoding);
        // Global::debug(0) << path << " rate " << rate << " channels " << channels << " encoding " << encoding << std::endl;
        */
    } catch (const MusicException & fail){
        if (*mp3 != NULL){
            mpg123_close(*mp3);
            mpg123_delete(*mp3);
            *mp3 = NULL;
        }
        mpg123_exit();
        throw;
    }
}

static const int MPG123_BUFFER_SIZE = 1 << 11;
Mp3Player::Mp3Player(const char * path):
mp3(NULL){
    initializeMpg123(&mp3, path);
    long rate = 0;
    int channels = 0, encoding = 0;
    mpg123_getformat(mp3, &rate, &channels, &encoding);
}

void Mp3Player::render(void * data, int samples){
    /* buffer * 4 for 16 bits per sample * 2 samples for stereo */
    size_t out = 0;
    mpg123_read(mp3, (unsigned char *) data, samples * 4, &out);

    /*
       long rate;
       int channels, encoding;
       mpg123_getformat(mp3, &rate, &channels, &encoding);
       Global::debug(0) << "rate " << rate << " channels " << channels << " encoding " << encoding << std::endl;
       */
}

void Mp3Player::setVolume(double volume){
    mpg123_volume(mp3, volume);
    /*
    this->volume = volume;
    // mpg123_volume(mp3, volume * base_volume / 5000);
    mpg123_volume(mp3, 0.0001);
    */
    // mpg123_volume(mp3, volume);
}

Mp3Player::~Mp3Player(){
    mpg123_close(mp3);
    mpg123_exit();
}

#endif /* MP3_MPG123 */

}
