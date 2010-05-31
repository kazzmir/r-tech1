#ifndef _paintown_music_player_h
#define _paintown_music_player_h

struct DUH;
#ifdef USE_ALLEGRO
struct AL_DUH_PLAYER;
#endif

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
#ifdef USE_ALLEGRO
    AL_DUH_PLAYER * player;
#endif
    DUH * music_file;

    double volume;
};

}

#endif
