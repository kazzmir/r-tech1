#include "../sound.h"

Sound::Sound(){
    /* TODO */
}

/* create from wav file (riff header + pcm) */
Sound::Sound(const char * data, int length){
    /* TODO */
}

/* load from path */
Sound::Sound(const std::string & path) throw (LoadException){
    /* TODO */
}

Sound::Sound(const Sound & copy){
    /* TODO */
}

Sound & Sound::operator=( const Sound & rhs ){
    /* TODO */
    return *this;
}

void Sound::play(){
    /* TODO */
}

void Sound::play( int volume, int pan ){
    /* TODO */
}

void Sound::playLoop(){
    /* TODO */
}

void Sound::stop(){
    /* TODO */
}

Sound::~Sound(){
    /* TODO */
}
