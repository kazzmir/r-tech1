#ifndef _paintown_music_player_h
#define _paintown_music_player_h

#ifdef USE_SDL
/* for Uint8 */
#include <SDL.h>
#include "sdl/mixer/SDL_mixer.h"
#endif

#ifdef HAVE_MP3_MPG123
#include <mpg123.h>
#endif

#ifdef HAVE_MP3_MAD
#include <mad.h>
#endif

#include "pointer.h"

struct DUH;
struct DUH_SIGRENDERER;
#ifdef USE_ALLEGRO
struct AL_DUH_PLAYER;
struct AUDIOSTREAM;
struct LOGG_Stream;
#endif
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
    virtual ~MusicRenderer();
    void poll(MusicPlayer & player);
    void play(MusicPlayer & player);
    void pause();

protected:
#ifdef USE_SDL
    static void mixer(void * arg, Uint8 * stream, int length);
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

    /* length is in samples not bytes */
    virtual void render(void * stream, int length) = 0;

    virtual inline double getVolume() const {
        return volume;
    }

protected:
    double volume;
    ReferenceCount<MusicRenderer> out;
};

/* uses the GME library, plays nintendo music files and others */
class GMEPlayer: public MusicPlayer {
public:
    GMEPlayer(const char * path);
    virtual void setVolume(double volume);
    virtual ~GMEPlayer();
    virtual void render(void * stream, int length);

protected:
    Music_Emu * emulator;
};

#ifdef HAVE_OGG
/* Maybe have some common sdl mixer class that this can inherit? */
class OggPlayer: public MusicPlayer {
public:
    OggPlayer(const char * path);
    virtual void setVolume(double volume);
    virtual void render(void * stream, int length);

    virtual ~OggPlayer();
protected:
    /*
#ifdef USE_ALLEGRO
    struct LOGG_Stream * stream;
#endif
*/
};
#endif

#if defined (HAVE_MP3_MPG123) || defined (HAVE_MP3_MAD)
/* Interface for mp3s */
class Mp3Player: public MusicPlayer {
public:
    Mp3Player(const char * path);
    virtual void setVolume(double volume);
    virtual void render(void * data, int length);

    virtual ~Mp3Player();
protected:    
#ifdef HAVE_MP3_MPG123
    mpg123_handle * mp3;
    double base_volume;
#elif HAVE_MP3_MAD
#endif
};
#endif

/* interface to DUMB, plays mod/s3m/xm/it */
class DumbPlayer: public MusicPlayer {
public:
    DumbPlayer(const char * path);
    virtual void setVolume(double volume);
    virtual void render(void * data, int samples);

    virtual ~DumbPlayer();

protected:
    DUH * loadDumbFile(const char * path);

protected:
    DUH * music_file;
    DUH_SIGRENDERER * renderer;
};

}

#endif
