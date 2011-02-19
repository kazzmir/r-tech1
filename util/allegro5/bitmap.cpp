#include <sstream>
#include <allegro5/allegro_memfile.h>
#include "util/debug.h"

namespace Graphics{

static const int rgb_r_shift_16 = 0;
static const int rgb_g_shift_16 = 5;
static const int rgb_b_shift_16 = 11;

static inline int pack565(unsigned char red, unsigned char green, unsigned char blue){
    return (((red >> 3) << rgb_r_shift_16) |
            ((green >> 2) << rgb_g_shift_16) |
            ((blue >> 3) << rgb_b_shift_16));
}

/* from allegro4 */
static inline void unpack565(int color, unsigned char * red, unsigned char * green, unsigned char * blue){
    static int _rgb_scale_5[32] = {
        0,   8,   16,  24,  33,  41,  49,  57,
        66,  74,  82,  90,  99,  107, 115, 123,
        132, 140, 148, 156, 165, 173, 181, 189,
        198, 206, 214, 222, 231, 239, 247, 255
    };

    static int _rgb_scale_6[64] = {
        0,   4,   8,   12,  16,  20,  24,  28,
        32,  36,  40,  44,  48,  52,  56,  60,
        65,  69,  73,  77,  81,  85,  89,  93,
        97,  101, 105, 109, 113, 117, 121, 125,
        130, 134, 138, 142, 146, 150, 154, 158,
        162, 166, 170, 174, 178, 182, 186, 190,
        195, 199, 203, 207, 211, 215, 219, 223,
        227, 231, 235, 239, 243, 247, 251, 255
    };

    *red = _rgb_scale_5[(color >> rgb_r_shift_16) & 0x1F];
    *green = _rgb_scale_6[(color >> rgb_g_shift_16) & 0x3F];
    *blue = _rgb_scale_5[(color >> rgb_b_shift_16) & 0x1F];
}

int Bitmap::MaskColor(){
    static int mask = makeColor(255, 0, 255);
    return mask;
}

static const int WINDOWED = 0;
static const int FULLSCREEN = 1;

Bitmap * Bitmap::Screen = NULL;
static Bitmap * Scaler = NULL;

Bitmap::Bitmap():
own(NULL),
mustResize(false),
bit8MaskColor(0){
    /* TODO */
}

Bitmap::Bitmap( const char * load_file ):
own(NULL),
mustResize(false),
bit8MaskColor(0){
    internalLoadFile(load_file);
}

Bitmap::Bitmap( const std::string & load_file ):
own(NULL),
mustResize(false){
    internalLoadFile(load_file.c_str());
}

Bitmap::Bitmap(ALLEGRO_BITMAP * who, bool deep_copy):
own(NULL),
mustResize(false),
bit8MaskColor(0){
    if (deep_copy){
        ALLEGRO_BITMAP * clone = al_clone_bitmap(who);
        getData().setBitmap(clone);
        own = new int;
        *own = 1;
    } else {
        getData().setBitmap(who);
    }
}

Bitmap::Bitmap(int width, int height):
own(NULL),
mustResize(false),
bit8MaskColor(0){
    ALLEGRO_BITMAP * bitmap = al_create_bitmap(width, height);
    if (bitmap == NULL){
        std::ostringstream out;
        out << "Could not create bitmap with dimensions " << width << ", " << height;
        throw BitmapException(__FILE__, __LINE__, out.str());
    }
    getData().setBitmap(bitmap);
    own = new int;
    *own = 1;
}

Bitmap::Bitmap( const Bitmap & copy, bool deep_copy):
own(NULL),
mustResize(false),
bit8MaskColor(copy.bit8MaskColor){
    if (deep_copy){
        ALLEGRO_BITMAP * clone = al_clone_bitmap(copy.getData().getBitmap());
        getData().setBitmap(clone);
        own = new int;
        *own = 1;
    } else {
        getData().setBitmap(copy.getData().getBitmap());
        own = copy.own;
        if (own){
            *own += 1;
        }
    }
}

enum Format{
    PNG,
    GIF,
};

Bitmap Bitmap::memoryPCX(unsigned char * const data, const int length, const bool mask){
    /* TODO */
    return Bitmap();
}

void Bitmap::replaceColor(int original, int replaced){
    /* TODO */
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
    getData().setBitmap(loaded);
    /*
    ALLEGRO_BITMAP * loaded = IMG_Load(path);
    if (loaded){
        getData().setSurface(optimizedSurface(loaded));
        SDL_FreeSurface(loaded);
    } else {
        std::ostringstream out;
        out << "Could not load file '" << path << "'";
        throw BitmapException(__FILE__, __LINE__, out.str());
    }
    */
    own = new int;
    *own = 1;
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
own(NULL),
mustResize(false),
bit8MaskColor(0){
    Format type = GIF;
    getData().setBitmap(load_bitmap_from_memory(data, length, type));
    if (getData().getBitmap() == NULL){
        std::ostringstream out;
        out << "Could not create bitmap from memory";
        throw BitmapException(__FILE__, __LINE__, out.str());
    }
    own = new int;
    *own = 1;
}

Bitmap::Bitmap( const Bitmap & copy, int x, int y, int width, int height ):
own(NULL),
mustResize(false),
bit8MaskColor(copy.bit8MaskColor){
    path = copy.getPath();
    ALLEGRO_BITMAP * his = copy.getData().getBitmap();
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    if (width + x > al_get_bitmap_width(his)){
        width = al_get_bitmap_width(his) - x;
    }
    if (height + y > al_get_bitmap_height(his)){
        height = al_get_bitmap_height(his) - y;
    }

    ALLEGRO_BITMAP * sub = al_create_sub_bitmap(his, x, y, width, height);
    getData().setBitmap(sub);
    
    own = new int;
    *own = 1;
}

int Bitmap::getWidth() const {
    if (getData().getBitmap() != NULL){
        return al_get_bitmap_width(getData().getBitmap());
    }

    return 0;
}

int Bitmap::getRed(int color){
    unsigned char red, green, blue;
    unpack565(color, &red, &green, &blue);
    return red;
}

int Bitmap::getGreen(int color){
    unsigned char red, green, blue;
    unpack565(color, &red, &green, &blue);
    return green;
}

int Bitmap::getBlue(int color){
    unsigned char red, green, blue;
    unpack565(color, &red, &green, &blue);
    return blue;
}

int Bitmap::makeColor(int red, int blue, int green){
    return pack565(red, blue, green);
}

int Bitmap::getHeight() const {
    if (getData().getBitmap() != NULL){
        return al_get_bitmap_height(getData().getBitmap());
    }

    return 0;
}

int Bitmap::setGraphicsMode(int mode, int width, int height){
    initializeExtraStuff();
    switch (mode){
        case FULLSCREEN: {
            al_set_new_display_flags(ALLEGRO_FULLSCREEN);
            break;
        }
        case WINDOWED: {
            al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE);
            break;
        }
        default: break;
    }
    ALLEGRO_DISPLAY * display = al_create_display(width, height);
    if (display == NULL){
        std::ostringstream out;
        out << "Could not create display with dimensions " << width << ", " << height;
        throw BitmapException(__FILE__, __LINE__, out.str());
    }
    Screen = new Bitmap(al_get_backbuffer(display));
    Scaler = new Bitmap(width, height);
    return 0;
}

int Bitmap::getPixel(const int x, const int y) const {
    ALLEGRO_COLOR color = al_get_pixel(getData().getBitmap(), x, y);
    return pack565(color.r, color.g, color.b);
}

void Bitmap::putPixel(int x, int y, int pixel) const {
    /* TODO */
}

void Bitmap::putPixelNormal(int x, int y, int col) const {
    putPixel(x, y, col);
}

void Bitmap::fill(int color) const {
    unsigned char red, green, blue;
    unpack565(color, &red, &green, &blue);
    // al_set_target_bitmap(getData().getBitmap());
    al_clear_to_color(al_map_rgb(red, green, blue));
}

void Bitmap::transBlender( int r, int g, int b, int a ){
    /* TODO */
    /*
    globalBlend.red = r;
    globalBlend.green = g;
    globalBlend.blue = b;
    globalBlend.alpha = a;
    globalBlend.currentBlender = ::transBlender;
    */
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

void Bitmap::drawRotate( const int x, const int y, const int angle, const Bitmap & where ){
    /* TODO */
}

void Bitmap::drawPivot( const int centerX, const int centerY, const int x, const int y, const int angle, const Bitmap & where ){
    /* TODO */
}

void Bitmap::drawPivot( const int centerX, const int centerY, const int x, const int y, const int angle, const double scale, const Bitmap & where ){
    /* TODO */
}

void Bitmap::drawStretched( const int x, const int y, const int new_width, const int new_height, const Bitmap & who ) const {
    /* TODO */
}

void Bitmap::Blit(const int mx, const int my, const int width, const int height, const int wx, const int wy, const Bitmap & where) const {
    // double start = al_get_time();
    // al_set_target_bitmap(where.getData().getBitmap());
    // al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
    /*
    if (&where != Screen){
        al_draw_bitmap(getData().getBitmap(), wx, wy, 0);
    }
    */
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
    al_draw_bitmap(getData().getBitmap(), wx, wy, 0);
    /*
    double end = al_get_time();
    Global::debug(0) << "Draw in " << (end - start) << " seconds" << std::endl;
    */
}

void Bitmap::drawHFlip(const int x, const int y, const Bitmap & where) const {
    /* TODO */
}

void Bitmap::drawHFlip(const int x, const int y, Filter * filter, const Bitmap & where) const {
    /* TODO */
}

void Bitmap::drawVFlip( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void Bitmap::drawVFlip( const int x, const int y, Filter * filter, const Bitmap & where ) const {
    /* TODO */
}

void Bitmap::drawHVFlip( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void Bitmap::drawHVFlip( const int x, const int y, Filter * filter, const Bitmap & where ) const {
    /* TODO */
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
    al_flip_display();
}

void Bitmap::BlitAreaToScreen(const int upper_left_x, const int upper_left_y) const {
    /* TODO */
}

void Bitmap::draw(const int x, const int y, const Bitmap & where) const {
    // al_set_target_bitmap(where.getData().getBitmap());
    // al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
    al_draw_bitmap(getData().getBitmap(), x, y, 0);
}

void Bitmap::draw(const int x, const int y, Filter * filter, const Bitmap & where) const {
    /* TODO */
}

void Bitmap::hLine( const int x1, const int y, const int x2, const int color ) const {
    /* TODO */
}

void Bitmap::vLine( const int x1, const int y, const int x2, const int color ) const {
    /* TODO */
}

void Bitmap::arc(const int x, const int y, const double ang1, const double ang2, const int radius, const int color ) const {
    /* TODO */
}

void Bitmap::floodfill( const int x, const int y, const int color ) const {
    /* TODO */
}

void Bitmap::line( const int x1, const int y1, const int x2, const int y2, const int color ) const {
    /* TODO */
}

void Bitmap::circleFill(int x, int y, int radius, int color) const {
    /* TODO */
}

void Bitmap::circle(int x, int y, int radius, int color) const {
    /* TODO */
}

void Bitmap::rectangle( int x1, int y1, int x2, int y2, int color ) const {
    /* TODO */
}

void Bitmap::rectangleFill( int x1, int y1, int x2, int y2, int color ) const {
    /* TODO */
}

void Bitmap::triangle( int x1, int y1, int x2, int y2, int x3, int y3, int color ) const {
    /* TODO */
}

void Bitmap::polygon( const int * verts, const int nverts, const int color ) const {
    /* TODO */
}

void Bitmap::ellipse( int x, int y, int rx, int ry, int color ) const {
    /* TODO */
}

void Bitmap::ellipseFill( int x, int y, int rx, int ry, int color ) const {
    /* TODO */
}

void Bitmap::applyTrans(const int color) const {
    /* TODO */
}

void Bitmap::light(int x, int y, int width, int height, int start_y, int focus_alpha, int edge_alpha, int focus_color, int edge_color) const {
    /* TODO */
}

void Bitmap::drawCharacter( const int x, const int y, const int color, const int background, const Bitmap & where ) const {
    /* TODO */
}

void Bitmap::save( const std::string & str ) const {
    /* TODO */
}

void Bitmap::readLine( std::vector< int > & line, int y ){
    /* TODO */
}

void TranslucentBitmap::draw(const int x, const int y, const Bitmap & where) const {
    /* FIXME */
    // al_set_target_bitmap(where.getData().getBitmap());
    // al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE);
    al_draw_bitmap(getData().getBitmap(), x, y, 0);
}

void LitBitmap::draw( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
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
    /* TODO */
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

void TranslucentBitmap::hLine( const int x1, const int y, const int x2, const int color ) const {
    /* TODO */
}

void TranslucentBitmap::circleFill(int x, int y, int radius, int color) const {
    /* TODO */
}

void TranslucentBitmap::putPixelNormal(int x, int y, int color) const {
    /* TODO */
}

void TranslucentBitmap::rectangle( int x1, int y1, int x2, int y2, int color ) const {
    /* TODO */
}

void TranslucentBitmap::rectangleFill(int x1, int y1, int x2, int y2, int color) const {
    /* TODO */
}

void TranslucentBitmap::ellipse( int x, int y, int rx, int ry, int color ) const {
    /* TODO */
}

void Bitmap::initializeExtraStuff(){
    al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_RGB_565);
}

Bitmap & Bitmap::operator=(const Bitmap & copy){
    releaseInternalBitmap();
    path = copy.getPath();
    getData().setBitmap(copy.getData().getBitmap());
    // own = false;
    own = copy.own;
    if (own)
        *own += 1;
    return *this;
}

void Bitmap::setClipRect( int x1, int y1, int x2, int y2 ) const {
    /* TODO */
}

void Bitmap::getClipRect(int & x1, int & y1, int & x2, int & y2) const {
    /* TODO */
}

void Bitmap::destroyPrivateData(){
    al_destroy_bitmap(getData().getBitmap());
}

int Bitmap::setGfxModeFullscreen(int x, int y){
    return setGraphicsMode(FULLSCREEN, x, y);
}

int Bitmap::setGfxModeWindowed( int x, int y ){
    return setGraphicsMode(WINDOWED, x, y);
}

int Bitmap::setGfxModeText(){
    /* TODO */
    return 0;
}

bool Bitmap::getError(){
    /* TODO */
    return false;
}

void Bitmap::addBlender( int r, int g, int b, int a ){
    /* TODO */
}

void Bitmap::differenceBlender( int r, int g, int b, int a ){
    /* TODO */
}

void Bitmap::multiplyBlender( int r, int g, int b, int a ){
    /* TODO */
}

void Bitmap::drawingMode(int type){
    /* TODO */
}

void Bitmap::shutdown(){
    delete Screen;
    Screen = NULL;
    /*
    delete Scaler;
    Scaler = NULL;
    delete Buffer;
    Buffer = NULL;
    */
}

}
