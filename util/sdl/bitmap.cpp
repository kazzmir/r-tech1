#include "../bitmap.h"
#include "../lit_bitmap.h"
#include "stretch/SDL_stretch.h"
#include "sprig/sprig.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_gfxPrimitives.h>
#include <exception>

static const int FULLSCREEN = 0;
/* bits per pixel */
static int SCREEN_DEPTH = 16;
static SDL_Surface * screen;

typedef unsigned int (*blender)(unsigned int color1, unsigned int color2, unsigned int alpha);

/* taken from allegro 4.2: src/colblend.c, _blender_trans16 */
static unsigned int transBlender(unsigned int x, unsigned int y, unsigned int n){
    unsigned long result;

    if (n)
        n = (n + 1) / 8;

    x = ((x & 0xFFFF) | (x << 16)) & 0x7E0F81F;
    y = ((y & 0xFFFF) | (y << 16)) & 0x7E0F81F;

    result = ((x - y) * n / 32 + y) & 0x7E0F81F;

    return ((result & 0xFFFF) | (result >> 16));
}

static unsigned int noBlender(unsigned int a, unsigned int b, unsigned int c){
    return a;
}

struct BlendingData{
    BlendingData():
        red(0), green(0), blue(0), alpha(0), currentBlender(noBlender){}

    int red, green, blue, alpha;
    blender currentBlender;
};

static BlendingData globalBlend;
static int drawingMode = Bitmap::MODE_SOLID;

static int drawingAlpha(){
    switch (::drawingMode){
        case Bitmap::MODE_SOLID : return 255;
        case Bitmap::MODE_TRANS : return globalBlend.alpha;
        default : return 255;
    }
}

static void paintown_draw_sprite_ex16(SDL_Surface * dst, SDL_Surface * src, int dx, int dy, int mode, int flip );

const int Bitmap::MaskColor(){
    static int mask = makeColor(255, 0, 255);
    return mask;
}

Bitmap * Bitmap::Screen = NULL;
static Bitmap * Scaler = NULL;
static Bitmap * Buffer = NULL;

void BitmapData::setSurface(SDL_Surface * surface){
    this->surface = surface;
    clip_left = 0;
    clip_right = surface->w - 1;
    clip_top = 0;
    clip_bottom = surface->h - 1;
}

Bitmap::Bitmap():
own(NULL){
    /* TODO */
}

Bitmap::Bitmap(SDL_Surface * who, bool deep_copy):
own(NULL){
    if (deep_copy){
        SDL_Surface * surface = SDL_CreateRGBSurface(SDL_SWSURFACE, who->w, who->h, SCREEN_DEPTH, 0, 0, 0, 0);
        SDL_Rect source;
        SDL_Rect destination;
        source.w = surface->w;
        source.h = surface->h;
        source.x = 0;
        source.y = 0;

        destination.w = surface->w;
        destination.h = surface->h;
        destination.x = 0;
        destination.y = 0;

        SDL_BlitSurface(who, &source, surface, &destination);
        getData().setSurface(surface);
        own = new int;
        *own = 1;
    } else {
        /* FIXME: handle deep_copy */
        getData().setSurface(who);
    }
}

Bitmap::Bitmap(int w, int h){
    SDL_Surface * surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, SCREEN_DEPTH, 0, 0, 0, 0);
    if (surface == NULL){
        /* FIXME */
        throw std::exception();
    }
    getData().setSurface(surface);
    own = new int;
    *own = 1;
}

Bitmap::Bitmap( const char * load_file ):
own(NULL){
    internalLoadFile(load_file);
}

Bitmap::Bitmap( const std::string & load_file ):
own(NULL){
    internalLoadFile(load_file.c_str());
}

Bitmap::Bitmap( const char * load_file, int sx, int sy ):
own(NULL){
    /* TODO */
}

Bitmap::Bitmap( const char * load_file, int sx, int sy, double accuracy ):
own(NULL){
    /* TODO */
}

Bitmap::Bitmap( const Bitmap & copy, bool deep_copy):
own(NULL){
    if (deep_copy){
        SDL_Surface * who = copy.getData().getSurface();
        SDL_Surface * surface = SDL_CreateRGBSurface(SDL_SWSURFACE, who->w, who->h, SCREEN_DEPTH, 0, 0, 0, 0);
        SDL_Rect source;
        SDL_Rect destination;
        source.w = surface->w;
        source.h = surface->h;
        source.x = 0;
        source.y = 0;

        destination.w = surface->w;
        destination.h = surface->h;
        destination.x = 0;
        destination.y = 0;

        SDL_BlitSurface(who, &source, surface, &destination);
        getData().setSurface(surface);
        own = new int;
        *own = 1;
    } else {
        getData().setSurface(copy.getData().getSurface());
        own = copy.own;
        if (own){
            *own += 1;
        }
    }
}

