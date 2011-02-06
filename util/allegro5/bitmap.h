#include <allegro5/allegro.h>

struct BitmapData{
    BitmapData():
    bitmap(NULL){
    }

    inline ALLEGRO_BITMAP * getBitmap() const {
        return bitmap;
    }

    inline void setBitmap(ALLEGRO_BITMAP * b){
        bitmap = b;
    }

    ALLEGRO_BITMAP * bitmap;
};
