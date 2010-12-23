#ifndef _paintown_trans_bitmap_h
#define _paintown_trans_bitmap_h

#include "bitmap.h"

class TranslucentBitmap: public Bitmap {
public:
    TranslucentBitmap(const Bitmap & b);
    TranslucentBitmap();
    virtual ~TranslucentBitmap();

    virtual void putPixelNormal(int x, int y, int col) const;
    virtual void rectangleFill(int x1, int y1, int x2, int y2, int color) const;
    virtual void rectangle(int x1, int y1, int x2, int y2, int color) const;
    virtual void fill(int color) const;
    virtual void hLine( const int x1, const int y, const int x2, const int color ) const;
    virtual void circleFill( int x, int y, int radius, int color ) const;
    virtual void ellipse( int x, int y, int rx, int ry, int color ) const;
};

#endif
