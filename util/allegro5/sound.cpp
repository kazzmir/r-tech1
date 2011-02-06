#include "../sound.h"

Sound::Sound():
own(NULL){
}

/* create from wav file (riff header + pcm) */
Sound::Sound(const char * data, int length):
own(NULL){
    /* TODO */
    /*
    SDL_RWops * ops = SDL_RWFromConstMem(data, length);
    this->data.chunk = Mix_LoadWAV_RW(ops, 1);
    */
    own = new int;
    *own = 1;
}

/* load from path */
Sound::Sound(const std::string & path) throw (LoadException):
own(NULL){
    /* TODO */
    /*
    data.chunk = Mix_LoadWAV(path.c_str());
    if (!data.chunk){
        printf("Can't load sound %s\n", path.c_str());
        // throw LoadException("Could not load sound " + path);
    } else {
        own = new int;
        *own = 1;
    }
    */
}


void Sound::initialize(){
    /* TODO */
}

void Sound::uninitialize(){
    /* TODO */
}

void Sound::play(){
    /* TODO */
}

void Sound::play(double volume, int pan){
    /* TODO */
}

void Sound::playLoop(){
    /* TODO */
}

void Sound::stop(){
    /* TODO */
}

void Sound::destroy(){
    if (own){
        *own -= 1;
        if ( *own == 0 ){
            delete own;
            /* TODO */
            /*
            if (data.chunk != NULL){
                Mix_FreeChunk(data.chunk);
            }
            */
            own = NULL;
        }
    }
}
