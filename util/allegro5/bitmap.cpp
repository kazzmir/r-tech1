#include <sstream>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_memfile.h>
#include <allegro5/allegro_primitives.h>
#include "util/debug.h"
#include "util/thread.h"

namespace Graphics{

ALLEGRO_DISPLAY * the_display = NULL;

enum BlendingType{
    Translucent,
    Add,
    Difference,
    Multiply
};

struct BlendingData{
    BlendingData():
        red(0), green(0), blue(0), alpha(0), type(Translucent){
        }

    int red, green, blue, alpha;
    BlendingType type;
};

static BlendingData globalBlend;

/* must be a pointer so it can be created dynamically after allegro init */
// Util::Thread::LockObject * allegroLock;

Color makeColorAlpha(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha){
    return al_map_rgba(red, green, blue, alpha);
}

Color MaskColor(){
    static Color mask = makeColorAlpha(0, 0, 0, 0);
    return mask;
}

Color getBlendColor(){
    /* sort of a hack */
    if (globalBlend.type == Multiply){
        return makeColorAlpha(255, 255, 255, 255);
    }
    return makeColorAlpha(255, 255, 255, globalBlend.alpha);
}

Color doTransBlend(const Color & color, int alpha){
    unsigned char red, green, blue;
    al_unmap_rgb(color, &red, &green, &blue);
    return makeColorAlpha(red, green, blue, alpha);
    /*
    red *= alpha_f;
    green *= alpha_f;
    blue *= alpha_f;
    return al_map_rgb_f(red, green, blue);
    */
}

Color transBlendColor(const Color & color){
    return doTransBlend(color, globalBlend.alpha);
}

int getRealWidth(const Bitmap & what){
    return al_get_bitmap_width(what.getData()->getBitmap());
}

int getRealHeight(const Bitmap & what){
    return al_get_bitmap_height(what.getData()->getBitmap());
}

class Blender{
public:
    Blender(){
    }

    virtual ~Blender(){
        /* default is to draw the source and ignore the destination */
        al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
    }
};

class MaskedBlender: public Blender {
public:
    MaskedBlender(){
        al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
    }
};

class LitBlender: public Blender {
public:
    LitBlender(ALLEGRO_COLOR lit){
        // al_set_blend_color(lit);
        // al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE_MINUS_DST_COLOR, ALLEGRO_ZERO);
    }
};

class TransBlender: public Blender {
public:
    TransBlender(){
        switch (globalBlend.type){
            case Translucent: al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA); break;
            case Add: al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE); break;
            case Multiply: al_set_blender(ALLEGRO_ADD, ALLEGRO_DST_COLOR, ALLEGRO_INVERSE_ALPHA); break;
            case Difference: al_set_blender(ALLEGRO_DEST_MINUS_SRC, ALLEGRO_ONE, ALLEGRO_ONE); break;
        }
    }
};

static const int WINDOWED = 0;
static const int FULLSCREEN = 1;

// static Bitmap * Scaler = NULL;

Bitmap::Bitmap():
mustResize(false),
bit8MaskColor(makeColor(0, 0, 0)),
width(0),
height(0){
    /* TODO */
}

Bitmap::Bitmap( const char * load_file ):
mustResize(false),
bit8MaskColor(makeColor(0, 0, 0)){
    internalLoadFile(load_file);
    width = al_get_bitmap_width(getData()->getBitmap());
    height = al_get_bitmap_height(getData()->getBitmap());
}

Bitmap::Bitmap( const std::string & load_file ):
mustResize(false),
bit8MaskColor(makeColor(0, 0, 0)){
    internalLoadFile(load_file.c_str());
    width = al_get_bitmap_width(getData()->getBitmap());
    height = al_get_bitmap_height(getData()->getBitmap());
}

Bitmap::Bitmap(ALLEGRO_BITMAP * who, bool deep_copy):
mustResize(false),
bit8MaskColor(makeColor(0, 0, 0)){
    if (deep_copy){
        ALLEGRO_BITMAP * clone = al_clone_bitmap(who);
        setData(new BitmapData(clone));
    } else {
        setData(new BitmapData(who));
    }
    this->width = al_get_bitmap_width(getData()->getBitmap());
    this->height = al_get_bitmap_height(getData()->getBitmap());
}

Bitmap::Bitmap(int width, int height):
mustResize(false),
bit8MaskColor(makeColor(0, 0, 0)){
    ALLEGRO_BITMAP * bitmap = al_create_bitmap(width, height);
    if (bitmap == NULL){
        std::ostringstream out;
        out << "Could not create bitmap with dimensions " << width << ", " << height;
        throw BitmapException(__FILE__, __LINE__, out.str());
    }
    setData(new BitmapData(bitmap));
    this->width = al_get_bitmap_width(getData()->getBitmap());
    this->height = al_get_bitmap_height(getData()->getBitmap());
}

Bitmap::Bitmap( const Bitmap & copy, bool deep_copy):
mustResize(false),
bit8MaskColor(copy.bit8MaskColor),
width(copy.width),
height(copy.height){
    if (deep_copy){
        ALLEGRO_BITMAP * clone = al_clone_bitmap(copy.getData()->getBitmap());
        setData(new BitmapData(clone));
    } else {
        setData(copy.getData());
    }
}

