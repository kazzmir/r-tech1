#ifdef USE_ALLEGRO
#include <allegro.h>
#endif

#include "music-player.h"
#include "globals.h"
#include "util/debug.h"
#include <iostream>
#include "configuration.h"
#include "sound.h"
#include "timedifference.h"
#include "dumb/include/dumb.h"
#include "gme/Music_Emu.h"
#include "exceptions/exception.h"
#include "file-system.h"
#include <sstream>
#include <stdio.h>

#ifdef USE_ALLEGRO5
#include <allegro5/allegro_audio.h>
#endif

#ifdef USE_ALLEGRO
#include "dumb/include/aldumb.h"

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

using std::string;

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

/* 1 for big endian (most significant byte)
 * 0 for little endian (least significant byte)
 */
/* FIXME: move this to global or something and find a better #ifdef */
int bigEndian(){
#if defined(PS3) || defined(WII)
    return 1;
#else
    return 0;
#endif
}

#ifdef USE_ALLEGRO5
const int DUMB_SAMPLES = 1024;
MusicRenderer::MusicRenderer(){
    create(Sound::Info.frequency, 2);
}
    
MusicRenderer::MusicRenderer(int frequency, int channels){
    create(frequency, channels);
}

void MusicRenderer::create(int frequency, int channels){
    ALLEGRO_CHANNEL_CONF configuration = ALLEGRO_CHANNEL_CONF_2;
    switch (channels){
        case 1: configuration = ALLEGRO_CHANNEL_CONF_1; break;
        case 2: configuration = ALLEGRO_CHANNEL_CONF_2; break;
        case 3: configuration = ALLEGRO_CHANNEL_CONF_3; break;
        case 4: configuration = ALLEGRO_CHANNEL_CONF_4; break;
        case 5: configuration = ALLEGRO_CHANNEL_CONF_5_1; break;
        case 6: configuration = ALLEGRO_CHANNEL_CONF_6_1; break;
        case 7: configuration = ALLEGRO_CHANNEL_CONF_7_1; break;
        default: configuration = ALLEGRO_CHANNEL_CONF_2; break;
    }
    stream = al_create_audio_stream(4, DUMB_SAMPLES, frequency, ALLEGRO_AUDIO_DEPTH_INT16,  configuration);
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
#ifdef PS3
static const int BUFFER_SIZE = 1024 * 4;
#else
static const int BUFFER_SIZE = 1024 * 16;
#endif

// static const int BUFFER_SIZE = 65536 * 2;
Encoding formatType(int sdlFormat){
    switch (sdlFormat){
        case AUDIO_U8: return Unsigned8;
        case AUDIO_S16SYS: return Signed16;
#if SDL_VERSION_ATLEAST(1, 3, 0)
        case AUDIO_F32MSB: return Float32;
        case AUDIO_F32LSB: return Float32;
#endif
    }

    std::ostringstream out;
    out << "Don't know how to deal with SDL format " << sdlFormat << std::endl;
    throw MusicException(__FILE__, __LINE__, out.str());

    /*
    if (bigEndian()){
        switch (Sound::Info.format){
            case AUDIO_S16MSB: return Signed16;
        }
        return Signed16;
    } else {
        switch (Sound::Info.format){
            case AUDIO_S16LSB: return Signed16;
        }
        return Signed16;
    }
    */
}
MusicRenderer::MusicRenderer():
convert(formatType(AUDIO_S16SYS), Sound::Info.channels, Sound::Info.frequency,
        formatType(Sound::Info.format), Sound::Info.channels, Sound::Info.frequency){
    create(Sound::Info.frequency, Sound::Info.channels);
}

MusicRenderer::MusicRenderer(int frequency, int channels):
convert(formatType(AUDIO_S16SYS), channels, frequency,
        formatType(Sound::Info.format), Sound::Info.channels, Sound::Info.frequency){
    create(frequency, channels);
}

void MusicRenderer::create(int frequency, int channels){
    
    // Global::debug(1) << "Convert between " << format << ", " << channels << ", " << frequency << " to " << Sound::Info.format << ", " << Sound::Info.channels << ", " << Sound::Info.frequency << std::endl;
    /*
    SDL_BuildAudioCVT(&convert, format, channels, frequency,
                                Sound::Info.format, Sound::Info.channels,
                                Sound::Info.frequency);
                                */
    int size = convert.convertedLength(BUFFER_SIZE);
    data = new Uint8[size < BUFFER_SIZE ? BUFFER_SIZE : size];
    position = 0;
    converted = 0;
}

static int sampleSize(){
    int size = 1;
    switch (Sound::Info.format){
        case AUDIO_U8:
        case AUDIO_S8: size = 1; break;
        case AUDIO_U16LSB:
        case AUDIO_S16LSB:
        case AUDIO_U16MSB:
        case AUDIO_S16MSB: size = 2; break;
#if SDL_VERSION_ATLEAST(1, 3, 0)
        case AUDIO_S32LSB:
        case AUDIO_S32MSB:
        case AUDIO_F32LSB:
        case AUDIO_F32MSB: size = 2; break;
#endif
        default: size = 2; break;
    }
    return size * Sound::Info.channels;
}

void MusicRenderer::fill(MusicPlayer * player){
    position = 0;
    /* read samples in dual-channel, 16-bit, signed form */
    // TimeDifference time;
    // time.startTime();
    player->render(data, BUFFER_SIZE / 4);
    // time.endTime();
    // Global::debug(0) << time.printTime("Render time") << std::endl;

    converted = convert.convert(data, BUFFER_SIZE);
}

void MusicRenderer::read(MusicPlayer * player, Uint8 * stream, int bytes){
    // Global::debug(0) << "Need " << bytes << " bytes. Have " << (converted - position) << std::endl;
    while (bytes > 0){
        int length = bytes;
        if (length + position >= converted){
            length = converted - position;
        }
        /*
        if (length % 4 != 0){
            length -= length % 4;
            if (length == 0){
                fill(player);
                continue;
            }
        }
        */

        // Global::debug(0) << "Copy " << length << " bytes" << std::endl;
        /* data contains samples in the same format as the output */
        memcpy(stream, data + position, length);
        stream += length;
        position += length;
        bytes -= length;
        if (position >= converted){
            fill(player);
        }
    }
}

void MusicRenderer::mixer(void * arg, Uint8 * stream, int bytes){
    MusicPlayer * player = (MusicPlayer*) arg;

    player->getRenderer()->read(player, stream, bytes);
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
    delete[] data;
}
#elif USE_ALLEGRO
int BUFFER_SIZE = 1 << 11;
static int ALLEGRO_MONO = 0;
static int ALLEGRO_STEREO = 1;
MusicRenderer::MusicRenderer(){
    create(Sound::Info.frequency, 2);
}

MusicRenderer::MusicRenderer(int frequency, int channels){
    create(frequency, channels);
}

void MusicRenderer::create(int frequency, int channels){
    int configuration = ALLEGRO_STEREO;
    if (channels == 1){
        configuration = ALLEGRO_MONO;
    }
    stream = play_audio_stream(BUFFER_SIZE, 16, configuration, frequency, 255, 128);
    if (!stream){
        throw MusicException(__FILE__, __LINE__, "Could not create Allegro stream");
    }
    if (stream->len != BUFFER_SIZE){
        throw MusicException(__FILE__, __LINE__, "Buffer size mismatch");
    }
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
    
void MusicPlayer::setRenderer(const ReferenceCount<MusicRenderer> & what){
    this->out = what;
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

DumbPlayer::DumbSystem::DumbSystem(){
}

DumbPlayer::DumbSystem::~DumbSystem(){
}

class StreamingSystem: public DumbPlayer::DumbSystem {
public:
    StreamingSystem(const Util::ReferenceCount<Storage::File> & file):
    file(file),
    dumb(NULL){
        system.open = NULL;
        system.skip = skip;
        system.getc = getc;
        system.getnc = getnc;
        system.close = close;
    }

    int doSkip(long n){
        file->seek(n, SEEK_CUR);
        return 0;
    }

    static int skip(void *f, long n){
        StreamingSystem * self = (StreamingSystem*) f;
        return self->doSkip(n);
    }

    int doGetc(){
        unsigned char x = 0;
        file->readLine((char*) &x, 1);
        return x;
    }

    static int getc(void *f){
        StreamingSystem * self = (StreamingSystem*) f;
        return self->doGetc();
    }

    int doGetnc(unsigned char * ptr, long n){
        return file->readLine((char*) ptr, n);
    }

    static long getnc(unsigned char *ptr, long n, void *f){
        StreamingSystem * self = (StreamingSystem*) f;
        return self->doGetnc(ptr, n);
    }

    void doClose(){
    }

    static void close(void *f){
        StreamingSystem * self = (StreamingSystem*) f;
        return self->doClose();
    }

    void closeDumb(){
        if (dumb != NULL){
            dumbfile_close(dumb);
            dumb = NULL;
        }
    }

    void reset(){
        file->reset();
        file->seek(0, SEEK_SET);
    }

    DUH * load(DUH * (*reader)(DUMBFILE *)){
        closeDumb();
        reset();
        dumb = dumbfile_open_ex(this, &system);
        return reader(dumb);
    }

    virtual ~StreamingSystem(){
        closeDumb();
    }

    /* Keep a reference so the file doesn't close */
    Util::ReferenceCount<Storage::File> file;

    DUMBFILE_SYSTEM system;
    DUMBFILE * dumb;

    DUH * loadDumbFile(){
        DUH * what = NULL;
        for (int i = 0; i < 4; i++){
            /* the order of trying xm/s3m/it/mod matters because mod could be
             * confused with one of the other formats, so load it last.
             */
            switch (i){
                case 0: {
                    what = load(dumb_read_xm_quick);
                    break;
                }
                case 1: {
                    what = load(dumb_read_s3m_quick);
                    break;
                }
                case 2: {
                    what = load(dumb_read_it_quick);
                    break;
                }
                case 3: {
                    what = load(dumb_read_mod_quick);
                    break;
                }
            }
            if (what != NULL){
                return what;
            }
        }

        return NULL;
    }

};

class MemorySystem: public DumbPlayer::DumbSystem {
public:
    MemorySystem(const Util::ReferenceCount<Storage::File> & file):
    memory(NULL),
    position(0),
    dumb(NULL){
        length = file->getSize();
        if (length == 0){
            throw MusicException(__FILE__, __LINE__, "Length was 0");
        }
        memory = new unsigned char[length];
        if (file->readLine((char*) memory, length) != length){
            throw MusicException(__FILE__, __LINE__, "Could not read entire file");
        }

        system.open = NULL;
        system.skip = skip;
        system.getc = getc;
        system.getnc = getnc;
        system.close = close;
    }

    virtual ~MemorySystem(){
        closeDumb();
        delete[] memory;
    }

    unsigned char * memory;
    int length;
    int position;

    DUMBFILE_SYSTEM system;
    DUMBFILE * dumb;

    int doSkip(long n){
        position += n;
        if (position > length){
            position = length;
        }
        if (position < 0){
            position = 0;
        }
        return 0;
    }

    static int skip(void *f, long n){
        MemorySystem * self = (MemorySystem*) f;
        return self->doSkip(n);
    }

    int doGetc(){
        if (position < length){
            unsigned char out = memory[position];
            position += 1;
            return out;
        }
        return -1;
    }

    static int getc(void *f){
        MemorySystem * self = (MemorySystem*) f;
        return self->doGetc();
    }

    int doGetnc(unsigned char * ptr, long n){
        int actual = n;
        if (actual + position >= length){
            actual = length - position;
        }
        memcpy(ptr, memory + position, actual);
        position += actual;
        return actual;
    }

    static long getnc(unsigned char *ptr, long n, void *f){
        MemorySystem * self = (MemorySystem*) f;
        return self->doGetnc(ptr, n);
    }

    void doClose(){
    }

    static void close(void *f){
        MemorySystem * self = (MemorySystem*) f;
        return self->doClose();
    }

    void closeDumb(){
        if (dumb != NULL){
            dumbfile_close(dumb);
            dumb = NULL;
        }
    }

    void reset(){
        position = 0;
    }

    DUH * load(DUH * (*reader)(DUMBFILE *)){
        closeDumb();
        reset();
        dumb = dumbfile_open_ex(this, &system);
        return reader(dumb);
    }

    DUH * loadDumbFile(){
        DUH * what = NULL;
        for (int i = 0; i < 4; i++){
            /* the order of trying xm/s3m/it/mod matters because mod could be
             * confused with one of the other formats, so load it last.
             */
            switch (i){
                case 0: {
                    what = load(dumb_read_xm_quick);
                    break;
                }
                case 1: {
                    what = load(dumb_read_s3m_quick);
                    break;
                }
                case 2: {
                    what = load(dumb_read_it_quick);
                    break;
                }
                case 3: {
                    what = load(dumb_read_mod_quick);
                    break;
                }
            }
            if (what != NULL){
                return what;
            }
        }

        return NULL;
    }
};

/* expects each sample to be 4 bytes, 2 bytes per sample * 2 channels */
DumbPlayer::DumbPlayer(const Filesystem::AbsolutePath & path){
    Util::ReferenceCount<Storage::File> file = Storage::instance().open(path);
    if (file == NULL){
        throw MusicException(__FILE__, __LINE__, "Could not open " + path.path());
    }

    if (file->canStream()){
        system = Util::ReferenceCount<DumbSystem>(new StreamingSystem(file));
    } else {
        system = Util::ReferenceCount<DumbSystem>(new MemorySystem(file));
    }

    music_file = system->loadDumbFile();
    if (music_file == NULL){
        std::ostringstream error;
        error << "Could not load DUMB file " << path.path();
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
    double delta = 65536.0 / Sound::Info.frequency;
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

#if 0
DUH * DumbPlayer::loadDumbFile(string path){
    DUH * what;
    for (int i = 0; i < 4; i++){
        /* the order of trying xm/s3m/it/mod matters because mod could be
         * confused with one of the other formats, so load it last.
         */
        switch (i){
            case 0 : {
                what = dumb_load_xm_quick(path.c_str());
                break;
            }
            case 1 : {
                what = dumb_load_s3m_quick(path.c_str());
                break;
            }
            case 2 : {
                what = dumb_load_it_quick(path.c_str());
                break;
            }
            case 3 : {
                what = dumb_load_mod_quick(path.c_str());
                break;
            }
        }
        
        if (what != NULL){
            Global::debug(1) << "Loaded " << path << " type " << typeToExtension(i) << "(" << i << ")" << std::endl;
            return what;
        }
    }
    return NULL;
}
#endif

GMEPlayer::GMEPlayer(string path):
emulator(NULL){
    gme_err_t fail = gme_open_file(path.c_str(), &emulator, Sound::Info.frequency);
    if (fail != NULL){
        Global::debug(0) << "GME load error for " << path << ": " << fail << std::endl;
        throw MusicException(__FILE__, __LINE__, "Could not load GME file");
    }
    emulator->start_track(0);
    Global::debug(1) << "Loaded GME file " << path << std::endl;
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
struct Mpg123FileIO{
    ssize_t (*read)(void *, void *, size_t);
    off_t (*seek)(void *, off_t, int);
    void (*close)(void *);
};

/* TODO: review this method, I'm not sure we need to reset the decoder after opening.
 * Also do we need to close or will open_handle do it for us?
 */
static void openMpg123(mpg123_handle ** mp3, const Mpg123FileIO & fileIO, Mpg123Handler * handler){
    int error = mpg123_replace_reader_handle(*mp3, fileIO.read, fileIO.seek, fileIO.close);
    /* stream has progressed a little bit so reset it by opening it again */
    error = mpg123_open_handle(*mp3, handler);
    if (error == -1){
        std::ostringstream fail;
        fail << "Could not open mpg123 file " << handler->name() << " error code " << error;
        throw MusicException(__FILE__,__LINE__, fail.str());
    }
    // error = mpg123_replace_reader_handle(*mp3, fileIO.read, fileIO.seek, fileIO.close);
    /* FIXME end */

    /* some of the native decoders aren't stable in older versions of mpg123
     * so just use generic for now. 1.13.1 should work better
     */
    error = mpg123_decoder(*mp3, "generic");
    if (error != MPG123_OK){
        std::ostringstream fail;
        fail << "Could not use 'generic' mpg123 decoder for " << handler->name() << " error code " << error;
        throw MusicException(__FILE__,__LINE__, fail.str());
    }
}

static void initializeMpg123(mpg123_handle ** mp3, const Mpg123FileIO & fileIO, Mpg123Handler * handler){
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
        int error = mpg123_format(*mp3, Sound::Info.frequency, MPG123_STEREO, MPG123_ENC_SIGNED_16);
        if (error != MPG123_OK){
            Global::debug(0) << "Could not set format for mpg123 handle" << std::endl;
        }

        error = mpg123_replace_reader_handle(*mp3, fileIO.read, fileIO.seek, fileIO.close);
        if (error == -1){
            std::ostringstream fail;
            fail << "Could not open mpg123 file " << handler->name() << " error code " << error;
            throw MusicException(__FILE__,__LINE__, fail.str());
        }
        
        /* FIXME workaround for libmpg issues with "generic" decoder frequency not being set */

        /* These next two lines work around an uninitialized memory problem in mpg123 1.12.1. The issue does not exist in later versions of mpg123.
         * Specifically in parse.c is the following code
         *    unsigned char *newbuf = fr->bsspace[fr->bsnum]+512;
         * Without a call to frame_reset, fr->bsnum will be uninitialized. mpg123 1.12.1 did not call frame_reset() any time between mpg123_open_handle and mpg123_read().
         * mpg123_open_fd on the other hand will call frame_reset() so we call mpg123_open_fd to force it. We make an extra mpg123_close call to undo the state changes associated with mpg123_open_fd.
         */
        error = mpg123_open_fd(*mp3, 0);
        mpg123_close(*mp3);

        error = mpg123_open_handle(*mp3, handler);
        if (error == -1){
            std::ostringstream fail;
            fail << "Could not open mpg123 file " << handler->name() << " error code " << error;
            throw MusicException(__FILE__,__LINE__, fail.str());
        }
        
        /* reading a frame is the only surefire way to get mpg123 to set the
         * sampling_frequency which it needs to set the decoder a few lines below
         */
        size_t dont_care;
        unsigned char tempBuffer[4096];
        error = mpg123_read(*mp3, tempBuffer, sizeof(tempBuffer), &dont_care);
	if (!(error == MPG123_OK || error == MPG123_NEW_FORMAT)){
            std::ostringstream fail;
            fail << "Could not read mpg123 file " << handler->name() << " error code " << error;
            throw MusicException(__FILE__,__LINE__, fail.str());
        }
	mpg123_close(*mp3);
	
        error = mpg123_replace_reader_handle(*mp3, fileIO.read, fileIO.seek, fileIO.close);
        /* stream has progressed a little bit so reset it by opening it again */
        error = mpg123_open_handle(*mp3, handler);
        if (error == -1){
            std::ostringstream fail;
            fail << "Could not open mpg123 file " << handler->name() << " error code " << error;
            throw MusicException(__FILE__,__LINE__, fail.str());
        }
        // error = mpg123_replace_reader_handle(*mp3, fileIO.read, fileIO.seek, fileIO.close);
        /* FIXME end */

        /* some of the native decoders aren't stable in older versions of mpg123
         * so just use generic for now. 1.13.1 should work better
         */
        error = mpg123_decoder(*mp3, "generic");
        if (error != MPG123_OK){
            std::ostringstream fail;
            fail << "Could not use 'generic' mpg123 decoder for " << handler->name() << " error code " << error;
            throw MusicException(__FILE__,__LINE__, fail.str());
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

/* initialize the mpg123 library and open up an mp3 file for reading */
static void initializeMpg123(mpg123_handle ** mp3, const Filesystem::AbsolutePath & path){
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
        int error = mpg123_format(*mp3, Sound::Info.frequency, MPG123_STEREO, MPG123_ENC_SIGNED_16);
        if (error != MPG123_OK){
            Global::debug(0) << "Could not set format for mpg123 handle" << std::endl;
        }
        
        /* FIXME workaround for libmpg issues with "generic" decoder frequency not being set */
        error = mpg123_open(*mp3, (char*) path.path().c_str());
        if (error == -1){
            std::ostringstream error;
            error << "Could not open mpg123 file " << path.path() << " error code " << error;
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
            error << "Could not read mpg123 file " << path.path() << " error code " << error;
            throw MusicException(__FILE__,__LINE__, error.str());
        }
	mpg123_close(*mp3);
	
        /* stream has progressed a little bit so reset it by opening it again */
	error = mpg123_open(*mp3, (char*) path.path().c_str());
        if (error == -1){
            std::ostringstream error;
            error << "Could not open mpg123 file " << path.path() << " error code " << error;
            throw MusicException(__FILE__,__LINE__, error.str());
        }
        /* FIXME end */

        /* some of the native decoders aren't stable in older versions of mpg123
         * so just use generic for now. 1.13.1 should work better
         */
        error = mpg123_decoder(*mp3, "generic");
        if (error != MPG123_OK){
            std::ostringstream error;
            error << "Could not use 'generic' mpg123 decoder for " << path.path() << " error code " << error;
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

class StreamMpg123Handler: public Mpg123Handler {
public:
    StreamMpg123Handler(const Util::ReferenceCount<Storage::File> & file):
    file(file){
        initializeMpg123(&mp3, mpg123IO(), this);
        long rate = 0;
        int channels = 0, encoding = 0;
        mpg123_getformat(mp3, &rate, &channels, &encoding);
    }
    
    /* Keep a reference to the file so it doesn't close */
    Util::ReferenceCount<Storage::File> file;
    
    virtual void reopen(){
        file->reset();
        openMpg123(&mp3, mpg123IO(), this);
    }

    ssize_t doRead(char * buffer, size_t bytes){
        return file->readLine(buffer, bytes);
    }

    static ssize_t read(void * handle, void * buffer, size_t bytes){
        StreamMpg123Handler * self = (StreamMpg123Handler*) handle;
        return self->doRead((char*) buffer, bytes);
    }

    off_t doSeek(off_t offset, int whence){
        return file->seek(offset, whence);
    }

    static off_t seek(void * handle, off_t offset, int whence){
        StreamMpg123Handler * self = (StreamMpg123Handler*) handle;
        return self->doSeek(offset, whence);
    }

    void doClose(){
        file->seek(0, SEEK_SET);
    }

    static void close(void * handle){
        StreamMpg123Handler * self = (StreamMpg123Handler*) handle;
        return self->doClose();
    }

    Mpg123FileIO mpg123IO(){
        Mpg123FileIO io;
        io.read = read;
        io.seek = seek;
        io.close = close;
        return io;
    }
    
    virtual std::string name() const {
        return "stream mp3";
    }
};

class MemoryMpg123Handler: public Mpg123Handler {
public:
    MemoryMpg123Handler(const Util::ReferenceCount<Storage::File> & file):
    memory(NULL){
        initializeMemory(file);
        initializeMpg123(&mp3, mpg123IO(), this);
        long rate = 0;
        int channels = 0, encoding = 0;
        mpg123_getformat(mp3, &rate, &channels, &encoding);
    }

    virtual ~MemoryMpg123Handler(){
        delete[] memory;
    }
    
    virtual void reopen(){
        position = 0;
        openMpg123(&mp3, mpg123IO(), this);
    }

    void initializeMemory(const Util::ReferenceCount<Storage::File> & file){
        length = file->getSize();
        if (length == 0){
            throw MusicException(__FILE__, __LINE__, "Length of file was 0");
        }
        memory = new char[length];
        if (file->readLine(memory, length) != length){
            throw MusicException(__FILE__, __LINE__, "Could not read entire file");
        }
        position = 0;
    }

    ssize_t doRead(char * buffer, size_t bytes){
        int actual = bytes;
        if (actual + position >= length){
            actual = length - position;
        }
        memcpy(buffer, memory + position, actual);
        position += actual;
        return actual;
    }

    static ssize_t read(void * handle, void * buffer, size_t bytes){
        MemoryMpg123Handler * self = (MemoryMpg123Handler*) handle;
        return self->doRead((char*) buffer, bytes);
    }

    off_t doSeek(off_t offset, int whence){
        switch (whence){
            case SEEK_SET: position = offset; break;
            case SEEK_CUR: position += offset; break;
            case SEEK_END: position = length + offset; break;
        }
        if (position < 0){
            position = 0;
        }
        if (position > length){
            position = length;
        }
        return position;
    }

    static off_t seek(void * handle, off_t offset, int whence){
        MemoryMpg123Handler * self = (MemoryMpg123Handler*) handle;
        return self->doSeek(offset, whence);
    }

    void doClose(){
        position = 0;
    }

    static void close(void * handle){
        MemoryMpg123Handler * self = (MemoryMpg123Handler*) handle;
        return self->doClose();
    }

    Mpg123FileIO mpg123IO(){
        Mpg123FileIO io;
        io.read = read;
        io.seek = seek;
        io.close = close;
        return io;
    }
 
    virtual std::string name() const {
        return "memory mp3";
    }

    char * memory;
    int length;
    int position;
};

Mpg123Handler::Mpg123Handler():
mp3(NULL){
}

void Mpg123Handler::read(void * data, int samples){
    /* buffer * 4 for 16 bits per sample * 2 samples for stereo */
    size_t out = 0;
    if (mpg123_read(mp3, (unsigned char *) data, samples * 4, &out) == MPG123_DONE){
        /* What if reopening fails? */
        reopen();

        /* Don't get into an infinite loop */
        if (out != 0){
            /* samples left = (total bytes - bytes read) / 4
             * total bytes = samples * 4
             * bytes read = out
             */
            read((char*) data + out, (samples * 4 - out) / 4);
        }
    }
}

void Mpg123Handler::setVolume(double volume){
    mpg123_volume(mp3, volume);
}

Mpg123Handler::~Mpg123Handler(){
    mpg123_close(mp3);
    mpg123_exit();
}

// static const int MPG123_BUFFER_SIZE = 1 << 11;
Mp3Player::Mp3Player(const Filesystem::AbsolutePath & path){
    Util::ReferenceCount<Storage::File> file = Storage::instance().open(path);
    if (file == NULL){
        throw MusicException(__FILE__, __LINE__, "Could not open mp3 " + path.path());
    }

    if (file->canStream()){
        handler = new StreamMpg123Handler(file);
    } else {
        handler = new MemoryMpg123Handler(file);
    }
    // handler = new MemoryMpg123Handler(file);
}

void Mp3Player::render(void * data, int samples){
    handler->read(data, samples);

    /*
       long rate;
       int channels, encoding;
       mpg123_getformat(mp3, &rate, &channels, &encoding);
       Global::debug(0) << "rate " << rate << " channels " << channels << " encoding " << encoding << std::endl;
       */
}

void Mp3Player::setVolume(double volume){
    handler->setVolume(volume);
    /*
    this->volume = volume;
    // mpg123_volume(mp3, volume * base_volume / 5000);
    mpg123_volume(mp3, 0.0001);
    */
    // mpg123_volume(mp3, volume);
}

Mp3Player::~Mp3Player(){
}

#endif /* MP3_MPG123 */

#ifdef HAVE_OGG
int OGG_BUFFER_SIZE = 1024 * 32;
OggPlayer::OggPlayer(string path):
path(path){
    file = fopen(path.c_str(), "rb");
    if (!file) {
        throw MusicException(__FILE__, __LINE__, "Could not open file");
    }

    if (ov_open_callbacks(file, &ogg, 0, 0, OV_CALLBACKS_DEFAULT) != 0) {
        fclose(file);
        throw MusicException(__FILE__, __LINE__, "Could not open ogg");
    }

    vorbis_info * info = ov_info(&ogg, -1);

    frequency = info->rate;
    channels = info->channels;
    bits = 16;
    length = ov_pcm_total(&ogg, -1);

    setRenderer(Util::ReferenceCount<MusicRenderer>(new MusicRenderer(info->rate, info->channels)));

    buffer = new OggPage();
    buffer->buffer1.buffer = new char[OGG_BUFFER_SIZE];
    // buffer->buffer2.buffer = new char[OGG_BUFFER_SIZE];

    fillPage(&buffer->buffer1);
    // fillPage(&buffer->buffer2);
    // buffer->use = 0;
}

void OggPlayer::fillPage(OggPage::Page * page){
    int dont_care;
    page->position = 0;
    page->max = 0;
    while (page->max < OGG_BUFFER_SIZE){
        /* ov_read might not read all available samples, I guess it stops
         * reading on a page boundary. We just plow on through.
         */
        int read = ov_read(&ogg, (char*) page->buffer + page->max, OGG_BUFFER_SIZE - page->max,
                           bigEndian(), 2, 1, &dont_care);
        /* if we hit the end of the file then re-open it and keep reading */
        if (read == 0){
            ov_clear(&ogg);
            file = fopen(path.c_str(), "rb");
            if (!file){
                throw MusicException(__FILE__, __LINE__, "Could not open file");
            }
            int ok = ov_open_callbacks(file, &ogg, 0, 0, OV_CALLBACKS_DEFAULT);
            if (ok != 0){
                fclose(file);
                throw MusicException(__FILE__, __LINE__, "Could not open ogg");
            }
        } else if (read == OV_HOLE){
            throw MusicException(__FILE__, __LINE__, "Garbage in ogg file");
        } else if (read == OV_EBADLINK){
            throw MusicException(__FILE__, __LINE__, "Invalid stream section in ogg");
        } else if (read == OV_EINVAL){
            throw MusicException(__FILE__, __LINE__, "File headers are corrupt in ogg");
        } else {
            page->max += read;
        }
    }
}

void OggPlayer::doRender(char * data, int bytes){
    OggPage::Page & page = buffer->buffer1;
    if (page.max - page.position >= bytes){
        memcpy(data, page.buffer + page.position, bytes);
        page.position += bytes;
    } else {
        /* copy the rest, fill the page, switch to the other buffer */
        memcpy(data, page.buffer + page.position, page.max - page.position);
        int at = page.max - page.position;
        int rest = bytes - (page.max - page.position);
        fillPage(&page);
        doRender(data + at, rest);
    }
}

void OggPlayer::render(void * data, int length){
    doRender((char*) data, length * 4);
}

void OggPlayer::setVolume(double volume){
    this->volume = volume;
    // Mix_VolumeMusic(volume * MIX_MAX_VOLUME);
}

OggPlayer::~OggPlayer(){
    /* ov_clear will close the file */
    ov_clear(&ogg);
}
#endif /* OGG */

#ifdef HAVE_MP3_MAD
Mp3Player::Mp3Player(const Filesystem::AbsolutePath & path):
available(NULL),
bytesLeft(0),
position(0),
raw(NULL){
    Util::ReferenceCount<Storage::File> file = Storage::instance().open(path);
    if (file == NULL){
        throw MusicException(__FILE__, __LINE__, "Could not open mp3 " + path.path());
    }

    rawLength = file->getSize();
    raw = new unsigned char[rawLength];
    if (file->readLine((char*) raw, rawLength) != rawLength){
        throw MusicException(__FILE__, __LINE__, "Could not read entire mp3 " + path.path());
    }
   
    int rate = 44100, channels = 2;
    discoverInfo(raw, rawLength, &rate, &channels);
    setRenderer(Util::ReferenceCount<MusicRenderer>(new MusicRenderer(rate, channels)));

    Global::debug(0) << "Opened mp3 file " << path.path() << " rate " << rate << " channels " << channels << std::endl;

    mad_stream_init(&stream);
    mad_frame_init(&frame);
    mad_synth_init(&synth);
    mad_stream_buffer(&stream, raw, rawLength);

    fill(4);
}

/* read the first frame and get the rate and channels from the header.
 * assume all other frames use the same rate and channels
 */ 
void Mp3Player::discoverInfo(unsigned char * raw, int length, int * rate, int * channels){
    mad_frame frame;
    mad_stream stream;
    mad_frame_init(&frame);
    mad_stream_init(&stream);
    mad_stream_buffer(&stream, raw, length);
    int ok = mad_header_decode(&frame.header, &stream);
    while (ok == -1){
        if (MAD_RECOVERABLE(stream.error)){
            ok = mad_header_decode(&frame.header, &stream);
        } else {
            throw MusicException(__FILE__, __LINE__, "Could not decode mp3 frame");
        }
    }
    *rate = frame.header.samplerate;
    switch (frame.header.mode){
        case MAD_MODE_SINGLE_CHANNEL: *channels = 1; break;
        case MAD_MODE_DUAL_CHANNEL: *channels = 2; break;
        case MAD_MODE_JOINT_STEREO: *channels = 2; break;
        case MAD_MODE_STEREO: *channels = 2; break;
    }

    mad_frame_finish(&frame);
    mad_stream_finish(&stream);
}

mad_flow Mp3Player::error(void * data, mad_stream * stream, mad_frame * frame){
    if (MAD_RECOVERABLE(stream->error)){
        return MAD_FLOW_CONTINUE;
    }
    throw MusicException(__FILE__, __LINE__, "Error decoding mp3 stream");
}

static inline signed int mad_scale(mad_fixed_t sample){
    /* round */
    sample += (1L << (MAD_F_FRACBITS - 16));

    /* clip */
    if (sample >= MAD_F_ONE)
        sample = MAD_F_ONE - 1;
    else if (sample < -MAD_F_ONE)
        sample = -MAD_F_ONE;

    /* quantize */
    return sample >> (MAD_F_FRACBITS + 1 - 16);
}

void Mp3Player::output(mad_header const * header, mad_pcm * pcm){
    unsigned int channels = pcm->channels;
    unsigned int samples = pcm->length;
    /*
    mad_fixed_t const * left = pcm->samples[0];
    mad_fixed_t const * right = pcm->samples[1];
    */

    unsigned short * out = new unsigned short[samples * channels];
    for (unsigned int index = 0; index < samples; index++){
        for (int channel = 0; channel < channels; channel++){
            mad_fixed_t const * left = pcm->samples[channel] + index;
            out[index * channels + channel] = mad_scale(*left) & 0xffff;
            // out[index * 2 + 1] = mad_scale(*right) & 0xffff;
        }
        // left += 1;
        // right += 1;
    }

    /* N channels * 2 bytes per sample */
    pages.push_back(Data((char*) out, samples * channels * 2));
}

mad_flow Mp3Player::input(void * data, mad_stream * stream){
    /*
    Mp3Player * player = (Mp3Player*) data;
    if (!player->readMore){
        return MAD_FLOW_STOP;
    } else {
        player->readMore = false;
    }
    int read = fread(player->raw, 1, RAW_SIZE, player->handle);
    if (feof(player->handle)){
        / * start over * /
        fseek(player->handle, 0, SEEK_SET);
    }
    mad_stream_buffer(stream, player->raw, read);
    return MAD_FLOW_CONTINUE;
    */
    return MAD_FLOW_CONTINUE;
}

void Mp3Player::fill(int frames){
    for (int i = 0; i < frames; i++){
        int headerError = mad_header_decode(&frame.header, &stream);
        while (headerError == -1){
            if (MAD_RECOVERABLE(stream.error)){
            } else {
                if (stream.error == MAD_ERROR_BUFLEN){
                    mad_stream_finish(&stream);
                    mad_frame_finish(&frame);
                    mad_synth_finish(&synth);

                    mad_stream_init(&stream);
                    mad_frame_init(&frame);
                    mad_synth_init(&synth);
                    mad_stream_buffer(&stream, raw, rawLength);
                }
            }
            headerError = mad_header_decode(&frame.header, &stream);
        }

        mad_frame_decode(&frame, &stream);
        mad_synth_frame(&synth, &frame);
        output(&frame.header, &synth.pcm);
    }

    /*
    readMore = true;
    int result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
    */

    bytesLeft = 0;
    for (std::vector<Data>::iterator it = pages.begin(); it != pages.end(); it++){
        bytesLeft += it->length;
    }

    // Global::debug(0) << "Read " << bytesLeft << std::endl;

    delete[] available;
    available = new char[bytesLeft];
    position = 0;
    int here = 0;
    for (std::vector<Data>::iterator it = pages.begin(); it != pages.end(); it++){
        memcpy(available + here, it->data, it->length);
        here += it->length;

        delete[] it->data;
    }

    // Global::debug(0) << "Filled mp3 with " << bytesLeft << std::endl;

    pages.clear();
}

void Mp3Player::render(void * data, int length){
    length *= 4;
    // Global::debug(0) << "Mp3 render " << length << " have " << bytesLeft << std::endl;
    while (length > 0){
        int left = length;
        if (left > bytesLeft){
            left = bytesLeft;
        }
        memcpy(data, available + position, left);
        length -= left;
        bytesLeft -= left;
        position += left;
        data = ((char*) data) + left;

        if (bytesLeft == 0){
            fill(2);
        }
    }
}

void Mp3Player::setVolume(double volume){
    /* TODO */
}

Mp3Player::~Mp3Player(){
    delete[] raw;
    delete[] available;
    mad_stream_finish(&stream);
    mad_frame_finish(&frame);
    mad_synth_finish(&synth);
    // mad_decoder_finish(&decoder);
}
#endif /* MP3_MAD */


}
