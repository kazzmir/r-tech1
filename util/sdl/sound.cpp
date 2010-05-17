#include "../sound.h"
#include <SDL.h>
#include "mixer/SDL_mixer.h"

Sound::Sound():
own(NULL){
}

/* create from wav file (riff header + pcm) */
Sound::Sound(const char * data, int length){
    /* TODO */
}

/* load from path */
Sound::Sound(const std::string & path) throw (LoadException){
    data.chunk = Mix_LoadWAV(path.c_str());
    if (!data.chunk){
        printf("Can't load sound %s\n", path.c_str());
        // throw LoadException("Could not load sound " + path);
    } else {
        own = new int;
        *own = 1;
    }
}

void Sound::initialize(){
    int audio_rate = 22050;
    // Uint16 audio_format = AUDIO_S16; 
    Uint16 audio_format = MIX_DEFAULT_FORMAT; 
    int audio_channels = 2;
    int audio_buffers = 4096;
    if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
        printf("Unable to open audio: %s!\n", Mix_GetError());
        // exit(1);
    }
}

void Sound::play(){
    Mix_PlayChannel(-1, data.chunk, 0);
}

void Sound::play( int volume, int pan ){
    Mix_PlayChannel(-1, data.chunk, 0);
}

void Sound::playLoop(){
    /* TODO */
}

void Sound::destroy(){
    if ( own ){
        *own -= 1;
        if ( *own == 0 ){
            delete own;
            Mix_FreeChunk(data.chunk);
            own = NULL;
        }
    }
}

void Sound::stop(){
    /* TODO */
}
