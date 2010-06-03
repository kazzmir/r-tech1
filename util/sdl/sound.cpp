#include "../sound.h"
#include <SDL.h>
#include "mixer/SDL_mixer.h"

Sound::Sound():
own(NULL){
}

/* create from wav file (riff header + pcm) */
Sound::Sound(const char * data, int length):
own(NULL){
    /* TODO */
}

/* load from path */
Sound::Sound(const std::string & path) throw (LoadException):
own(NULL){
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
    // int audio_rate = 22050;
    /* allegro uses 44100 by default with alsa9 */
    int audio_rate = FREQUENCY;
    // int audio_rate = 22050;
    Uint16 audio_format = AUDIO_S16; 
    // Uint16 audio_format = MIX_DEFAULT_FORMAT; 
    int audio_channels = 2;
    int audio_buffers = 4096;
    // int audio_buffers = 44100;
    if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
        printf("Unable to open audio: %s!\n", Mix_GetError());
        // exit(1);
    }
}

void Sound::play(){
    if (data.chunk != NULL){
        /* FIXME: scale the volume based on the configuration sound setting */
        Mix_VolumeChunk(data.chunk, MIX_MAX_VOLUME);
        Mix_PlayChannel(-1, data.chunk, 0);
    }
}

void Sound::play(double volume, int pan){
    if (data.chunk != NULL){
        /* FIXME: scale the volume based on the configuration sound setting */
        Mix_VolumeChunk(data.chunk, (int)(volume * MIX_MAX_VOLUME));
        Mix_PlayChannel(-1, data.chunk, 0);
    }
}

void Sound::playLoop(){
    /* TODO */
}

void Sound::destroy(){
    if ( own ){
        *own -= 1;
        if ( *own == 0 ){
            delete own;
            if (data.chunk != NULL){
                Mix_FreeChunk(data.chunk);
            }
            own = NULL;
        }
    }
}

void Sound::stop(){
    if (data.channel != -1){
        Mix_HaltChannel(data.channel);
    }
}