void Bitmap::convertToVideo(){
    ALLEGRO_BITMAP * original = getData()->getBitmap();
    ALLEGRO_BITMAP * copy = al_clone_bitmap(original);
    if (copy == NULL){
        throw BitmapException(__FILE__, __LINE__, "Could not create video bitmap");
    }
    al_destroy_bitmap(getData()->getBitmap());
    getData()->setBitmap(copy);
}

void changeTarget(const Bitmap & from, const Bitmap & who){
    /* pray that if drawing is held then who is already the current target */
    if (!al_is_bitmap_drawing_held()){
        al_set_target_bitmap(who.getData()->getBitmap());
        if ((al_get_bitmap_flags(who.getData()->getBitmap()) & ALLEGRO_VIDEO_BITMAP) &&
            (al_get_bitmap_flags(from.getData()->getBitmap()) & ALLEGRO_MEMORY_BITMAP)){
            ((Bitmap&) from).convertToVideo();
            if (&from == &who){
                al_set_target_bitmap(who.getData()->getBitmap());
            }
        }
    }
}

void changeTarget(const Bitmap * from, const Bitmap & who){
    changeTarget(*from, who);
}

void changeTarget(const Bitmap & from, const Bitmap * who){
    changeTarget(from, *who);
}

void changeTarget(const Bitmap * from, const Bitmap * who){
    changeTarget(*from, *who);
}

enum Format{
    PNG,
    GIF,
};

void dumpColor(const Color & color){
    unsigned char red, green, blue, alpha;
    al_unmap_rgba(color, &red, &green, &blue, &alpha);
    Global::debug(0) << "red " << (int) red << " green " << (int) green << " blue " << (int) blue << " alpha " << (int) alpha << std::endl;
}

Color pcxMaskColor(unsigned char * data, const int length){
    if (length >= 769){
        if (data[length - 768 - 1] == 12){
            unsigned char * palette = &data[length - 768];
            unsigned char red = palette[0];
            unsigned char green = palette[1];
            unsigned char blue = palette[2];
            return makeColorAlpha(red, green, blue, 255);
        }
    }
    return makeColorAlpha(255, 255, 255, 255);
}

Bitmap Bitmap::memoryPCX(unsigned char * const data, const int length, const bool mask){
    ALLEGRO_FILE * memory = al_open_memfile((void *) data, length, "r");
    ALLEGRO_BITMAP * pcx = al_load_bitmap_f(memory, ".pcx");
    al_fclose(memory);
    if (pcx == NULL){
        throw BitmapException(__FILE__, __LINE__, "Could not load pcx");
    }
    // dumpColor(al_get_pixel(pcx, 0, 0));
    Bitmap out(pcx);
    out.set8BitMaskColor(pcxMaskColor(data, length));
    return out;
}

static bool isVideoBitmap(ALLEGRO_BITMAP * bitmap){
    return (al_get_bitmap_flags(bitmap) & ALLEGRO_VIDEO_BITMAP) &&
           !(al_get_bitmap_flags(bitmap) & ALLEGRO_MEMORY_BITMAP);
}

void Bitmap::replaceColor(const Color & original, const Color & replaced){
    changeTarget(this, this);

    if (isVideoBitmap(getData()->getBitmap())){
        al_lock_bitmap(getData()->getBitmap(), ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READWRITE);
    }

    int width = getRealWidth(*this);
    int height = getRealHeight(*this);
    for (int x = 0; x < width; x++){
        for (int y = 0; y < height; y++){
            Color pixel = getPixel(x, y);
            if (pixel == original){
                al_put_pixel(x, y, replaced);
            }
        }
    }

    if (isVideoBitmap(getData()->getBitmap())){
        al_unlock_bitmap(getData()->getBitmap());
    }
}

static ALLEGRO_BITMAP * memoryGIF(const char * data, int length){
    ALLEGRO_FILE * memory = al_open_memfile((void *) data, length, "r");
    al_fclose(memory);

/* FIXME: get gif addon for a5 */

#if 0
    RGB * palette = NULL;
    /* algif will close the packfile for us in both error and success cases */
    BITMAP * gif = load_gif_packfile(pack, palette);
    if (!gif){
        al_fclose(memory);
        // pack_fclose(pack);
        ostringstream out;
        out <<"Could not load gif from memory: " << (void*) data << " length " << length;
        throw LoadException(__FILE__, __LINE__, out.str());
    }

    BITMAP * out = create_bitmap(gif->w, gif->h);
    blit(gif, out, 0, 0, 0, 0, gif->w, gif->h);
    destroy_bitmap(gif);
    // pack_fclose(pack);
#endif

    ALLEGRO_BITMAP * out = NULL;

    return out;
}

void Bitmap::internalLoadFile(const char * path){
    this->path = path;
    ALLEGRO_BITMAP * loaded = al_load_bitmap(path);
    if (loaded == NULL){
        std::ostringstream out;
        out << "Could not load file '" << path << "'";
        throw BitmapException(__FILE__, __LINE__, out.str());
    }
    al_convert_mask_to_alpha(loaded, al_map_rgb(255, 0, 255));
    setData(new BitmapData(loaded));
}

