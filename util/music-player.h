#ifndef _paintown_music_player_h
#define _paintown_music_player_h

#ifdef USE_SDL
/* for Uint8 */
#include <SDL.h>
#include "sdl/mixer/SDL_mixer.h"
#endif

struct DUH;
struct DUH_SIGRENDERER;
#ifdef USE_ALLEGRO
struct AL_DUH_PLAYER;
struct AUDIOSTREAM;
#endif
class Music_Emu;

namespace Util{

class MusicPlayer{
public:
    MusicPlayer();
    virtual void play() = 0;
    virtual void poll() = 0;
    virtual void pause() = 0;
    virtual void setVolume(double volume) = 0;
    virtual ~MusicPlayer();
};

/* uses the GME library, plays nintendo music files and others */
class GMEPlayer: public MusicPlayer {
public:
    GMEPlayer(const char * path);
    virtual void play();
    virtual void poll();
    virtual void pause();
    virtual void setVolume(double volume);
    virtual ~GMEPlayer();
protected:
#ifdef USE_SDL
    static void mixer(void * arg, Uint8 * stream, int length);
    void render(Uint8 * stream, int length);
#endif
#ifdef USE_ALLEGRO
    AUDIOSTREAM * stream;
#endif
    double volume;
    Music_Emu * emulator;
};

#ifdef HAVE_OGG
/* Maybe have some common sdl mixer class that this can inherit? */
class OggPlayer: public MusicPlayer {
public:
    OggPlayer(const char * path);
    virtual void play();

    virtual void poll();
    virtual void pause();
    virtual void setVolume(double volume);

    virtual ~OggPlayer();
protected:
#if USE_SDL
    Mix_Music * music;
#endif
};
#endif

/* interface to DUMB, plays mod/s3m/xm/it */
class DumbPlayer: public MusicPlayer {
public:
    DumbPlayer(const char * path);
    virtual void play();
    virtual void poll();
    virtual void pause();
    virtual void setVolume(double volume);

    virtual ~DumbPlayer();

protected:
    DUH * loadDumbFile(const char * path);

#ifdef USE_SDL
    static void mixer(void * player, Uint8 * stream, int length);
    void render(Uint8 * stream, int length);
#endif

protected:
#ifdef USE_ALLEGRO
    AL_DUH_PLAYER * player;
#endif
    DUH * music_file;

#ifdef USE_SDL
    DUH_SIGRENDERER * renderer;
#endif

    double volume;
};

}

#endif
