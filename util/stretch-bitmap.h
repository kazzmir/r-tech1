#ifndef _paintown_stretch_bitmap_h
#define _paintown_stretch_bitmap_h

#include "bitmap.h"

namespace Graphics{

class StretchedBitmap: public Bitmap {
public:
    StretchedBitmap(int width, int height, const Bitmap & where, QualityFilter filter = NoFilter);
    void finish();
    void start();
    virtual int getWidth() const;
    virtual int getHeight() const;

    virtual double getScaleWidth() const;
    virtual double getScaleHeight() const;

protected:
    double width;
    double height;
    double scale_x, scale_y;
    const Bitmap & where;
    const QualityFilter filter;
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
