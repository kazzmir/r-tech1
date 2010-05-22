#ifndef _paintown_trans_bitmap_h
#define _paintown_trans_bitmap_h

#include "bitmap.h"

class TranslucentBitmap: public Bitmap {
public:
    TranslucentBitmap(const Bitmap & b);
    TranslucentBitmap();
    virtual ~TranslucentBitmap();
	
    virtual void putPixelNormal(int x, int y, int col) const;
};

#endif
