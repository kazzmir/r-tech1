#include <allegro.h>
#ifdef _WIN32
#include <winalleg.h>
#endif

/*
#ifdef _WIN32
#define BITMAP dummyBITMAP
#include <windows.h>
#undef BITMAP
#endif
*/

struct BITMAP;

struct BitmapData{
    BitmapData():
        bitmap(0){}

    inline BITMAP * getBitmap() const {
        return bitmap;
    }

    inline void setBitmap(BITMAP * b) {
        bitmap = b;
    }

    BITMAP * bitmap;
};

namespace Graphics{
    typedef int Color;
}

#include "../software-renderer/bitmap.h"