static ALLEGRO_BITMAP * load_bitmap_from_memory(const char * data, int length, Format type){
    switch (type){
        case PNG : {
            break;
        }
        case GIF : {
            return memoryGIF(data, length);
            break;
        }
    }
    throw Exception::Base(__FILE__, __LINE__);
}

Bitmap::Bitmap(const char * data, int length):
mustResize(false),
bit8MaskColor(makeColor(0, 0, 0)){
    Format type = GIF;
    setData(new BitmapData(load_bitmap_from_memory(data, length, type)));
    if (getData()->getBitmap() == NULL){
        std::ostringstream out;
        out << "Could not create bitmap from memory";
        throw BitmapException(__FILE__, __LINE__, out.str());
    }
    width = al_get_bitmap_width(getData()->getBitmap());
    height = al_get_bitmap_height(getData()->getBitmap());
}

Bitmap::Bitmap( const Bitmap & copy, int x, int y, int width, int height ):
mustResize(false),
bit8MaskColor(copy.bit8MaskColor),
width(width),
height(height){
    path = copy.getPath();
    ALLEGRO_BITMAP * his = copy.getData()->getBitmap();
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    /*
    if (width + x > al_get_bitmap_width(his)){
        width = al_get_bitmap_width(his) - x;
    }
    if (height + y > al_get_bitmap_height(his)){
        height = al_get_bitmap_height(his) - y;
    }
    */
    
    ALLEGRO_BITMAP * old_target = al_get_target_bitmap();
    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    al_set_target_bitmap(copy.getData()->getBitmap());
    if (al_get_current_transform() != NULL){
        al_set_target_bitmap(copy.getData()->getBitmap());
        al_copy_transform(&transform, al_get_current_transform());
    }

    float x_scaled = x;
    float y_scaled = y;
    float width_scaled = width;
    float height_scaled = height;

    al_transform_coordinates(&transform, &x_scaled, &y_scaled);
    al_transform_coordinates(&transform, &width_scaled, &height_scaled);

    // ALLEGRO_BITMAP * sub = al_create_sub_bitmap(his, x, y, width, height);
    ALLEGRO_BITMAP * sub = al_create_sub_bitmap(his, (int) x_scaled, (int) y_scaled, (int) width_scaled, (int) height_scaled);
    // ALLEGRO_BITMAP * sub = al_create_sub_bitmap(his, (int) x_scaled, (int) y_scaled, width, height);
    setData(new BitmapData(sub));

    al_set_target_bitmap(sub);
    al_use_transform(&transform);
    al_set_target_bitmap(old_target);
}

int Bitmap::getWidth() const {
    return width;
    /*
    if (getData()->getBitmap() != NULL){
        return al_get_bitmap_width(getData()->getBitmap());
    }

    return 0;
    */
}

int getRed(Color color){
    unsigned char red, green, blue;
    al_unmap_rgb(color, &red, &green, &blue);
    return red;
}

int getGreen(Color color){
    unsigned char red, green, blue;
    al_unmap_rgb(color, &red, &green, &blue);
    return green;
}

int getBlue(Color color){
    unsigned char red, green, blue;
    al_unmap_rgb(color, &red, &green, &blue);
    return blue;
}

Color makeColor(int red, int blue, int green){
    return al_map_rgb(red, blue, green);
}

int Bitmap::getHeight() const {
    return height;
    /*
    if (getData()->getBitmap() != NULL){
        return al_get_bitmap_height(getData()->getBitmap());
    }

    return 0;
    */
}

void initializeExtraStuff(){
    // al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_RGB_565);
    // allegroLock = new Util::Thread::LockObject();
}

int setGraphicsMode(int mode, int width, int height){
    initializeExtraStuff();

    /* FIXME: the configuration should pass in fullscreen mode here */
#ifdef IPHONE
    mode = FULLSCREEN;
#endif
    switch (mode){
        case FULLSCREEN: {
#ifdef IPHONE
            al_set_new_display_option(ALLEGRO_SUPPORTED_ORIENTATIONS, ALLEGRO_DISPLAY_ORIENTATION_LANDSCAPE, ALLEGRO_SUGGEST);
            al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
#else
            al_set_new_display_flags(ALLEGRO_FULLSCREEN);
#endif
            break;
        }
        case WINDOWED: {
            al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE);
            break;
        }
        default: break;
    }
    the_display = al_create_display(width, height);
    if (the_display == NULL){
        std::ostringstream out;
        out << "Could not create display with dimensions " << width << ", " << height;
        throw BitmapException(__FILE__, __LINE__, out.str());
    }

    // Global::debug(0) << "Set width " << al_get_display_width(the_display) << " height " << al_get_display_height(the_display) << std::endl;
    // Global::debug(0) << "Backbuffer width " << al_get_bitmap_width(al_get_backbuffer(the_display)) << " height " << al_get_bitmap_height(al_get_backbuffer(the_display)) << std::endl;

    try{
        /* TODO: maybe find a more general way to get the icon */
        ALLEGRO_BITMAP * icon = al_load_bitmap(Storage::instance().find(Filesystem::RelativePath("menu/icon.bmp")).path().c_str());
        if (icon != NULL){
            al_set_display_icon(the_display, icon);
        }
    } catch (const Filesystem::NotFound & fail){
        Global::debug(0) << "Could not set window icon: " << fail.getTrace() << std::endl;
    }
    Screen = new Bitmap(al_get_backbuffer(the_display));
    /* dont destroy the backbuffer */
    Screen->getData()->setDestroy(false);

    ALLEGRO_TRANSFORM transformation;
    al_identity_transform(&transformation);
    al_scale_transform(&transformation, (double) Screen->getWidth() / (double) width, (double) Screen->getHeight() / (double) height);
    al_set_target_bitmap(Screen->getData()->getBitmap());
    al_use_transform(&transformation);

    // Scaler = new Bitmap(width, height);
    /* default drawing mode */
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
    return 0;
}

