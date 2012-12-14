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

/* Filters only work for multiples of 320x240 up to 4x */
static bool filterMultiple(int width, int height, int toWidth, int toHeight){
    return (toWidth == width * 2 && toHeight == height * 2) ||
           (toWidth == width * 3 && toHeight == height * 3) ||
           (toWidth == width * 4 && toHeight == height * 4);
}

static void closestMultipleSize(int width, int height, int toWidth, int toHeight, int * outWidth, int * outHeight){
    /* If we are down-scaling then we don't need any filters */
    if (toWidth < width || toHeight < height){
        *outWidth = 1;
        *outHeight = 1;
    } else if (toWidth < width * 2 && toHeight < height * 2){
        *outWidth = 1;
        *outHeight = 1;
    } else if (toWidth < width * 3 && toHeight < height * 3){
        *outWidth = width * 2;
        *outHeight = height * 2;
    } else {
        *outWidth = width * 3;
        *outHeight = height * 3;
    }
}

StretchedBitmap::StretchedBitmap(int width, int height, const Bitmap & parent, QualityFilter filter):
Bitmap(1, 1),
width(1),
height(1),
where(parent),
filter(filter),
scaleToFilter(1, 1){
    if (width == parent.getWidth() && height == parent.getHeight()){
        setData(parent.getData());
    } else {
        updateSize(width, height);
    }
    
    if (!filterMultiple(width, height, parent.getWidth(), parent.getHeight()) &&
        filter != NoFilter){
        int multipleWidth, multipleHeight;
        closestMultipleSize(width, height, parent.getWidth(), parent.getHeight(), &multipleWidth, &multipleHeight);
        scaleToFilter.updateSize(multipleWidth, multipleHeight);
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
            case HqxFilter: {
                if (scaleToFilter.getWidth() > 1 &&
                    scaleToFilter.getHeight() > 1){
                    StretchHqx(scaleToFilter);
                    scaleToFilter.Stretch(where);
                } else {
                    StretchHqx(where);
                }
                break;
            }
            case XbrFilter: {
                if (scaleToFilter.getWidth() > 1 &&
                    scaleToFilter.getHeight() > 1){
                    StretchXbr(scaleToFilter);
                    scaleToFilter.Stretch(where);
                } else {
                    StretchXbr(where);
                }
                break;
            }
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
