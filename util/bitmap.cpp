#include <allegro.h>

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