void Bitmap::lock() const {
    al_lock_bitmap(getData()->getBitmap(), ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READWRITE);
}
        
void Bitmap::lock(int x, int y, int width, int height) const {
    al_lock_bitmap_region(getData()->getBitmap(), x, y, width, height, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READWRITE);
}

void Bitmap::unlock() const {
    al_unlock_bitmap(getData()->getBitmap());
}

Color Bitmap::getPixel(const int x, const int y) const {
    // changeTarget(this, this);
    return al_get_pixel(getData()->getBitmap(), x, y);
}

void Bitmap::putPixel(int x, int y, Color pixel) const {
    changeTarget(this, this);
    al_put_pixel(x, y, pixel);
}

void Bitmap::putPixelNormal(int x, int y, Color col) const {
    putPixel(x, y, col);
}

void Bitmap::fill(Color color) const {
    changeTarget(this, this);
    al_clear_to_color(color);
}

void Bitmap::startDrawing() const {
    /* we are about to draw on this bitmap so make sure we are the target */
    changeTarget(this, this);
    al_hold_bitmap_drawing(true);
}

void Bitmap::endDrawing() const {
    al_hold_bitmap_drawing(false);
}

void TranslucentBitmap::startDrawing() const {
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
}

void TranslucentBitmap::endDrawing() const {
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
}

Color Bitmap::blendColor(const Color & input) const {
    return input;
}

Color TranslucentBitmap::blendColor(const Color & color) const {
    unsigned char red, green, blue;
    unsigned char alpha = globalBlend.alpha;
    al_unmap_rgb(color, &red, &green, &blue);
    return makeColorAlpha(red, green, blue, alpha);
}

void Bitmap::Stretch( const Bitmap & where, const int sourceX, const int sourceY, const int sourceWidth, const int sourceHeight, const int destX, const int destY, const int destWidth, const int destHeight ) const {
    /* TODO */
}

void Bitmap::StretchBy2( const Bitmap & where ){
    /* TODO */
}

void Bitmap::StretchBy4( const Bitmap & where ){
    /* TODO */
}

void Bitmap::drawRotate(const int x, const int y, const int angle, const Bitmap & where ){
    changeTarget(this, where);
    MaskedBlender blender;
    al_draw_rotated_bitmap(getData()->getBitmap(), getWidth() / 2, getHeight() / 2, x, y, Util::radians(angle), ALLEGRO_FLIP_HORIZONTAL);
}

void Bitmap::drawPivot( const int centerX, const int centerY, const int x, const int y, const int angle, const Bitmap & where ){
    /* TODO */
}

void Bitmap::drawPivot( const int centerX, const int centerY, const int x, const int y, const int angle, const double scale, const Bitmap & where ){
    /* TODO */
}

void Bitmap::drawStretched( const int x, const int y, const int new_width, const int new_height, const Bitmap & who ) const {
    /* FIXME */
    changeTarget(this, who);
    MaskedBlender blender;
    al_draw_scaled_bitmap(getData()->getBitmap(), 0, 0, al_get_bitmap_width(getData()->getBitmap()), al_get_bitmap_height(getData()->getBitmap()), x, y, new_width, new_height, 0);
#if 0
    ALLEGRO_TRANSFORM save;
    al_copy_transform(&save, al_get_current_transform());
    ALLEGRO_TRANSFORM stretch;
    al_identity_transform(&stretch);
    // al_translate_transform(&stretch, x / ((double) new_width / getWidth()), y / ((double) new_height / getHeight()));
    al_scale_transform(&stretch, (double) new_width / getWidth(), (double) new_height / getHeight());
    al_translate_transform(&stretch, x, y);
    // al_translate_transform(&stretch, -x / ((double) new_width / getWidth()), -y / ((double) (new_height / getHeight())));
    al_use_transform(&stretch);
    /* any source pixels with an alpha value of 0 will be masked */
    // al_draw_bitmap(getData().getBitmap(), x, y, 0);
    al_draw_bitmap(getData().getBitmap(), 0, 0, 0);
    al_use_transform(&save);
#endif
}

