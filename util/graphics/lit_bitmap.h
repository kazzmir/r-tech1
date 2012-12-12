#ifndef _paintown_lit_bitmap_h
#define _paintown_lit_bitmap_h

#include "bitmap.h"

namespace Graphics{

class LitBitmap: public Bitmap {
public:
    LitBitmap( const Bitmap & b );
    LitBitmap();
    virtual ~LitBitmap();

    using Bitmap::draw;
    virtual void draw(const int x, const int y, const Bitmap & where) const;
    virtual void draw(const int x, const int y, Filter * filter, const Bitmap & where ) const;
    using Bitmap::drawHFlip;
    virtual void drawHFlip( const int x, const int y, const Bitmap & where ) const;
    virtual void drawHFlip( const int x, const int y, Filter * filter, const Bitmap & where ) const;
    using Bitmap::drawVFlip;
    virtual void drawVFlip( const int x, const int y, const Bitmap & where ) const;
    virtual void drawVFlip( const int x, const int y, Filter * filter, const Bitmap & where ) const;
    using Bitmap::drawHVFlip;
    virtual void drawHVFlip( const int x, const int y, const Bitmap & where ) const;
    virtual void drawHVFlip( const int x, const int y, Filter * filter, const Bitmap & where ) const;
};

}

#endif
