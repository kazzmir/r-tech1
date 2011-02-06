#ifdef USE_ALLEGRO
#include "allegro/sound.cpp"
#endif
#ifdef USE_SDL
#include "sdl/sound.cpp"
#endif
#ifdef USE_ALLEGRO5
#include "allegro5/sound.cpp"
#endif
        
int Sound::FREQUENCY = 22050;

Sound::Sound( const Sound & copy ):
own( NULL ){
    own = copy.own;
    if ( own ){
        *own += 1;
    }
    data = copy.data;
}

Sound & Sound::operator=( const Sound & rhs ){
    if ( own ){
        destroy();
    }
    own = rhs.own;
    if ( own ){
        *own += 1;
    }

    data = rhs.data;

    return *this;
}

Sound::~Sound(){
    destroy();
}
