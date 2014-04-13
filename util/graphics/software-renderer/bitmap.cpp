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

int getAlpha(Color input){
    return 255;
}
        
Bitmap Bitmap::createMemoryBitmap(int width, int height){
    return Bitmap(width, height);
}

void Bitmap::drawShadow(Bitmap & where, int x, int y, int intensity, Color color, double scale, bool facingRight) const {
    const double newheight = getHeight() * scale;
    Bitmap shade(getWidth(), (int) fabs(newheight));
    Stretch(shade);

    /* Could be slow, but meh, lets do it for now to make it look like a real shadow */
    for (int h = 0; h < shade.getHeight(); ++h){
        for (int w = 0; w < shade.getWidth(); ++w){
            Color pix = shade.getPixel(w, h);
            if (pix != MaskColor()){
                shade.putPixel(w, h, color);
            }
        }
    }

    transBlender(0, 0, 0, intensity);

    if (scale > 0){
        if (facingRight){
            shade.translucent().drawVFlip(x, y, where);
        } else { 
            shade.translucent().drawHVFlip(x, y, where);
        }
    } else if (scale < 0){
        y -= fabs(newheight);
        if (facingRight){
            shade.translucent().draw(x + 3, y, where);
        } else { 
            shade.translucent().drawHFlip(x - 3, y, where);
        }
    }
}

void TranslucentBitmap::roundRect(int radius, int x1, int y1, int x2, int y2, Color color) const {
    Bitmap::roundRect(radius, x1, y1, x2, y2, color);
}

void Bitmap::roundRect(int radius, int x1, int y1, int x2, int y2, Color color) const {
    const int width = x2 - x1;
    const int height = y2 - y1;
    radius = Mid(0, radius, Min((x1+width - x1)/2, (y1+height - y1)/2));
    line(x1+radius, y1, x1+width-radius, y1, color);
    line(x1+radius, y1+height, x1+width-radius,y1+height, color);
    line(x1, y1+radius,x1, y1+height-radius, color);
    line(x1+width, y1+radius,x1+width, y1+height-radius, color);

    double quarterTurn = Util::pi / 2;
    double quadrant1 = 0;
    /* signs are flipped because the coordinate system is reflected over the y-axis */
    double quadrant2 = -Util::pi / 2;
    double quadrant3 = Util::pi;
    double quadrant4 = -3 * Util::pi / 2; 

    /* upper right. draw from 90 to 0 */
    arc(x1+radius + (width - radius *2), y1 + radius, quadrant1, quadrant1 + quarterTurn, radius, color);
    /* upper left. draw from 180 to 270 */
    arc(x1 + radius, y1 + radius, quadrant2, quadrant2 + quarterTurn, radius, color);
    /* lower left. draw from 180 to 270 */
    arc(x1 + radius, y1 + height - radius, quadrant3, quadrant3 + quarterTurn, radius, color);
    /* lower right. draw from 0 to 270 */
    arc(x1+width-radius, y1+height-radius, quadrant4, quadrant4 + quarterTurn, radius, color);
}

void TranslucentBitmap::roundRectFill(int radius, int x1, int y1, int x2, int y2, Graphics::Color color) const {
    Bitmap::roundRectFill(radius, x1, y1, x2, y2, color);
}

void Bitmap::roundRectFill(int radius, int x1, int y1, int x2, int y2, Graphics::Color color) const {
    const int width = x2 - x1;
    const int height = y2 - y1;
    radius = Mid(0, radius, Min((x1+width - x1)/2, (y1+height - y1)/2));

    double quarterTurn = Util::pi / 2;
    double quadrant1 = 0;
    /* signs are flipped because the coordinate system is reflected over the y-axis */
    double quadrant2 = -Util::pi / 2;
    double quadrant3 = Util::pi;
    double quadrant4 = -3 * Util::pi / 2; 

    /* upper right. draw from 90 to 0 */
    arcFilled(x2 - radius, y1 + radius, quadrant1, quadrant1 + quarterTurn, radius, color);
    /* upper left. draw from 180 to 90 */
    arcFilled(x1 + radius, y1 + radius, quadrant2, quadrant2 + quarterTurn, radius, color);
    /* lower left. draw from 270 to 180 */
    arcFilled(x1 + radius, y2 - radius, quadrant3, quadrant3 + quarterTurn, radius, color);
    /* lower right. draw from 360 to 270 */
    arcFilled(x2 - radius, y2 - radius, quadrant4, quadrant4 + quarterTurn, radius, color);

    rectangleFill(x1+radius + 1, y1, x2-radius - 1, y1+radius - 1, color);
    rectangleFill(x1, y1+radius, x2, y2-radius, color);
    rectangleFill(x1+radius + 1, y2-radius + 1, x2-radius - 1, y2, color);
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

StretchedBitmap::StretchedBitmap(int width, int height, const Bitmap & parent, Clear clear, QualityFilter filter):
Bitmap(1, 1),
width(1),
height(1),
where(parent),
filter(filter),
clearKind(clear),
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

    switch (clear){
        case NoClear: break;
        case FullClear: this->clear(); break;
        case Mask: this->clearToMask(); break;
    }
}

void StretchedBitmap::start(){
}

void StretchedBitmap::finish(){
    if (getData() != where.getData()){
        /* FIXME: make scalers understand the masking color. I kinf of doubt this is possible.. */
        if (clearKind == Mask){
            drawStretched(where);
        } else {
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

Shader::Shader(){
}

Shader::~Shader(){
}

SubTranslucentBitmap::SubTranslucentBitmap(const Bitmap & where, int x, int y, int width, int height, Clear clear):
Bitmap(where, x, y, width, height),
translucent(Bitmap(width, height)),
clearKind(clear){
    switch (clear){
        case NoClear: break;
        case FullClear: translucent.clear(); break;
        case Mask: translucent.clearToMask(); break;
    }
}
 
void SubTranslucentBitmap::roundRect(int radius, int x1, int y1, int x2, int y2, Color color) const {
    translucent.roundRect(radius, x1, y1, x2, y2, color);
}

void SubTranslucentBitmap::roundRectFill(int radius, int x1, int y1, int x2, int y2, Color color) const {
    translucent.roundRectFill(radius, x1, y1, x2, y2, color);
}

void SubTranslucentBitmap::finish(){
    translucent.translucent().draw(0, 0, *this);
}

void SubTranslucentBitmap::start(){
}

}
