#include "../bitmap.h"
#include <SDL.h>

static const int FULLSCREEN = 0;
/* bits per pixel */
static int SCREEN_DEPTH = 16;
static SDL_Surface * screen;

Bitmap * Bitmap::Screen = NULL;
static Bitmap * Scaler = NULL;
static Bitmap * Buffer = NULL;

Bitmap::Bitmap(){
    /* TODO */
}

Bitmap::Bitmap( int x, int y ){
    /* TODO */
}

Bitmap::Bitmap( const char * load_file ){
    /* TODO */
}

Bitmap::Bitmap( const std::string & load_file ){
    /* TODO */
}

Bitmap::Bitmap( const char * load_file, int sx, int sy ){
    /* TODO */
}

Bitmap::Bitmap( const char * load_file, int sx, int sy, double accuracy ){
    /* TODO */
}

Bitmap::Bitmap( const Bitmap & copy, bool deep_copy){
    /* TODO */
}

Bitmap::Bitmap( const Bitmap & copy, int sx, int sy ){
    /* TODO */
}

Bitmap::Bitmap( const Bitmap & copy, int sx, int sy, double accuracy ){
    /* TODO */
}

Bitmap::Bitmap( const Bitmap & copy, int x, int y, int width, int height ){
    /* TODO */
}

int Bitmap::getWidth() const {
    /* TODO */
    return 0;
}

int Bitmap::getHeight() const {
    /* TODO */
    return 0;
}

int Bitmap::getRed(int c){
    /* TODO */
    return 0;
}

int Bitmap::getBlue(int c){
    /* TODO */
    return 0;
}

int Bitmap::getGreen(int c){
    /* TODO */
    return 0;
}

int Bitmap::makeColor(int red, int blue, int green){
    /* TODO */
    return 0;
}
	
int Bitmap::setGraphicsMode(int mode, int width, int height){
    switch (mode){
        case FULLSCREEN : {
            screen = SDL_SetVideoMode(width, height, SCREEN_DEPTH, SDL_HWSURFACE);
            if (!screen){
                return 1;
            }
            break;
        }
    }

    if (SCALE_X == 0){
        SCALE_X = width;
    }
    if (SCALE_Y == 0){
        SCALE_Y = height;
    }
    if ( Screen != NULL ){
        delete Screen;
        Screen = NULL;
    }

    if ( Scaler != NULL ){
        delete Scaler;
        Scaler = NULL;
    }

    if ( Buffer != NULL ){
        delete Buffer;
        Buffer = NULL;
    }

    if (width != 0 && height != 0){
        Screen = new Bitmap(screen);
        if ( width != 0 && height != 0 && (width != SCALE_X || height != SCALE_Y) ){
            Scaler = new Bitmap(width, height);
            Buffer = new Bitmap(SCALE_X, SCALE_Y);
        }
    }

    return 0;
}

int Bitmap::setGfxModeFullscreen(int x, int y){
    return setGraphicsMode(FULLSCREEN, x, y);
}
	
void Bitmap::drawingMode(int type){
    /* TODO */
}
	
void Bitmap::transBlender( int r, int g, int b, int a ){
    /* TODO */
}
