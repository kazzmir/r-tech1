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

Bitmap Bitmap::scaleTo(const int width, const int height) const {
    if (width == getWidth() && height == getHeight()){
        return *this;
    }
    Bitmap scaled(width, height);
    this->Stretch(scaled);
    return scaled;
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

RestoreState::RestoreState(){
}

RestoreState::~RestoreState(){
}
    
TranslatedBitmap::TranslatedBitmap(int x, int y, const Bitmap & where):
Bitmap(where),
x(x),
y(y){
}
    
void TranslatedBitmap::BlitToScreen() const {
    Bitmap::BlitToScreen(x, y);
}

TranslatedBitmap::~TranslatedBitmap(){
}

}
