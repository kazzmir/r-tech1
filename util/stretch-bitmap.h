#ifndef _paintown_stretch_bitmap_h
#define _paintown_stretch_bitmap_h

#include "bitmap.h"

namespace Graphics{

class StretchedBitmap: public Bitmap {
public:
    StretchedBitmap(int width, int height, const Bitmap & where);
    void finish();
    void start();
    virtual int getWidth() const;
    virtual int getHeight() const;
protected:
    double width;
    double height;
    double scale_x, scale_y;
    const Bitmap & where;
};

class TranslatedBitmap: public Bitmap {
public:
    TranslatedBitmap(int x, int y, const Bitmap & where);
    using Bitmap::BlitToScreen;
    virtual void BlitToScreen() const;
    virtual ~TranslatedBitmap();
public:
    int x, y;
};

}

#endif
