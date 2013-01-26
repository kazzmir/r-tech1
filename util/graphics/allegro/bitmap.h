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
        bitmap(0),
        destroy(true){}

    BitmapData(BITMAP * bitmap):
        bitmap(bitmap),
        destroy(true){
        }

    ~BitmapData(){
        if (bitmap != NULL && destroy){
            destroy_bitmap(bitmap);
        }
    }

    void setDestroy(bool b){
        destroy = b;
    }

    inline BITMAP * getBitmap() const {
        return bitmap;
    }

    inline void setBitmap(BITMAP * b) {
        bitmap = b;
    }

    BITMAP * bitmap;
    bool destroy;
};

namespace Graphics{
    typedef int INTERNAL_COLOR;
}

#include "../software-renderer/bitmap.h"
