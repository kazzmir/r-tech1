/* generic software implementation of various bitmap operations */ 

namespace Graphics{

StretchedBitmap::StretchedBitmap(int width, int height, const Bitmap & parent):
Bitmap(width, height),
width(width),
height(height),
where(parent){
}

void StretchedBitmap::start(){
}

void StretchedBitmap::finish(){
    Stretch(where);
}

Bitmap getScreenBuffer(){
    return Bitmap(GFX_X, GFX_Y);
}

void resetDisplay(){
}

}
