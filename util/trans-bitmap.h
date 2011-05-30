#ifndef _paintown_trans_bitmap_h
#define _paintown_trans_bitmap_h

#include "bitmap.h"

namespace Graphics{

class TranslucentBitmap: public Bitmap {
public:
    TranslucentBitmap(const Bitmap & b);
    TranslucentBitmap();
    virtual ~TranslucentBitmap();

    virtual Color blendColor(const Color & color) const;
    virtual void startDrawing() const;
    virtual void endDrawing() const;

    virtual void putPixelNormal(int x, int y, Color col) const;
    virtual void rectangleFill(int x1, int y1, int x2, int y2, Color color) const;
    virtual void rectangle(int x1, int y1, int x2, int y2, Color color) const;
    virtual void fill(Color color) const;
    virtual void line( const int x1, const int y1, const int x2, const int y2, const Color color ) const;
    virtual void hLine( const int x1, const int y, const int x2, const Color color ) const;
    virtual void arc(const int x, const int y, const double ang1, const double ang2, const int radius, const Color color) const;
    virtual void arcFilled(const int x, const int y, const double ang1, const double ang2, const int radius, const Color color ) const;
    virtual void circleFill( int x, int y, int radius, Color color ) const;
    virtual void ellipse( int x, int y, int rx, int ry, Color color ) const;

    using Bitmap::draw;
    virtual void draw(const int x, const int y, const Bitmap & where) const;
    virtual void draw(const int x, const int y, Filter * filter, const Bitmap & where) const;
    // virtual void draw(const int x, const int y, const int startWidth, const int startHeight, const int width, const int height, const Bitmap & where) const;
    // virtual void draw(const int x, const int y, const int startWidth, const int startHeight, const int width, const int height, Filter * filter, const Bitmap & where) const;

    using Bitmap::drawHFlip;
    virtual void drawHFlip(const int x, const int y, const Bitmap & where) const;
    virtual void drawHFlip(const int x, const int y, Filter * filter, const Bitmap & where) const;
    // virtual void drawHFlip(const int x, const int y, const int startWidth, const int startHeight, const int width, const int height, const Bitmap & where) const;
    // virtual void drawHFlip(const int x, const int y, const int startWidth, const int startHeight, const int width, const int height, Filter * filter, const Bitmap & where) const;
    virtual void drawVFlip( const int x, const int y, const Bitmap & where ) const;
    virtual void drawVFlip( const int x, const int y, Filter * filter, const Bitmap & where ) const;
    virtual void drawHVFlip( const int x, const int y, const Bitmap & where ) const;
    virtual void drawHVFlip( const int x, const int y, Filter * filter, const Bitmap & where ) const;

};

}

#endif