Bitmap::Bitmap( const Bitmap & copy, int sx, int sy ):
own(NULL){
    /* TODO */
}

Bitmap::Bitmap( const Bitmap & copy, int sx, int sy, double accuracy ):
own(NULL){
    /* TODO */
}

static Uint8* computeOffset(SDL_Surface * surface, int x, int y){
    int bpp = surface->format->BytesPerPixel;
    return ((Uint8*)surface->pixels) + y * surface->pitch + x * bpp;
}

Bitmap::Bitmap( const Bitmap & copy, int x, int y, int width, int height ):
own(NULL){
    /* TODO */
    path = copy.getPath();
    SDL_Surface * his = copy.getData().getSurface();
    if ( x < 0 )
        x = 0;
    if ( y < 0 )
        y = 0;
    if ( width > his->w )
        width = his->w;
    if ( height > his->h )
        height = his->h;

    SDL_Surface * sub = SDL_CreateRGBSurfaceFrom(computeOffset(his, x, y), width, height, SCREEN_DEPTH, his->pitch, 0, 0, 0, 0);
    getData().setSurface(sub);

    own = new int;
    *own = 1;
}

void Bitmap::internalLoadFile(const char * path){
    this->path = path;
    SDL_Surface * loaded = IMG_Load(path);
    if (loaded){
        getData().setSurface(SDL_DisplayFormat(loaded));
        SDL_FreeSurface(loaded);
    } else {
        /* FIXME: throw a standard bitmap exception */
        throw std::exception();
    }
    own = new int;
    *own = 1;
}

int Bitmap::getWidth() const {
    if (getData().getSurface() != NULL){
        return getData().getSurface()->w;
    }

    return 0;
}

int Bitmap::getHeight() const {
    if (getData().getSurface() != NULL){
        return getData().getSurface()->h;
    }

    return 0;
}

int Bitmap::getRed(int c){
    Uint8 red = 0;
    Uint8 green = 0;
    Uint8 blue = 0;
    SDL_GetRGB(c, Screen->getData().getSurface()->format, &red, &green, &blue);
    return red;
}

int Bitmap::getBlue(int c){
    Uint8 red = 0;
    Uint8 green = 0;
    Uint8 blue = 0;
    SDL_GetRGB(c, Screen->getData().getSurface()->format, &red, &green, &blue);
    return blue;
}

int Bitmap::getGreen(int c){
    Uint8 red = 0;
    Uint8 green = 0;
    Uint8 blue = 0;
    SDL_GetRGB(c, Screen->getData().getSurface()->format, &red, &green, &blue);
    return green;
}

int Bitmap::makeColor(int red, int blue, int green){
    return SDL_MapRGB(Screen->getData().getSurface()->format, red, blue, green);
}
	
int Bitmap::setGraphicsMode(int mode, int width, int height){
    switch (mode){
        default: {
        // case WINDOWED : {
            screen = SDL_SetVideoMode(width, height, SCREEN_DEPTH, SDL_HWSURFACE | SDL_DOUBLEBUF);
            // screen = SDL_SetVideoMode(width, height, SCREEN_DEPTH, SDL_SWSURFACE | SDL_DOUBLEBUF);
            if (!screen){
                return 1;
            }
            break;
        }
    }

    if (SCALE_X == 0){
        SCALE_X = width;
    }
    if (SCALE_Y == 0){
        SCALE_Y = height;
    }
    if ( Screen != NULL ){
        delete Screen;
        Screen = NULL;
    }

    if ( Scaler != NULL ){
        delete Scaler;
        Scaler = NULL;
    }

    if ( Buffer != NULL ){
        delete Buffer;
        Buffer = NULL;
    }

    if (width != 0 && height != 0){
        Screen = new Bitmap(screen);
        if ( width != 0 && height != 0 && (width != SCALE_X || height != SCALE_Y) ){
            Scaler = new Bitmap(width, height);
            Buffer = new Bitmap(SCALE_X, SCALE_Y);
        }
    }

    return 0;
}
	
void Bitmap::addBlender( int r, int g, int b, int a ){
    /* TODO */
}
	
void Bitmap::multiplyBlender( int r, int g, int b, int a ){
    /* TODO */
}
	
void Bitmap::differenceBlender( int r, int g, int b, int a ){
    /* TODO */
}
	
