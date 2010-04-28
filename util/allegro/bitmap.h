// #include <allegro.h>

#ifdef _WIN32
#define BITMAP dummyBITMAP
#include <windows.h>
#undef BITMAP
#endif

struct BITMAP;

struct BitmapData{
    inline BITMAP * getBitmap() const {
        return bitmap;
    }

    inline void setBitmap(BITMAP * b) {
        bitmap = b;
    }

    BITMAP * bitmap;
};
