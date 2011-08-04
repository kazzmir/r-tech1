#ifndef _paintown_music_player_h
#define _paintown_music_player_h

#include <string>
#include <vector>
#include <stdio.h>

#ifdef USE_SDL
/* for Uint8 */
#include <SDL.h>
#include "sdl/mixer/SDL_mixer.h"
#endif

#ifdef HAVE_MP3_MPG123
#include <mpg123.h>
#endif

#ifdef HAVE_OGG
#include <vorbis/vorbisfile.h>
#endif

#ifdef HAVE_MP3_MAD
#include <mad.h>
#endif

#include "pointer.h"

struct DUH;
struct DUH_SIGRENDERER;
#ifdef USE_ALLEGRO
struct AUDIOSTREAM;
#endif
struct LOGG_Stream;
class Music_Emu;

#ifdef USE_ALLEGRO5
struct ALLEGRO_AUDIO_STREAM;
struct ALLEGRO_EVENT_QUEUE;
#endif

namespace Util{

class MusicPlayer;
/* implemented by some backend: allegro4/sdl/allergo5 */
class MusicRenderer{
public:
    MusicRenderer();
    MusicRenderer(int frequency, int channels);
    virtual ~MusicRenderer();
    void poll(MusicPlayer & player);
    void play(MusicPlayer & player);
    void pause();

protected:
    void create(int frequency, int channels);
#ifdef USE_SDL
    static void mixer(void * arg, Uint8 * stream, int length);
    void fill(MusicPlayer * player);
    void read(MusicPlayer * player, Uint8 * stream, int bytes);
    SDL_AudioCVT convert;
    Uint8 * data;
    int position;
    int converted;
#endif

#ifdef USE_ALLEGRO
    AUDIOSTREAM * stream;
#endif

#ifdef USE_ALLEGRO5
    ALLEGRO_AUDIO_STREAM * stream;
    ALLEGRO_EVENT_QUEUE * queue;
#endif
};

class MusicPlayer{
public:
    MusicPlayer();
    virtual void play();
    virtual void poll();
    virtual void pause();
    virtual void setVolume(double volume) = 0;
    virtual ~MusicPlayer();

    /* length is in samples not bytes.
     * this function must produce samples that are
     *  signed
     *  16-bit
     *  use the native endian of the machine
     *  dual-channel
     *
     *  Which means each sample should be 4 bytes
     *    2 bytes * 2 channels
     */
    virtual void render(void * stream, int length) = 0;

    virtual inline double getVolume() const {
        return volume;
    }

    virtual const ReferenceCount<MusicRenderer> & getRenderer() const {
        return out;
    }
    virtual void setRenderer(const ReferenceCount<MusicRenderer> & what);

protected:
    double volume;
    ReferenceCount<MusicRenderer> out;
};

/* uses the GME library, plays nintendo music files and others */
class GMEPlayer: public MusicPlayer {
public:
    GMEPlayer(std::string path);
    virtual void setVolume(double volume);
    virtual ~GMEPlayer();
    virtual void render(void * stream, int length);

protected:
    Music_Emu * emulator;
};

#ifdef HAVE_OGG
struct OggPage{
    struct Page{
        int position;
        int max;
        char * buffer;

        ~Page(){
            delete[] buffer;
        }
    };
    Page buffer1;
    // Page buffer2;
    // int use;
};

/* Maybe have some common sdl mixer class that this can inherit? */
class OggPlayer: public MusicPlayer {
public:
    OggPlayer(std::string path);
    virtual void setVolume(double volume);
    virtual void render(void * stream, int length);

    virtual ~OggPlayer();
protected:
    void fillPage(OggPage::Page * page);
    void doRender(char * data, int bytes);
    FILE* file;
    std::string path;
    OggVorbis_File ogg;
    ReferenceCount<OggPage> buffer;

    int frequency;
    int channels;
    int bits;
    ogg_int64_t length;
};
#endif

#if defined (HAVE_MP3_MPG123) || defined (HAVE_MP3_MAD)
/* Interface for mp3s */
class Mp3Player: public MusicPlayer {
public:
    Mp3Player(std::string path);
    virtual void setVolume(double volume);
    virtual void render(void * data, int length);

    virtual ~Mp3Player();
protected:    
#ifdef HAVE_MP3_MPG123
    mpg123_handle * mp3;
    double base_volume;
#elif HAVE_MP3_MAD
    void output(mad_header const * header, mad_pcm * pcm);
    static mad_flow error(void * data, mad_stream * stream, mad_frame * frame);
    static mad_flow input(void * data, mad_stream * stream);
    void discoverInfo(FILE * handle, int * rate, int * channels);
    void fill();

    mad_stream stream;
    mad_frame frame;
    mad_synth synth;

    char * available;
    int bytesLeft;
    int position;
    bool readMore;
    struct Data{
        Data():
            data(NULL),
            length(0){
            }

        Data(char * data, int length):
            data(data), length(length){
            }

        ~Data(){
        }

        char * data;
        int length;
    };
    std::vector<Data> pages;
    unsigned char * raw;
    int rawLength;
#endif
};
#endif

/* interface to DUMB, plays mod/s3m/xm/it */
class DumbPlayer: public MusicPlayer {
public:
    DumbPlayer(std::string path);
    virtual void setVolume(double volume);
    virtual void render(void * data, int samples);

    virtual ~DumbPlayer();

protected:
    DUH * loadDumbFile(std::string path);

protected:
    DUH * music_file;
    DUH_SIGRENDERER * renderer;
};

}

#endif