Bitmap Bitmap::scaleTo(const int width, const int height) const {
    if (width == getRealWidth(*this) && height == getRealHeight(*this)){
        return *this;
    }
    Bitmap scaled(width, height);
    changeTarget(*this, scaled);
    al_draw_scaled_bitmap(getData()->getBitmap(), 0, 0, getRealWidth(*this), getRealHeight(*this),
                          0, 0, width, height, 0);
    return scaled;
}

void Bitmap::Blit(const int mx, const int my, const int width, const int height, const int wx, const int wy, const Bitmap & where) const {
    // double start = al_get_time();
    // changeTarget(this, where);
    // al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
    /*
    if (&where != Screen){
        al_draw_bitmap(getData().getBitmap(), wx, wy, 0);
    }
    */

    changeTarget(this, where);
    Bitmap part(*this, mx, my, width, height);
    // al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
    al_draw_bitmap(part.getData()->getBitmap(), wx, wy, 0);
    /*
    double end = al_get_time();
    Global::debug(0) << "Draw in " << (end - start) << " seconds" << std::endl;
    */
}

void Bitmap::drawHFlip(const int x, const int y, const Bitmap & where) const {
    changeTarget(this, where);
    MaskedBlender blender;
    /* any source pixels with an alpha value of 0 will be masked */
    al_draw_bitmap(getData()->getBitmap(), x, y, ALLEGRO_FLIP_HORIZONTAL);
}

void Bitmap::drawHFlip(const int x, const int y, Filter * filter, const Bitmap & where) const {
    /* FIXME: deal with filter */
    changeTarget(this, where);
    MaskedBlender blender;
    /* any source pixels with an alpha value of 0 will be masked */
    al_draw_bitmap(getData()->getBitmap(), x, y, ALLEGRO_FLIP_HORIZONTAL);
}

void Bitmap::drawVFlip( const int x, const int y, const Bitmap & where ) const {
    changeTarget(this, where);
    MaskedBlender blender;
    /* any source pixels with an alpha value of 0 will be masked */
    al_draw_bitmap(getData()->getBitmap(), x, y, ALLEGRO_FLIP_VERTICAL);
}

void Bitmap::drawVFlip( const int x, const int y, Filter * filter, const Bitmap & where ) const {
    /* FIXME: deal with filter */
    changeTarget(this, where);
    MaskedBlender blender;
    /* any source pixels with an alpha value of 0 will be masked */
    al_draw_bitmap(getData()->getBitmap(), x, y, ALLEGRO_FLIP_VERTICAL);
}

void Bitmap::drawHVFlip( const int x, const int y, const Bitmap & where ) const {
    changeTarget(this, where);
    MaskedBlender blender;
    /* any source pixels with an alpha value of 0 will be masked */
    al_draw_bitmap(getData()->getBitmap(), x, y, ALLEGRO_FLIP_VERTICAL | ALLEGRO_FLIP_HORIZONTAL);
}

void Bitmap::drawHVFlip( const int x, const int y, Filter * filter, const Bitmap & where ) const {
    /* FIXME: deal with filter */
    changeTarget(this, where);
    MaskedBlender blender;
    /* any source pixels with an alpha value of 0 will be masked */
    al_draw_bitmap(getData()->getBitmap(), x, y, ALLEGRO_FLIP_VERTICAL | ALLEGRO_FLIP_HORIZONTAL);
}

void Bitmap::BlitMasked(const int mx, const int my, const int width, const int height, const int wx, const int wy, const Bitmap & where) const {
    /* TODO */
}

void Bitmap::BlitToScreen(const int upper_left_x, const int upper_left_y) const {
#if 0
    if (getWidth() != Screen->getWidth() || getHeight() != Screen->getHeight()){
        /*
        this->Blit( upper_left_x, upper_left_y, *Buffer );
        Buffer->Stretch(*Scaler);
        Scaler->Blit(0, 0, 0, 0, *Screen);
        */

        this->Stretch(*Scaler, 0, 0, getWidth(), getHeight(), upper_left_x, upper_left_y, Scaler->getWidth(), Scaler->getHeight());
        Scaler->Blit(0, 0, 0, 0, *Screen);
    } else {
        this->Blit(upper_left_x, upper_left_y, *Screen);
    }
#endif
    /*
    if (&where == Screen){
        al_flip_display();
    }
    */
    changeTarget(this, Screen);
    if (getData()->getBitmap() != Screen->getData()->getBitmap()){
        Blit(*Screen);
    }
    al_flip_display();
}

void Bitmap::BlitAreaToScreen(const int upper_left_x, const int upper_left_y) const {
    changeTarget(this, Screen);
    /*
    if (getData()->getBitmap() != Screen->getData()->getBitmap()){
        Blit(upper_left_y, upper_left_y, *Screen);
    }
    */
    al_flip_display();
}

void Bitmap::draw(const int x, const int y, const Bitmap & where) const {
    // TransBlender blender;
    changeTarget(this, where);
    MaskedBlender blender;
    /* any source pixels with an alpha value of 0 will be masked */
    al_draw_bitmap(getData()->getBitmap(), x, y, 0);
}

void Bitmap::draw(const int x, const int y, Filter * filter, const Bitmap & where) const {
    /* FIXME */
    changeTarget(this, where);
    MaskedBlender blender;
    /* any source pixels with an alpha value of 0 will be masked */
    al_draw_bitmap(getData()->getBitmap(), x, y, 0);

}

