#include "../sound.h"
#include <allegro5/allegro5.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_memfile.h>
#include <allegro5/allegro_acodec.h>

Sound::Sound():
own(NULL){
}

/* create from wav file (riff header + pcm) */
Sound::Sound(const char * data, int length):
own(NULL){
    ALLEGRO_FILE * memory = al_open_memfile((void*) data, length, "r");
    this->data.sample = al_load_sample_f(memory, ".wav");
    al_fclose(memory);
    
    own = new int;
    *own = 1;
}

/* load from path */
Sound::Sound(const std::string & path) throw (LoadException):
own(NULL){
    data.sample = al_load_sample(path.c_str());
    if (data.sample == NULL){
    }
    own = new int;
    *own = 1;
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
    al_install_audio();
    al_init_acodec_addon();
    al_reserve_samples(8);
}

void Sound::uninitialize(){
}

void Sound::play(){
    if (data.sample != NULL){
        al_play_sample(data.sample, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
    }
}

void Sound::play(double volume, int pan){
    /* FIXME: deal with pan */
    if (data.sample != NULL){
        al_play_sample(data.sample, scale(volume), 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
    }
}

void Sound::playLoop(){
    if (data.sample != NULL){
        al_play_sample(data.sample, scale(1.0), 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);
    }
}

void Sound::stop(){
    /* TODO */
}

void Sound::destroy(){
    if (own){
        *own -= 1;
        if ( *own == 0 ){
            delete own;
            al_destroy_sample(data.sample);
            own = NULL;
        }
    }
}
