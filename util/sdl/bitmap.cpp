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
}

int Bitmap::getWidth() const {
    return 0;
}

int Bitmap::getHeight() const {
    return 0;
}

int Bitmap::getRed(int c){
    return 0;
}

int Bitmap::getBlue(int c){
    return 0;
}

int Bitmap::getGreen(int c){
    return 0;
}

int Bitmap::makeColor(int red, int blue, int green){
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