void Bitmap::hLine(const int x1, const int y, const int x2, const Color color) const {
    line(x1, y, x2, y, color);
}

void Bitmap::vLine(const int y1, const int x, const int y2, const Color color) const {
    line(x, y1, x, y2, color);
}

void Bitmap::arc(const int x, const int y, const double ang1, const double ang2, const int radius, const Color color ) const {
    changeTarget(this, this);
    al_draw_arc(x, y, radius, ang1 - Util::pi/2, ang2 - ang1, color, 1);
}

void TranslucentBitmap::arc(const int x, const int y, const double ang1, const double ang2, const int radius, const Color color ) const {
    TransBlender blender;
    Bitmap::arc(x, y, ang1, ang2, radius, transBlendColor(color));
    /*
    changeTarget(this);
    al_draw_arc(x, y, radius, ang1 + S_PI/2, ang2 - ang1, doTransBlend(color, globalBlend.alpha), 0);
    */
}

/* from http://www.allegro.cc/forums/thread/605684/892721#target */
#if 0
void al_draw_filled_pieslice(float cx, float cy, float r, float start_theta,
                                float delta_theta, ALLEGRO_COLOR color){
    ALLEGRO_VERTEX vertex_cache[ALLEGRO_VERTEX_CACHE_SIZE];
    int num_segments, ii;

    num_segments = fabs(delta_theta / (2 * ALLEGRO_PI) * ALLEGRO_PRIM_QUALITY * sqrtf(r));

    if (num_segments < 2)
        return;

    if (num_segments >= ALLEGRO_VERTEX_CACHE_SIZE) {
        num_segments = ALLEGRO_VERTEX_CACHE_SIZE - 1;
    }

    al_calculate_arc(&(vertex_cache[1].x), sizeof(ALLEGRO_VERTEX), cx, cy, r, r, start_theta, delta_theta, 0, num_segments);
    vertex_cache[0].x = cx; vertex_cache[0].y = cy;

    for (ii = 0; ii < num_segments + 1; ii++) {
        vertex_cache[ii].color = color;
        vertex_cache[ii].z = 0;
    }

    al_draw_prim(vertex_cache, NULL, NULL, 0, num_segments + 1, ALLEGRO_PRIM_TRIANGLE_FAN);
    // al_draw_prim(vertex_cache, NULL, NULL, 0, 3, ALLEGRO_PRIM_TRIANGLE_FAN);
}
#endif

void Bitmap::arcFilled(const int x, const int y, const double ang1, const double ang2, const int radius, const Color color ) const {
    changeTarget(this, this);
    al_draw_filled_pieslice(x, y, radius, ang1 - Util::pi/2, ang2 - ang1, color);
}

void TranslucentBitmap::arcFilled(const int x, const int y, const double ang1, const double ang2, const int radius, const Color color ) const {
    TransBlender blender;
    Bitmap::arcFilled(x, y, ang1, ang2, radius, transBlendColor(color));
}

void Bitmap::floodfill( const int x, const int y, const Color color ) const {
    /* TODO */
}

void Bitmap::line(const int x1, const int y1, const int x2, const int y2, const Color color) const {
    al_draw_line(x1, y1, x2, y2, color, 1.5);
}

void TranslucentBitmap::line(const int x1, const int y1, const int x2, const int y2, const Color color) const {
    TransBlender blender;
    Bitmap::line(x1, y1, x2, y2, transBlendColor(color));
}

void Bitmap::circleFill(int x, int y, int radius, Color color) const {
    changeTarget(this, this);
    al_draw_filled_circle(x, y, radius, color);
}

void Bitmap::circle(int x, int y, int radius, Color color) const {
    changeTarget(this, this);
    al_draw_circle(x, y, radius, color, 0);
}

void Bitmap::rectangle( int x1, int y1, int x2, int y2, Color color ) const {
    changeTarget(this, this);
    al_draw_rectangle(x1, y1, x2, y2, color, 0);
}

void Bitmap::rectangleFill( int x1, int y1, int x2, int y2, Color color ) const {
    changeTarget(this, this);
    al_draw_filled_rectangle(x1 - 0.5, y1 - 0.5, x2 + 0.5, y2 + 0.5, color);
}

void Bitmap::triangle( int x1, int y1, int x2, int y2, int x3, int y3, Color color ) const {
    changeTarget(this, this);
    al_draw_filled_triangle(x1, y1, x2, y2, x3, y3, color);
}

void Bitmap::polygon( const int * verts, const int nverts, const Color color ) const {
    /* TODO */
}

void Bitmap::ellipse( int x, int y, int rx, int ry, Color color ) const {
    changeTarget(this, this);
    al_draw_ellipse(x, y, rx, ry, color, 0);
}

void Bitmap::ellipseFill( int x, int y, int rx, int ry, Color color ) const {
    changeTarget(this, this);
    al_draw_filled_ellipse(x, y, rx, ry, color);
}

void Bitmap::applyTrans(const Color color) const {
    TransBlender blender;
    changeTarget(this, this);
    al_draw_filled_rectangle(0, 0, getWidth(), getHeight(), transBlendColor(color));
}