Bitmap & Bitmap::operator=(const Bitmap &){
    /* TODO */
    return *this;
}
        
int Bitmap::setGfxModeText(){
    /* TODO */
    return 0;
}

int Bitmap::setGfxModeFullscreen(int x, int y){
    return setGraphicsMode(FULLSCREEN, x, y);
}
	
void Bitmap::drawingMode(int type){
    ::drawingMode = type;
}
	
void Bitmap::transBlender( int r, int g, int b, int a ){
    globalBlend.red = r;
    globalBlend.green = g;
    globalBlend.blue = b;
    globalBlend.alpha = a;
    globalBlend.currentBlender = ::transBlender;
}
	
void Bitmap::setClipRect( int x1, int y1, int x2, int y2 ) const {
    getData().setClip(x1, y1, x2, y2);
    SDL_Rect area;
    area.x = x1;
    area.y = y1;
    area.w = x2 - x1;
    area.h = y2 - y1;
    SDL_SetClipRect(getData().getSurface(), &area);
}

void Bitmap::destroyPrivateData(){
    SDL_FreeSurface(getData().getSurface());
}

void Bitmap::putPixel(int x, int y, int pixel) const {
    SDL_Surface * surface = getData().getSurface();

    /* clip it */
    if (getData().isClipped(x, y)){
        return;
    }

    if (SDL_MUSTLOCK(surface)){
        SDL_LockSurface(surface);
    }

    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = computeOffset(surface, x, y);

    switch(bpp) {
        case 1:
            *p = pixel;
            break;

        case 2:
            switch (::drawingMode){
                case MODE_SOLID : {
                    *(Uint16 *)p = pixel;
                    break;
                }
                case MODE_TRANS : {
                    *(Uint16 *)p = globalBlend.currentBlender(pixel, *(Uint16*)p, globalBlend.alpha);
                    break;
                }
            }
            break;

        case 3:
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            } else {
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }
            break;
        case 4:
            *(Uint32 *)p = pixel;
            break;
    }

    if (SDL_MUSTLOCK(surface)){
        SDL_UnlockSurface(surface);
    }
}
	
void Bitmap::putPixelNormal(int x, int y, int col) const {
    putPixel(x, y, col);
}
	
bool Bitmap::getError(){
    /* TODO */
    return false;
}

void Bitmap::rectangle( int x1, int y1, int x2, int y2, int color ) const {
    Uint8 red, green, blue;
    SDL_GetRGB(color, getData().getSurface()->format, &red, &green, &blue);
    rectangleRGBA(getData().getSurface(), x1, y1, x2, y2, red, green, blue, 255);
}

void Bitmap::rectangleFill( int x1, int y1, int x2, int y2, int color ) const {
    Uint8 red, green, blue;
    SDL_GetRGB(color, getData().getSurface()->format, &red, &green, &blue);
    boxRGBA(getData().getSurface(), x1, y1, x2, y2, red, green, blue, 255);
}

void Bitmap::circleFill(int x, int y, int radius, int color) const {
    Uint8 red, green, blue;
    SDL_GetRGB(color, getData().getSurface()->format, &red, &green, &blue);
    int alpha = 255;
    switch (::drawingMode){
        case MODE_SOLID : alpha = 255; break;
        case MODE_TRANS : alpha = globalBlend.alpha; break;
    }

    filledCircleRGBA(getData().getSurface(), x, y, radius, red, green, blue, alpha);
}

void Bitmap::circle(int x, int y, int radius, int color) const {
    circleColor(getData().getSurface(), x, y, radius, color);
}

void Bitmap::line( const int x1, const int y1, const int x2, const int y2, const int color ) const {
    Uint8 red, green, blue;
    SDL_GetRGB(color, getData().getSurface()->format, &red, &green, &blue);
    lineRGBA(getData().getSurface(), x1, y1, x2, y2, red, green, blue, drawingAlpha());
}

void Bitmap::draw(const int x, const int y, const Bitmap & where) const {
    if (getData().getSurface() != NULL){
	paintown_draw_sprite_ex16(where.getData().getSurface(), getData().getSurface(), x, y, Bitmap::SPRITE_NORMAL, Bitmap::SPRITE_NO_FLIP);
        /*
        SDL_SetColorKey(getData().getSurface(), SDL_SRCCOLORKEY, makeColor(255, 0, 255));
        Blit(x, y, where);
        */
    }
}

void Bitmap::draw(const int x, const int y, const int startWidth, const int startHeight, const int width, const int height, const Bitmap & where) const {
    /* TODO */
}

