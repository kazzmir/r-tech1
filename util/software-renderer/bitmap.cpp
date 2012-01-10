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

StretchedBitmap::StretchedBitmap(int width, int height, const Bitmap & parent, QualityFilter filter):
Bitmap(1, 1),
width(1),
height(1),
where(parent),
filter(filter){
    if (width == parent.getWidth() && height == parent.getHeight()){
        setData(parent.getData());
    } else {
        updateSize(width, height);
    }
    this->width = width;
    this->height = height;
}

void StretchedBitmap::start(){
}

void StretchedBitmap::finish(){
    if (getData() != where.getData()){
        switch (filter){
            case NoFilter: Stretch(where); break;
            case HqxFilter: StretchHqx(where); break;
            case XbrFilter: StretchXbr(where); break;
        }
    }
}

/*
Bitmap getScreenBuffer(){
    return Bitmap(GFX_X, GFX_Y);
}
*/

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