void Bitmap::light(int x, int y, int width, int height, int start_y, int focus_alpha, int edge_alpha, int focus_color, Color edge_color) const {
    /* TODO */
}

void Bitmap::drawCharacter( const int x, const int y, const int color, const int background, const Bitmap & where ) const {
    /* TODO */
}

void Bitmap::save( const std::string & str ) const {
    /* TODO */
}

void Bitmap::readLine(std::vector<Color> & line, int y){
    /* TODO */
}

void TranslucentBitmap::draw(const int x, const int y, const Bitmap & where) const {
    changeTarget(this, where);
    TransBlender blender;
    al_draw_tinted_bitmap(getData()->getBitmap(), getBlendColor(), x, y, 0);
}

void LitBitmap::draw(const int x, const int y, const Bitmap & where) const {
    // changeTarget(this, where);
    // LitBlender blender(makeColorAlpha(globalBlend.red, globalBlend.green, globalBlend.blue, globalBlend.alpha));
    // TransBlender blender;
    // al_draw_bitmap(getData()->getBitmap(), x, y, 0);
    // al_draw_tinted_bitmap(getData()->getBitmap(), al_map_rgba_f(1, 0, 0, 1), x, y, 0);
}

void LitBitmap::draw( const int x, const int y, Filter * filter, const Bitmap & where ) const {
    /* TODO */
}

