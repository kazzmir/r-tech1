#include "../sound.h"
#include <SDL.h>
#include "mixer/SDL_mixer.h"

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

void Sound::initialize(){
    int audio_rate = 22050;
    Uint16 audio_format = AUDIO_S16; 
    int audio_channels = 2;
    int audio_buffers = 4096;
    if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
        printf("Unable to open audio!\n");
        exit(1);
    }
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
