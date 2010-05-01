#ifdef USE_ALLEGRO
#include <allegro.h>
#endif

#include "bitmap.h"

/* implementation independant definitions can go here */

Bitmap * Bitmap::temporary_bitmap = NULL;

int Bitmap::SCALE_X = 0;
int Bitmap::SCALE_Y = 0;
const int Bitmap::MODE_TRANS = 0;
const int Bitmap::MODE_SOLID = 1;
	
const int Bitmap::SPRITE_NO_FLIP = 0;
const int Bitmap::SPRITE_V_FLIP = 1;
const int Bitmap::SPRITE_H_FLIP = 2;
	
const int Bitmap::SPRITE_NORMAL = 1;
const int Bitmap::SPRITE_LIT = 2;
const int Bitmap::SPRITE_TRANS = 3;

#ifdef USE_ALLEGRO
#include "allegro/bitmap.cpp"
#endif
#ifdef USE_SDL
#include "sdl/bitmap.cpp"
#endif

static inline int max(int a, int b){
    return a > b ? a : b;
}

Bitmap::~Bitmap(){
    releaseInternalBitmap();
}

Bitmap Bitmap::temporaryBitmap(int w, int h){
    if (temporary_bitmap == NULL){
        temporary_bitmap = new Bitmap(w, h);
    } else if (temporary_bitmap->getWidth() < w || temporary_bitmap->getHeight() < h){
        int mw = max(temporary_bitmap->getWidth(), w);
        int mh = max(temporary_bitmap->getHeight(), h);
        // printf("Create temporary bitmap %d %d\n", mw, mh);
        delete temporary_bitmap;
        temporary_bitmap = new Bitmap(mw, mh);
    }
    if (temporary_bitmap == NULL){
        printf("*bug* temporary bitmap is null\n");
    }
    return Bitmap(*temporary_bitmap, 0, 0, w, h);
}
        
void Bitmap::cleanupTemporaryBitmaps(){
    if (temporary_bitmap != NULL){
        delete temporary_bitmap;
        temporary_bitmap = NULL;
    }
}

double Bitmap::getScale(){
    /* the game is pretty much hard coded to run at 320 scaled upto 640
     * and then scaled to whatever the user wants, but as long as
     * 320 and 640 remain this number will be 2.
     * maybe calculate this at some point
     */
    return 2;
    /*
    if (Scaler != NULL && Buffer != NULL){
        double x1 = Scaler->getWidth();
        double x2 = Buffer->getWidth();
        return x2 / x1;
    }
    return 1;
    */
}

/* taken from the color addon from allegro 4.9 */
static void al_color_cmyk_to_rgb(float cyan, float magenta, float yellow, float key, float *red, float *green, float *blue){
    float max = 1 - key;
    *red = max - cyan * max;
    *green = max - magenta * max;
    *blue = max - yellow * max;
}

void Bitmap::cymkToRGB(int c, int y, int m, int k, int * r, int * g, int * b){
    float fc = (float)c / 255.0;
    float fy = (float)y / 255.0;
    float fm = (float)m / 255.0;
    float fk = (float)k / 255.0;

    float fr, fg, fb;
    al_color_cmyk_to_rgb(fc, fm, fy, fk, &fr, &fg, &fb);
    *r = (int)(fr * 255.0);
    *g = (int)(fg * 255.0);
    *b = (int)(fb * 255.0);
}

int Bitmap::getScreenWidth(){
    if (Screen != 0){
        return Screen->getWidth();
    }
    return 0;
}

int Bitmap::getScreenHeight(){
    if (Screen != 0){
        return Screen->getHeight();
    }
    return 0;
}

/* decrement bitmap reference counter and free memory if counter hits 0 */
void Bitmap::releaseInternalBitmap(){
    const int MAGIC_DEBUG = 0xa5a5a5;
    if (own != NULL){
        if (*own == MAGIC_DEBUG){
            printf("[bitmap] Trying to delete an already deleted reference counter %p\n", own);
        }

        (*own) -= 1;
        if ( *own == 0 ){
            *own = MAGIC_DEBUG;
            delete own;
            destroyPrivateData();
            own = NULL;
        }
    }
}