void Bitmap::drawHFlip(const int x, const int y, const Bitmap & where) const {
    paintown_draw_sprite_ex16( where.getData().getSurface(), getData().getSurface(), x, y, Bitmap::SPRITE_NORMAL, Bitmap::SPRITE_H_FLIP );
}

void Bitmap::drawVFlip( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void Bitmap::drawHVFlip( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void Bitmap::drawTrans( const int x, const int y, const Bitmap & where ) const {
    /* FIXME */
    // draw(x, y, where);
    paintown_draw_sprite_ex16(where.getData().getSurface(), getData().getSurface(), x, y, Bitmap::SPRITE_TRANS, Bitmap::SPRITE_NO_FLIP);
}

void Bitmap::drawTransHFlip( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void Bitmap::drawTransVFlip( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void Bitmap::drawTransHVFlip( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void Bitmap::drawMask( const int x, const int y, const Bitmap & where ){
    /* TODO */
}

void Bitmap::drawStretched( const int x, const int y, const int new_width, const int new_height, const Bitmap & who ){

    if (getData().getSurface() != NULL){
        SDL_SetColorKey(getData().getSurface(), SDL_SRCCOLORKEY, makeColor(255, 0, 255));

        /*
        SDL_Rect source;
        SDL_Rect destination;
        source.x = 0;
        source.y = 0;
        source.w = getWidth();
        source.h = getHeight();

        destination.x = x;
        destination.y = y;
        destination.w = new_width;
        destination.h = new_height;
        */

        /*
        if (x < 0){
            source.x = -x;
            source.w -= -x;
            destination.x += -x;
            destination.w -= -x;
        }
        */

        /*
        source.x = 0;
        source.y = 0;
        source.w = getWidth() / 2;
        source.h = getHeight() / 2;

        destination.x = 0;
        destination.y = 0;
        destination.w = new_width;
        destination.h = new_height;
        */

        SDL_Surface * src = getData().getSurface();
        SDL_Surface * dst = who.getData().getSurface();

        float xscale = (float) new_width / (float) src->w;
        float yscale = (float) new_height / (float) src->h;
        SPG_TransformX(src, dst, 0, xscale, yscale, 0, 0, x, y, SPG_TCOLORKEY);

        /*
        if (SDL_MUSTLOCK(src)){
            SDL_LockSurface(src);
        }

        if (SDL_MUSTLOCK(dst)){
            SDL_LockSurface(dst);
        }

        SDL_StretchSurfaceRect(src, &source, dst, &destination);

        if (SDL_MUSTLOCK(src)){
            SDL_UnlockSurface(src);
        }

        if (SDL_MUSTLOCK(dst)){
            SDL_UnlockSurface(dst);
        }
        */
    }
}

void Bitmap::Blit( const std::string & xpath ) const {
    /* TODO */
}

void Bitmap::Blit( const Bitmap & where ) const {
    Blit(0, 0, where);
}

void Bitmap::Blit( const int x, const int y, const Bitmap & where ) const {
    Blit(0, 0, x, y, where);
}

void Bitmap::Blit( const int mx, const int my, const int wx, const int wy, const Bitmap & where ) const {
    Blit(mx, my, getWidth(), getHeight(), wx, wy, where);
}

void Bitmap::Blit( const int mx, const int my, const int width, const int height, const int wx, const int wy, const Bitmap & where ) const {
    SDL_Rect source;
    SDL_Rect destination;
    source.w = width;
    source.h = height;
    source.x = mx;
    source.y = my;

    destination.w = width;
    destination.h = height;
    destination.x = wx;
    destination.y = wy;

    SDL_BlitSurface(getData().getSurface(), &source, where.getData().getSurface(), &destination);
}

void Bitmap::BlitMasked( const int mx, const int my, const int width, const int height, const int wx, const int wy, const Bitmap & where ) const {
    /* TODO */
}

void Bitmap::BlitToScreen(const int upper_left_x, const int upper_left_y) const {
    if ( Scaler == NULL ){
        this->Blit( upper_left_x, upper_left_y, *Screen );
    } else {
        this->Blit( upper_left_x, upper_left_y, *Buffer );
        Buffer->Stretch(*Scaler);
        Scaler->Blit(0, 0, 0, 0, *Screen);
    }

    SDL_Flip(Screen->getData().getSurface());
    // SDL_UpdateRect(Screen->getData().getSurface(), 0, 0, Screen->getWidth(), Screen->getHeight());
}

void Bitmap::BlitAreaToScreen(const int upper_left_x, const int upper_left_y) const {
    /* TODO */
}

void Bitmap::BlitFromScreen(const int x, const int y) const {
    /* TODO */
}

void Bitmap::Stretch( const Bitmap & where ) const {
    if (getWidth() == where.getWidth() && getHeight() == where.getHeight()){
        Blit(where);
    } else {
        Stretch(where, 0, 0, getWidth(), getHeight(), 0, 0, where.getWidth(), where.getHeight());
    }
}

void Bitmap::Stretch( const Bitmap & where, const int sourceX, const int sourceY, const int sourceWidth, const int sourceHeight, const int destX, const int destY, const int destWidth, const int destHeight ) const {

    SDL_Surface * src = getData().getSurface();
    SDL_Surface * dst = where.getData().getSurface();

    float xscale = (float) destWidth / (float) sourceWidth;
    float yscale = (float) destHeight / (float) sourceHeight;
    SPG_TransformX(src, dst, 0, xscale, yscale, sourceX, sourceY, destX, destY, SPG_NONE);

    /*
    SDL_Rect source;
    SDL_Rect destination;
    source.x = sourceX;
    source.y = sourceY;
    source.w = sourceWidth;
    source.h = sourceHeight;

    destination.x = destX;
    destination.y = destY;
    destination.w = destWidth;
    destination.h = destHeight;

    // SDL_StretchSurfaceRect(getData().getSurface(), &source, where.getData().getSurface(), &destination);

    SDL_Surface * src = getData().getSurface();
    SDL_Surface * dst = where.getData().getSurface();

    if (SDL_MUSTLOCK(src)){
        SDL_LockSurface(src);
    }

    if (SDL_MUSTLOCK(dst)){
        SDL_LockSurface(dst);
    }

    SDL_StretchSurfaceRect(src, &source, dst, &destination);

    if (SDL_MUSTLOCK(src)){
        SDL_UnlockSurface(src);
    }

    if (SDL_MUSTLOCK(dst)){
        SDL_UnlockSurface(dst);
    }
    */
}
	
void Bitmap::save( const std::string & str ){
    /* TODO */
}
	
void Bitmap::triangle( int x1, int y1, int x2, int y2, int x3, int y3, int color ) const {
    /* TODO */
}

void Bitmap::ellipse( int x, int y, int rx, int ry, int color ) const {
    /* TODO */
}

void Bitmap::ellipseFill( int x, int y, int rx, int ry, int color ) const {
    /* TODO */
}

void Bitmap::light(int x, int y, int width, int height, int start_y, int focus_alpha, int edge_alpha, int focus_color, int edge_color) const {
    /* TODO */
}

void Bitmap::applyTrans(const int color){
    /* TODO */
}
	
void Bitmap::floodfill( const int x, const int y, const int color ) const {
    /* TODO */
}
	
void Bitmap::horizontalLine( const int x1, const int y, const int x2, const int color ) const {
    /* TODO */
}

void Bitmap::hLine( const int x1, const int y, const int x2, const int color ) const {
    /* TODO */
}

void Bitmap::vLine( const int y1, const int x, const int y2, const int color ) const {
    /* TODO */
}
	
void Bitmap::polygon( const int * verts, const int nverts, const int color ) const {
    /* TODO */
}
	
void Bitmap::arc(const int x, const int y, const double ang1, const double ang2, const int radius, const int color ) const {
    /* TODO */
    // arcColor(getData().getSurface(), x, y, 
}
	
void Bitmap::fill(int color) const {
    SDL_Rect area;
    area.x = 0;
    area.y = 0;
    area.w = getWidth();
    area.h = getHeight();
    SDL_FillRect(getData().getSurface(), &area, color);
}
	
int Bitmap::darken( int color, double factor ){
    /* TODO */
    return color;
}
        
Bitmap Bitmap::greyScale(){
    /* TODO */
    return *this;
}
	
void Bitmap::drawCharacter( const int x, const int y, const int color, const int background, const Bitmap & where ) const {
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
        
Bitmap Bitmap::memoryPCX(unsigned char * const data, const int length, const bool mask){
    SDL_RWops * ops = SDL_RWFromConstMem(data, length);
    SDL_Surface * pcx = IMG_LoadPCX_RW(ops);
    SDL_FreeRW(ops);
    Bitmap out(pcx, true);
    SDL_FreeSurface(pcx);
    return out;
}
	
int Bitmap::getPixel( const int x, const int y ) const {
    /* TODO */
    return 0;
}
	
void Bitmap::readLine( std::vector< int > & vec, int y ){
    /* TODO */
}

void Bitmap::StretchBy2( const Bitmap & where ){
    /* TODO */
}

void Bitmap::StretchBy4( const Bitmap & where ){
    /* TODO */
}

void LitBitmap::draw( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void LitBitmap::drawHFlip( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void LitBitmap::drawVFlip( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void LitBitmap::drawHVFlip( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

/*
#define PAINTOWN_DLS_BLENDER BLENDER_FUNC
#define PAINTOWN_DTS_BLENDER BLENDER_FUNC
#define PAINTOWN_MAKE_DLS_BLENDER(a) _blender_func16
#define PAINTOWN_MAKE_DTS_BLENDER() _blender_func16
#define PAINTOWN_PIXEL_PTR unsigned short*
#define PAINTOWN_OFFSET_PIXEL_PTR(p,x) ((PAINTOWN_PIXEL_PTR) (p) + (x))
#define PAINTOWN_INC_PIXEL_PTR(p) ((p)++)
#define PAINTOWN_INC_PIXEL_PTR_EX(p,d)  ((p) += d)
#define PAINTOWN_GET_MEMORY_PIXEL(p)    (*(p))
#define PAINTOWN_IS_SPRITE_MASK(b,c)    ((unsigned long) (c) == (unsigned long) MASK_COLOR)
#define PAINTOWN_DLSX_BLEND(b,n)       ((*(b))(_blender_col_16, (n), globalBlend.alpha))
#define PAINTOWN_GET_PIXEL(p)           *((unsigned short *) (p))
#define PAINTOWN_DTS_BLEND(b,o,n)       ((*(b))((n), (o), globalBlend.alpha))
#define PAINTOWN_PUT_PIXEL(p,c)         (*((unsigned short *) p) = (c))
#define PAINTOWN_PUT_MEMORY_PIXEL(p,c)  (*(p) = (c))
#define PAINTOWN_SET_ALPHA(a)           (globalBlend.alpha = (a))
*/

static void paintown_draw_sprite_ex16(SDL_Surface * dst, SDL_Surface * src, int dx, int dy, int mode, int flip){
    int x, y, w, h;
    int x_dir = 1, y_dir = 1;
    int dxbeg, dybeg;
    int sxbeg, sybeg;
    /*
    PAINTOWN_DLS_BLENDER lit_blender;
    PAINTOWN_DTS_BLENDER trans_blender;
    */

    /*
    ASSERT(dst);
    ASSERT(src);
    */
    
    if (SDL_MUSTLOCK(src)){
        SDL_LockSurface(src);
    }

    if (SDL_MUSTLOCK(dst)){
        SDL_LockSurface(dst);
    }

    if ( flip & Bitmap::SPRITE_V_FLIP ){
        y_dir = -1;
    }
    if ( flip & Bitmap::SPRITE_H_FLIP ){
        x_dir = -1;
    }

    if (true /* dst->clip*/ ) {
        int tmp;

        tmp = dst->clip_rect.x - dx;
        sxbeg = ((tmp < 0) ? 0 : tmp);
        dxbeg = sxbeg + dx;

        tmp = dst->clip_rect.x + dst->clip_rect.w - dx;
        w = ((tmp > src->w) ? src->w : tmp) - sxbeg;
        if (w <= 0)
            return;

        if ( flip & Bitmap::SPRITE_H_FLIP ){
            /* use backward drawing onto dst */
            sxbeg = src->w - (sxbeg + w);
            dxbeg += w - 1;
        }

        tmp = dst->clip_rect.y - dy;
        sybeg = ((tmp < 0) ? 0 : tmp);
        dybeg = sybeg + dy;

        tmp = dst->clip_rect.y + dst->clip_rect.h - dy;
        h = ((tmp > src->h) ? src->h : tmp) - sybeg;
        if (h <= 0)
            return;

        if ( flip & Bitmap::SPRITE_V_FLIP ){
            /* use backward drawing onto dst */
            sybeg = src->h - (sybeg + h);
            dybeg += h - 1;
        }
    } else {
        w = src->w;
        h = src->h;
        sxbeg = 0;
        sybeg = 0;
        dxbeg = dx;
        if ( flip & Bitmap::SPRITE_H_FLIP ){
            dxbeg = dx + w - 1;
        }
        dybeg = dy;
        if ( flip & Bitmap::SPRITE_V_FLIP ){
            dybeg = dy + h - 1;
        }
    }

    /*
    lit_blender = PAINTOWN_MAKE_DLS_BLENDER(0);
    trans_blender = PAINTOWN_MAKE_DTS_BLENDER();
    */

#if 0
    if (dst->id & (BMP_ID_VIDEO | BMP_ID_SYSTEM)) {
        bmp_select(dst);

        switch (mode){
            case Bitmap::SPRITE_NORMAL : {
                                             for (y = 0; y < h; y++) {
                                                 PAINTOWN_PIXEL_PTR s = PAINTOWN_OFFSET_PIXEL_PTR(src->line[sybeg + y], sxbeg);
                                                 PAINTOWN_PIXEL_PTR d = PAINTOWN_OFFSET_PIXEL_PTR(bmp_write_line(dst, dybeg + y * y_dir), dxbeg);
                                                 for (x = w - 1; x >= 0; PAINTOWN_INC_PIXEL_PTR(s), PAINTOWN_INC_PIXEL_PTR_EX(d,x_dir), x--) {

                                                     unsigned long c = PAINTOWN_GET_MEMORY_PIXEL(s);
                                                     if (!PAINTOWN_IS_SPRITE_MASK(src, c)) {
                                                         PAINTOWN_PUT_PIXEL(d, c);
                                                     }
                                                 }
                                             }

                                             break;
                                         }
            case Bitmap::SPRITE_LIT : {
                                          for (y = 0; y < h; y++) {
                                              PAINTOWN_PIXEL_PTR s = PAINTOWN_OFFSET_PIXEL_PTR(src->line[sybeg + y], sxbeg);
                                              PAINTOWN_PIXEL_PTR d = PAINTOWN_OFFSET_PIXEL_PTR(bmp_write_line(dst, dybeg + y * y_dir), dxbeg);
                                              for (x = w - 1; x >= 0; PAINTOWN_INC_PIXEL_PTR(s), PAINTOWN_INC_PIXEL_PTR_EX(d,x_dir), x--) {

                                                  unsigned long c = PAINTOWN_GET_MEMORY_PIXEL(s);
                                                  if (!PAINTOWN_IS_SPRITE_MASK(src, c)) {
                                                      c = PAINTOWN_DLSX_BLEND(lit_blender, c);
                                                      PAINTOWN_PUT_PIXEL(d, c);
                                                  }
                                              }
                                          }
                                          break;
                                      }
            case Bitmap::SPRITE_TRANS : {
                                            for (y = 0; y < h; y++) {
                                                PAINTOWN_PIXEL_PTR s = PAINTOWN_OFFSET_PIXEL_PTR(src->line[sybeg + y], sxbeg);
                                                PAINTOWN_PIXEL_PTR d = PAINTOWN_OFFSET_PIXEL_PTR(bmp_write_line(dst, dybeg + y * y_dir), dxbeg);
                                                for (x = w - 1; x >= 0; PAINTOWN_INC_PIXEL_PTR(s), PAINTOWN_INC_PIXEL_PTR_EX(d,x_dir), x--) {

                                                    unsigned long c = PAINTOWN_GET_MEMORY_PIXEL(s);
                                                    if (!PAINTOWN_IS_SPRITE_MASK(src, c)) {
                                                        c = PAINTOWN_DTS_BLEND(trans_blender, PAINTOWN_GET_PIXEL(d), c);
                                                        PAINTOWN_PUT_PIXEL(d, c);
                                                    }
                                                }
                                            }
                                            break;
                                        }
        }
#xendif

        for (y = 0; y < h; y++) {
            PAINTOWN_PIXEL_PTR s = PAINTOWN_OFFSET_PIXEL_PTR(src->line[sybeg + y], sxbeg);

            /* flipped if y_dir is -1 */
            PAINTOWN_PIXEL_PTR d = PAINTOWN_OFFSET_PIXEL_PTR(bmp_write_line(dst, dybeg + y * y_dir), dxbeg);

            /* d is incremented by x_dir, -1 if flipped */
            for (x = w - 1; x >= 0; PAINTOWN_INC_PIXEL_PTR(s), PAINTOWN_INC_PIXEL_PTR_EX(d,x_dir), x--) {
                unsigned long c = PAINTOWN_GET_MEMORY_PIXEL(s);
                if (!PAINTOWN_IS_SPRITE_MASK(src, c)) {
                    switch( mode ){
                        case Bitmap::SPRITE_NORMAL : break;
                        case Bitmap::SPRITE_LIT : {
                                                      c = PAINTOWN_DLSX_BLEND(lit_blender, c);
                                                      break;
                                                  }
                        case Bitmap::SPRITE_TRANS : {
                                                        c = PAINTOWN_DTS_BLEND(trans_blender, PAINTOWN_GET_PIXEL(d), c);
                                                        break;
                                                    }
                    }
                    PAINTOWN_PUT_PIXEL(d, c);
                }
            }
        }

        bmp_unwrite_line(dst);
    }
    else {
#endif
        {

        switch (mode){
            case Bitmap::SPRITE_NORMAL : {
                unsigned int mask = Bitmap::makeColor(255, 0, 255);
                int bpp = src->format->BytesPerPixel;
                for (y = 0; y < h; y++) {
                    Uint8 * sourceLine = computeOffset(src, sxbeg, sybeg + y);
                    Uint8 * destLine = computeOffset(dst, dxbeg, dybeg + y * y_dir);

                    for (x = w - 1; x >= 0; sourceLine += bpp, destLine += bpp * x_dir, x--) {
                        unsigned long sourcePixel = *(Uint16*) sourceLine;
                        if (!(sourcePixel == mask)){
                            // unsigned int destPixel = *(Uint16*) destLine;
                            // sourcePixel = globalBlend.currentBlender(destPixel, sourcePixel, globalBlend.alpha);
                            *(Uint16 *)destLine = sourcePixel;
                        }
                    }
                }
             }
                                         /*
             case Bitmap::SPRITE_LIT : {
                                          for (y = 0; y < h; y++) {
                                              PAINTOWN_PIXEL_PTR s = PAINTOWN_OFFSET_PIXEL_PTR(src->line[sybeg + y], sxbeg);
                                              PAINTOWN_PIXEL_PTR d = PAINTOWN_OFFSET_PIXEL_PTR(bmp_write_line(dst, dybeg + y * y_dir), dxbeg);

                                              for (x = w - 1; x >= 0; PAINTOWN_INC_PIXEL_PTR(s), PAINTOWN_INC_PIXEL_PTR_EX(d,x_dir), x--) {
                                                  unsigned long c = PAINTOWN_GET_MEMORY_PIXEL(s);
                                                  if (!PAINTOWN_IS_SPRITE_MASK(src, c)) {
                                                      c = PAINTOWN_DLSX_BLEND(lit_blender, c);
                                                      PAINTOWN_PUT_MEMORY_PIXEL(d, c);
                                                  }
                                              }
                                          }
                                          break;
                                      }
                                      */
            case Bitmap::SPRITE_TRANS : {
                int bpp = src->format->BytesPerPixel;
                unsigned int mask = Bitmap::makeColor(255, 0, 255);
                for (y = 0; y < h; y++) {
                    Uint8 * sourceLine = computeOffset(src, sxbeg, sybeg + y);
                    Uint8 * destLine = computeOffset(dst, dxbeg, dybeg + y * y_dir);

                    for (x = w - 1; x >= 0; sourceLine += bpp, destLine += bpp * x_dir, x--) {
                        unsigned long sourcePixel = *(Uint16*) sourceLine;
                        if (!(sourcePixel == mask)){
                            unsigned int destPixel = *(Uint16*) destLine;
                            sourcePixel = globalBlend.currentBlender(destPixel, sourcePixel, globalBlend.alpha);
                            *(Uint16 *)destLine = sourcePixel;
                        }
                    }
                }
                break;
            }
            default : { break; }
        }

#if 0
        for (y = 0; y < h; y++) {
            PAINTOWN_PIXEL_PTR s = PAINTOWN_OFFSET_PIXEL_PTR(src->line[sybeg + y], sxbeg);
            PAINTOWN_PIXEL_PTR d = PAINTOWN_OFFSET_PIXEL_PTR(bmp_write_line(dst, dybeg + y * y_dir), dxbeg);

            for (x = w - 1; x >= 0; PAINTOWN_INC_PIXEL_PTR(s), PAINTOWN_INC_PIXEL_PTR_EX(d,x_dir), x--) {
                unsigned long c = PAINTOWN_GET_MEMORY_PIXEL(s);
                if (!PAINTOWN_IS_SPRITE_MASK(src, c)) {
                    switch( mode ){
                        case Bitmap::SPRITE_NORMAL : break;
                        case Bitmap::SPRITE_LIT : {
                                                      c = PAINTOWN_DLSX_BLEND(lit_blender, c);
                                                      break;
                                                  }
                        case Bitmap::SPRITE_TRANS : {
                                                        c = PAINTOWN_DTS_BLEND(trans_blender, PAINTOWN_GET_PIXEL(d), c);
                                                        break;
                                                    }
                    }
                    PAINTOWN_PUT_MEMORY_PIXEL(d, c);
                }
            }
        }
#endif
    }
    
    if (SDL_MUSTLOCK(src)){
        SDL_UnlockSurface(src);
    }

    if (SDL_MUSTLOCK(dst)){
        SDL_UnlockSurface(dst);
    }
}
