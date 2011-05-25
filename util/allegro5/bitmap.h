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

namespace Graphics{
    typedef ALLEGRO_COLOR Color;
}

bool operator<(const ALLEGRO_COLOR color1, const ALLEGRO_COLOR color2);
bool operator!=(const ALLEGRO_COLOR color1, const ALLEGRO_COLOR color2);
bool operator==(const ALLEGRO_COLOR color1, const ALLEGRO_COLOR color2);
