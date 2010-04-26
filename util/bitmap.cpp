#include "bitmap.h"

/* implementation independant definitions can go here */

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
