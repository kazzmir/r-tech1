/* generic software implementation of various bitmap operations */ 

namespace Graphics{

Color Bitmap::blendColor(const Color & input) const {
    return input;
}

Color TranslucentBitmap::blendColor(const Color & input) const {
    return input;
}

void Bitmap::startDrawing() const {
}

void Bitmap::endDrawing() const {
}

void TranslucentBitmap::startDrawing() const {
}

void TranslucentBitmap::endDrawing() const {
}

void Bitmap::lock() const {
}

void Bitmap::unlock() const {
}

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

RestoreState::RestoreState(){
}

RestoreState::~RestoreState(){
}

}