void LitBitmap::drawHFlip( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void LitBitmap::drawHFlip( const int x, const int y, Filter * filter, const Bitmap & where ) const {
    /* TODO */
}

void LitBitmap::drawVFlip( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void LitBitmap::drawVFlip( const int x, const int y, Filter * filter, const Bitmap & where ) const {
    /* TODO */
}

void LitBitmap::drawHVFlip( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void LitBitmap::drawHVFlip( const int x, const int y, Filter * filter, const Bitmap & where ) const {
    /* TODO */
}

void TranslucentBitmap::draw( const int x, const int y, Filter * filter, const Bitmap & where ) const {
    changeTarget(this, where);
    TransBlender blender;
    al_draw_tinted_bitmap(getData()->getBitmap(), getBlendColor(), x, y, 0);
}

void TranslucentBitmap::drawHFlip( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void TranslucentBitmap::drawHFlip( const int x, const int y, Filter * filter, const Bitmap & where ) const {
    /* TODO */
}

void TranslucentBitmap::drawVFlip( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void TranslucentBitmap::drawVFlip( const int x, const int y, Filter * filter, const Bitmap & where ) const {
    /* TODO */
}

void TranslucentBitmap::drawHVFlip( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void TranslucentBitmap::drawHVFlip( const int x, const int y, Filter * filter,const Bitmap & where ) const {
    /* TODO */
}

void TranslucentBitmap::hLine( const int x1, const int y, const int x2, const Color color ) const {
    /* TODO */
}

void TranslucentBitmap::circleFill(int x, int y, int radius, Color color) const {
    TransBlender blender;
    Bitmap::circleFill(x, y, radius, doTransBlend(color, globalBlend.alpha));
}

void TranslucentBitmap::putPixelNormal(int x, int y, Color color) const {
    TransBlender blender;
    Bitmap::putPixelNormal(x, y, doTransBlend(color, globalBlend.alpha));
}

void TranslucentBitmap::rectangle( int x1, int y1, int x2, int y2, Color color ) const {
    TransBlender blender;
    Bitmap::rectangle(x1, y1, x2, y2, doTransBlend(color, globalBlend.alpha));
}

void TranslucentBitmap::rectangleFill(int x1, int y1, int x2, int y2, Color color) const {
    TransBlender blender;
    Bitmap::rectangleFill(x1, y1, x2, y2, doTransBlend(color, globalBlend.alpha));
}

void TranslucentBitmap::ellipse( int x, int y, int rx, int ry, Color color ) const {
    TransBlender blender;
    Bitmap::ellipse(x, y, rx, ry, doTransBlend(color, globalBlend.alpha));
}

void Bitmap::setClipRect( int x1, int y1, int x2, int y2 ) const {
    /* TODO */
}

void Bitmap::getClipRect(int & x1, int & y1, int & x2, int & y2) const {
    /* TODO */
}

int setGfxModeFullscreen(int x, int y){
    return setGraphicsMode(FULLSCREEN, x, y);
}

int setGfxModeWindowed( int x, int y ){
    return setGraphicsMode(WINDOWED, x, y);
}

int setGfxModeText(){
    /* TODO */
    return 0;
}

bool Bitmap::getError(){
    /* TODO */
    return false;
}

void Bitmap::transBlender(int r, int g, int b, int a){
    globalBlend.red = r;
    globalBlend.green = g;
    globalBlend.blue = b;
    globalBlend.alpha = a;
    globalBlend.type = Translucent;
}

void Bitmap::addBlender(int r, int g, int b, int a){
    globalBlend.red = r;
    globalBlend.green = g;
    globalBlend.blue = b;
    globalBlend.alpha = a;
    globalBlend.type = Add;
}

void Bitmap::differenceBlender( int r, int g, int b, int a ){
    globalBlend.red = r;
    globalBlend.green = g;
    globalBlend.blue = b;
    globalBlend.alpha = a;
    globalBlend.type = Difference;
}

void Bitmap::multiplyBlender( int r, int g, int b, int a ){
    globalBlend.red = r;
    globalBlend.green = g;
    globalBlend.blue = b;
    globalBlend.alpha = a;
    globalBlend.type = Multiply;
}

void Bitmap::drawingMode(int type){
    /* TODO */
}

void Bitmap::shutdown(){
    delete Screen;
    Screen = NULL;
    // delete allegroLock;
    // allegroLock = NULL;
    /*
    delete Scaler;
    Scaler = NULL;
    delete Buffer;
    Buffer = NULL;
    */
}

StretchedBitmap::StretchedBitmap(int width, int height, const Bitmap & parent):
Bitmap(parent, 0, 0, parent.getWidth(), parent.getHeight()),
width(width),
height(height),
where(parent){
    scale_x = (double) parent.getWidth() / width;
    scale_y = (double) parent.getHeight() / height;
    ALLEGRO_BITMAP * old_target = al_get_target_bitmap();
    al_set_target_bitmap(parent.getData()->getBitmap());
    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    if (al_get_current_transform() != NULL){
        al_copy_transform(&transform, al_get_current_transform());
    }
    al_scale_transform(&transform, scale_x, scale_y);
    al_set_target_bitmap(getData()->getBitmap());
    al_use_transform(&transform);
    al_set_target_bitmap(old_target);
}

void StretchedBitmap::start(){
#if 0
    ALLEGRO_TRANSFORM transform;
    changeTarget(this, this);
    al_copy_transform(&transform, al_get_current_transform());
    // al_identity_transform(&transform);
    // al_scale_transform(&transform, Bitmap::getWidth() / width, Bitmap::getHeight() / height);
    al_scale_transform(&transform, scale_x, scale_y);
    al_use_transform(&transform);
#endif
}

void StretchedBitmap::finish(){
#if 0
    ALLEGRO_TRANSFORM transform;
    changeTarget(this, this);
    al_copy_transform(&transform, al_get_current_transform());
    /* apply the inverse transform */
    al_scale_transform(&transform, 1.0/scale_x, 1.0/scale_y);
    // al_identity_transform(&transform);
    al_use_transform(&transform);
#endif
}

TranslatedBitmap::TranslatedBitmap(int x, int y, const Bitmap & where):
Bitmap(where),
x(x),
y(y){
    ALLEGRO_TRANSFORM transform;
    changeTarget(this, where);
    al_identity_transform(&transform);
    if (al_get_current_transform() != NULL){
        al_copy_transform(&transform, al_get_current_transform());
    }
    al_translate_transform(&transform, x, y);
    al_use_transform(&transform);
}

void TranslatedBitmap::BlitToScreen() const {
    Bitmap::BlitToScreen();
}

TranslatedBitmap::~TranslatedBitmap(){
    ALLEGRO_TRANSFORM transform;
    al_copy_transform(&transform, al_get_current_transform());
    al_translate_transform(&transform, -x, -y);
    al_use_transform(&transform);
}

Bitmap getScreenBuffer(){
    return *Screen;
}

RestoreState::RestoreState(){
    al_store_state(&state, ALLEGRO_STATE_ALL);
}

RestoreState::~RestoreState(){
    al_restore_state(&state);
}

}

static inline bool close(float x, float y){
    static float epsilon = 0.001;
    return fabs(x - y) < epsilon;
}

static inline bool sameColor(Graphics::Color color1, Graphics::Color color2){
    // return memcmp(&color1, &color2, sizeof(Graphics::Color)) == 0;
    float r1, g1, b1, a1;
    float r2, g2, b2, a2;
    al_unmap_rgba_f(color1, &r1, &g1, &b1, &a1);
    al_unmap_rgba_f(color2, &r2, &g2, &b2, &a2);
    return close(r1, r2) &&
           close(g1, g2) &&
           close(b1, b2) &&
           close(a1, a2);

    /*
    unsigned char r1, g1, b1, a1;
    unsigned char r2, g2, b2, a2;
    al_unmap_rgba(color1, &r1, &g1, &b1, &a1);
    al_unmap_rgba(color2, &r2, &g2, &b2, &a2);
    return r1 == r2 &&
           g1 == g2 &&
           b1 == b2 &&
           a1 == a2;
    */
}

static uint32_t quantify(const ALLEGRO_COLOR & color){
    unsigned char red, green, blue, alpha;
    al_unmap_rgba(color, &red, &green, &blue, &alpha);
    return (red << 24) |
           (green << 16) |
           (blue << 8) |
           alpha;
}

bool operator<(const ALLEGRO_COLOR & color1, const ALLEGRO_COLOR & color2){
    return quantify(color1) < quantify(color2);
}

bool operator!=(const ALLEGRO_COLOR & color1, const ALLEGRO_COLOR & color2){
    return !(color1 == color2);
}

bool operator==(const ALLEGRO_COLOR & color1, const ALLEGRO_COLOR & color2){
    return sameColor(color1, color2);
}
