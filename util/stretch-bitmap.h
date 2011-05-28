#ifndef _paintown_stretch_bitmap_h
#define _paintown_stretch_bitmap_h

#include "bitmap.h"

namespace Graphics{

class StretchedBitmap: public Bitmap {
public:
    StretchedBitmap(int width, int height, const Bitmap & where);
    void finish();
    void start();
protected:
    double width;
    double height;
    const Bitmap & where;
};

}

#endif
