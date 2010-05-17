#ifdef USE_ALLEGRO
#include "allegro/sound.cpp"
#endif
#ifdef USE_SDL
#include "sdl/sound.cpp"
#endif

Sound::Sound( const Sound & copy ):
my_sound( NULL ),
own( NULL ){
    own = copy.own;
    if ( own ){
        *own += 1;
    }
    my_sound = copy.my_sound;
}

Sound & Sound::operator=( const Sound & rhs ){
    if ( own ){
        destroy();
    }
    own = rhs.own;
    if ( own ){
        *own += 1;
    }
    my_sound = rhs.my_sound;

    return *this;
}


Sound::~Sound(){
    destroy();
}
