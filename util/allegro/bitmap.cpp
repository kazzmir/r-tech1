/* allegro.h must be on top, don't move it!!!! */
#include <allegro.h>

/* FIXME: I think this can be just #include <winalleg.h> */
#ifdef _WIN32
#include <winalleg.h>
/*
#define BITMAP dummyBITMAP
#include <windows.h>
#undef BITMAP
*/
#endif

#include "../gif/algif.h"
#include "../bitmap.h"
#include "../lit_bitmap.h"
#include "../trans-bitmap.h"
#include <stdarg.h>
#include <vector>
#include <string>
#include <iostream>
#include <math.h>
#include "../funcs.h"
#include <stdio.h>
#include "../load_exception.h"
#include "../memory.h"
#include <sstream>
// #include <fblend.h>

#ifdef _WIN32
#include <winalleg.h>
#define EXTERNAL_VARIABLE __declspec(dllimport)
#else
#define EXTERNAL_VARIABLE
#endif

using namespace std;

#ifndef debug
#define debug cout<<"File: "<<__FILE__<<" Line: "<<__LINE__<<endl;
#endif

static void paintown_draw_sprite_ex16( BITMAP * dst, BITMAP * src, int dx, int dy, int mode, int flip );
static void paintown_draw_sprite_filter_ex16(BITMAP * dst, BITMAP * src, int x, int y, const Bitmap::Filter & filter);
static void paintown_light16(BITMAP * dst, const int x, const int y, const int width, const int height, const int start_y, const int focus_alpha, const int edge_alpha, const int focus_color, const int edge_color);
static void paintown_applyTrans16(BITMAP * dst, const int color);

int Bitmap::MaskColor(){
    return MASK_COLOR_16;
}

Bitmap * Bitmap::Screen = NULL;
static Bitmap * Scaler = NULL;
static Bitmap * Buffer = NULL;

Bitmap::Bitmap():
own( NULL ),
mustResize(false),
error( false ),
bit8MaskColor(0){
	getData().setBitmap( create_bitmap( 10, 10 ) );
	if (! getData().getBitmap()){
		error = true;
		cerr << "Could not create bitmap!" << endl;
	} else {
		clear();
		own = new int;
		*own = 1;
	}
}

Bitmap::Bitmap( int x, int y ):
own( NULL ),
mustResize(false),
error( false ),
bit8MaskColor(0){
	if ( x < 1 ){
		x = 1;
	}
	if ( y < 1 ){
		y = 1;
	}
	getData().setBitmap( create_bitmap( x, y ) );
	if ( ! getData().getBitmap() ){
		error = true;
		cerr << "Could not create bitmap!" << endl;
	} else {
		clear();
		own = new int;
		*own = 1;
	}
}

enum Format{
    PNG,
    GIF,
};

static BITMAP * memoryGIF(const char * data, int length){
    PACKFILE_VTABLE table = Memory::makeTable();
    Memory::memory memory((unsigned char *) data, length);

    PACKFILE * pack = pack_fopen_vtable(&table, &memory);
    /* need to supply a proper palette at some point */
    RGB * palette = NULL;
    /* algif will close the packfile for us in both error and success cases */
    BITMAP * gif = load_gif_packfile(pack, palette);
    if (!gif){
        // pack_fclose(pack);
        ostringstream out;
        out <<"Could not load gif from memory: " << (void*) data << " length " << length;
        throw LoadException(__FILE__, __LINE__, out.str());
    }

#if 0
    /* converts 8-bit pcx mask to allegro's mask */
    if (mask){
        /* warning! 8-bit assumptions */
        int colors = 256;
        int maskR = (int)data[length - colors*3 + 0];
        int maskG = (int)data[length - colors*3 + 1];
        int maskB = (int)data[length - colors*3 + 2];
        int mask = makeColor(maskR, maskG, maskB);

        // printf("mask r %d g %d b %d = %d\n", maskR, maskG, maskB, mask);

        if (mask != MaskColor()){
            for( int i = 0; i < pcx->h; ++i ){
                for( int j = 0; j < pcx->w; ++j ){
                    /* use getPixel/putPixel? */
                    int pix = getpixel(pcx,j,i);
                    if (pix == mask){
                        putpixel(pcx,j,i, MaskColor());
                    }
                }
            }
        }
    }
#endif

    BITMAP * out = create_bitmap(gif->w, gif->h);
    blit(gif, out, 0, 0, 0, 0, gif->w, gif->h);
    destroy_bitmap(gif);
    // pack_fclose(pack);

    return out;
}

static BITMAP * load_bitmap_from_memory(const char * data, int length, Format type){
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
error(false),
bit8MaskColor(0){
    /* FIXME: pass the type in */
    Format type = GIF;
    getData().setBitmap(load_bitmap_from_memory(data, length, type));
    own = new int;
    *own = 1;
}

/* If a BITMAP is given to us, we didn't make it so we don't own it */
Bitmap::Bitmap( BITMAP * who, bool deep_copy ):
own( NULL ),
mustResize(false),
error( false ),
bit8MaskColor(0){
	
	if ( deep_copy ){
		BITMAP * his = who;
		getData().setBitmap( create_bitmap( his->w, his->h ) );
		if ( ! getData().getBitmap() ){
			cout << "Could not create bitmap" << endl;
			error = true;
		}
		::blit( his, getData().getBitmap(), 0, 0, 0, 0, his->w, his->h );
		own = new int;
		*own = 1;
	} else {
		getData().setBitmap( who );
		own = NULL;
	}
}
	
Bitmap::Bitmap( const char * load_file ):
own( NULL ),
mustResize(false),
error( false ),
bit8MaskColor(0){
	internalLoadFile( load_file );
	/*
	my_bitmap = load_bitmap( load_file, NULL );
	if ( !my_bitmap ){
		my_bitmap = create_bitmap( 100, 100 );
		clear( my_bitmap );
		cout<<"Could not load "<<load_file<<endl;
		error = true;
	}
	own = true;
	*/
}

Bitmap::Bitmap( const string & load_file ):
own( NULL ),
mustResize(false),
error( false ),
bit8MaskColor(0){
	internalLoadFile( load_file.c_str() );
}

Bitmap::Bitmap( const char * load_file, int sx, int sy ):
own( NULL ),
mustResize(false),
error( false ),
bit8MaskColor(0){
	path = load_file;
	BITMAP * temp = load_bitmap( load_file, NULL );
	// my_bitmap = load_bitmap( load_file, NULL );
	getData().setBitmap( create_bitmap( sx, sy ) );
	// clear( my_bitmap );
	if ( !temp || ! getData().getBitmap() ){
		cout<<"Could not load "<<load_file<<endl;
		error = true;
	} else {
		clear();
		stretch_blit( temp, getData().getBitmap(), 0, 0, temp->w, temp->h, 0, 0, getData().getBitmap()->w, getData().getBitmap()->h );
		destroy_bitmap( temp );
	}
	// own = true;
	own = new int;
	*own = 1;
}

Bitmap::Bitmap( const char * load_file, int sx, int sy, double accuracy ):
own( NULL ),
mustResize(false),
error( false ),
bit8MaskColor(0){
	path = load_file;
	BITMAP * temp = load_bitmap( load_file, NULL );
	own = new int;
	*own = 1;
	if ( !temp ){
		cout<<"Could not load "<<load_file<<endl;
		getData().setBitmap( create_bitmap( sx, sy ) );
		// clear( my_bitmap );
		clear();
		error = true;
	} else {
		if ( temp->w > sx || temp->h > sy ){
			double bx = temp->w / sx;
			double by = temp->h / sy;
			double use;
			use = bx > by ? bx : by;
			int fx = (int)(sx / use);
			int fy = (int)(sy / use);
			getData().setBitmap( create_bitmap( fx, fy ) );
			
			stretch_blit( temp, getData().getBitmap(), 0, 0, temp->w, temp->h, 0, 0, getData().getBitmap()->w, getData().getBitmap()->h );
			
			destroy_bitmap( temp );
		} else getData().setBitmap( temp );
	}
	// own = true;
}

Bitmap::Bitmap( const Bitmap & copy, int sx, int sy ):
own( NULL ),
mustResize(false),
error( false ),
bit8MaskColor(copy.bit8MaskColor){
	path = copy.getPath();
	BITMAP * temp = copy.getData().getBitmap();
	getData().setBitmap( create_bitmap( sx, sy ) );
	if ( ! getData().getBitmap() ){
		error = true;
		cout << "Could not copy bitmap" << endl;
	}
	// clear( my_bitmap );
	clear();
	stretch_blit( temp, getData().getBitmap(), 0, 0, temp->w, temp->h, 0, 0, getData().getBitmap()->w, getData().getBitmap()->h );
	// own = true;
	// own = copy.own;
	own = new int;
	*own = 1;
}

Bitmap::Bitmap( const Bitmap & copy, int sx, int sy, double accuracy ):
own( NULL ),
mustResize(false),
error( false ),
bit8MaskColor(copy.bit8MaskColor){
	path = copy.getPath();
	BITMAP * temp = copy.getData().getBitmap();
	own = new int;
	*own = 1;
	if ( temp->w > sx || temp->h > sy ){
		double bx = (double)temp->w / (double)sx;
		double by = (double)temp->h / (double)sy;
		double use;
		use = bx > by ? bx : by;
		int fx = (int)(temp->w / use);
		int fy = (int)(temp->h / use);
		getData().setBitmap( create_bitmap( fx, fy ) );
		if ( ! getData().getBitmap() ){
			allegro_message("Could not create bitmap\n");
			// own = false;
			// own = copy.own;
			// *own++;
			error = true;
			return;
		}
	
		stretch_blit( temp, getData().getBitmap(), 0, 0, temp->w, temp->h, 0, 0, getData().getBitmap()->w, getData().getBitmap()->h );
		// destroy_bitmap( temp );
	} else {
		getData().setBitmap( create_bitmap( temp->w, temp->h ) );
		blit( temp, getData().getBitmap(), 0, 0, 0, 0, temp->w, temp->h );
		// own = new int
		// own = true;
	}
}

Bitmap::Bitmap( const Bitmap & copy, bool deep_copy ):
own( NULL ),
mustResize(false),
error( false ),
bit8MaskColor(copy.bit8MaskColor){

	path = copy.getPath();
	if ( deep_copy ){
		BITMAP * his = copy.getData().getBitmap();
		getData().setBitmap( create_bitmap( his->w, his->h ) );
		if ( ! getData().getBitmap() ){
			cout << "Could not create bitmap" << endl;
			error = true;
		}
		::blit( his, getData().getBitmap(), 0, 0, 0, 0, his->w, his->h );
		own = new int;
		*own = 1;
	} else {
		getData().setBitmap( copy.getData().getBitmap() );
		// own = false;
		own = copy.own;
		if ( own ){
			*own = *own + 1;
		}
	}
	/*
	BITMAP * his = copy.getBitmap();
	my_bitmap = create_bitmap( his->w, his->h );
	::blit( his, my_bitmap, 0, 0, 0, 0, his->w, his->h );
	own = true;
	*/
}

Bitmap::Bitmap( const Bitmap & copy, int x, int y, int width, int height ):
own( NULL ),
mustResize(false),
error( false ),
bit8MaskColor(copy.bit8MaskColor){
	path = copy.getPath();
	BITMAP * his = copy.getData().getBitmap();
	if ( x < 0 )
		x = 0;
	if ( y < 0 )
		y = 0;
	if ( width + x > his->w )
		width = his->w - x;
	if ( height + y > his->h )
		height = his->h - y;
	getData().setBitmap( create_sub_bitmap( his, x, y, width, height ) );

	if ( ! getData().getBitmap() ){
		cout<<"Could not create sub-bitmap"<<endl;
		getData().setBitmap( create_bitmap( 10, 10 ) );
		// clear( my_bitmap );
		clear();
	}
	// own = true;
	own = new int;
	*own = 1;
}
	
void Bitmap::internalLoadFile( const char * load_file ){
	path = load_file;
	getData().setBitmap( load_bitmap( load_file, NULL ) );
	if ( ! getData().getBitmap() ){
		cout<<"Could not load "<<load_file<<". Using default"<<endl;
		getData().setBitmap( create_bitmap( 10, 10 ) );
		if ( ! getData().getBitmap() ){
			cout<<"Out of memory or Allegro not initialized"<<endl;
			error = true;
			return;
		}
		// clear( my_bitmap );
		clear();
		// cout<<"Could not load "<<load_file<<endl;
		error = true;
	}
	// own = true;
	own = new int;
	*own = 1;
}
	
void Bitmap::save( const string & str ) const {
    save_bitmap( str.c_str(), getData().getBitmap(), NULL );
}

Bitmap Bitmap::memoryPCX(unsigned char * const data, const int length, const bool mask){
    PACKFILE_VTABLE table = Memory::makeTable();
    Memory::memory memory(data, length);

    PACKFILE * pack = pack_fopen_vtable(&table, &memory);
    /* need to supply a proper palette at some point */
    RGB * palette = NULL;
    BITMAP * pcx = load_pcx_pf(pack, palette);
    if (!pcx){
        pack_fclose(pack);
        ostringstream out;
        out <<"Could not load pcx from memory: " << (void*) data << " length " << length;
        throw LoadException(__FILE__, __LINE__, out.str());
    }

    int colors = 256;
    int maskR = (int)data[length - colors*3 + 0];
    int maskG = (int)data[length - colors*3 + 1];
    int maskB = (int)data[length - colors*3 + 2];
    int maskColor = makeColor(maskR, maskG, maskB);


#if 0
    /* converts 8-bit pcx mask to allegro's mask */
    if (mask){
        /* warning! 8-bit assumptions */
        int colors = 256;
        int maskR = (int)data[length - colors*3 + 0];
        int maskG = (int)data[length - colors*3 + 1];
        int maskB = (int)data[length - colors*3 + 2];
        int mask = makeColor(maskR, maskG, maskB);

        // printf("mask r %d g %d b %d = %d\n", maskR, maskG, maskB, mask);

        if (mask != MaskColor()){
            for( int i = 0; i < pcx->h; ++i ){
                for( int j = 0; j < pcx->w; ++j ){
                    /* use getPixel/putPixel? */
                    int pix = getpixel(pcx,j,i);
                    if (pix == mask){
                        putpixel(pcx,j,i, MaskColor());
                    }
                }
            }
        }
    }
#endif

    Bitmap bitmap(pcx, true);
    bitmap.set8BitMaskColor(maskColor);
    destroy_bitmap(pcx);
    pack_fclose(pack);

    return bitmap;
}

void Bitmap::initializeExtraStuff(){
    /* nothing yet */
}

void Bitmap::destroyPrivateData(){
    destroy_bitmap(getData().getBitmap());
}

int Bitmap::getWidth() const{
	return getData().getBitmap()->w;
}

int Bitmap::getHeight() const{
	return getData().getBitmap()->h;
}
	
int Bitmap::getRed( int x ){
	return ::getr( x );
}

int Bitmap::getBlue( int x ){
	return ::getb( x );
}

int Bitmap::getGreen( int x ){
	return ::getg( x );
}

void Bitmap::setClipRect( int x1, int y1, int x2, int y2 ) const {
    ::set_clip_rect( getData().getBitmap(), x1, y1, x2, y2 );
}

void Bitmap::getClipRect(int & x1, int & y1, int & x2, int & y2) const {
    ::get_clip_rect(getData().getBitmap(), &x1, &y1, &x2, &y2);
}

/*
const string & Bitmap::getPath() const{
	return path;
}
*/

void Bitmap::debugSelf() const{
	cout<<"Bitmap: "<<endl;
	cout<<"Self = "<< getData().getBitmap() <<endl;
	cout<<"Own = "<< own <<" : "<< *own <<endl;
	cout<<"Path = "<<path<<endl;
}
	
Bitmap & Bitmap::operator=( const Bitmap & copy ){
	
	/*
	if ( own ){
		(*own)--;
		if ( *own == 0 ){
			destroy_bitmap( getBitmap() );
			delete own;
		}
	}
	*/
	
	releaseInternalBitmap();

	path = copy.getPath();
	getData().setBitmap( copy.getData().getBitmap() );
	// own = false;
	own = copy.own;
	if ( own )
		*own += 1;

	return *this;
}

/*
Bitmap::~Bitmap(){
    releaseInternalBitmap();
}
*/
	
void Bitmap::detach(){

	/*
	if ( own ){
		(*own)--;

		if ( *own == 0 ){
			destroy_bitmap( getBitmap() );
			delete own;
		}
	}
	*/
	releaseInternalBitmap();
}

/*
BITMAP * Bitmap::getBitmap() const{
	return my_bitmap;
}
*/

bool Bitmap::getError(){
	return error;
}

/*
int Bitmap::getWidth(){
	return my_bitmap->w;
}

int Bitmap::getHeight(){
	return my_bitmap->h;
}
*/
	
void Bitmap::acquire(){
	acquire_bitmap( getData().getBitmap() );
}

void Bitmap::release(){
	release_bitmap( getData().getBitmap() );
}

void Bitmap::circleFill( int x, int y, int radius, int color ) const {
	::circlefill( getData().getBitmap(), x, y, radius, color );
}

void TranslucentBitmap::circleFill(int x, int y, int radius, int color) const {
    drawingMode(MODE_TRANS);
    Bitmap::circleFill(x, y, radius, color);
    drawingMode(MODE_SOLID);
}
	
void Bitmap::circle( int x, int y, int radius, int color ) const{
	::circle( getData().getBitmap(), x, y, radius, color );
}
	
void Bitmap::line( const int x1, const int y1, const int x2, const int y2, const int color ) const{

	::fastline( getData().getBitmap(), x1, y1, x2, y2, color );
	
}
	
void Bitmap::floodfill( const int x, const int y, const int color ) const {
	::floodfill( getData().getBitmap(), x, y, color );
}
	
void Bitmap::drawCharacter( const int x, const int y, const int color, const int background, const Bitmap & where ) const {
	::draw_character_ex( where.getData().getBitmap(), getData().getBitmap(), x, y, color, background );
}

void Bitmap::transBlender( int r, int g, int b, int a ){
	set_trans_blender( r, g, b, a );
}
	
void Bitmap::multiplyBlender( int r, int g, int b, int a ){
	set_multiply_blender( r, g, b, a );
}
	
void Bitmap::dissolveBlender( int r, int g, int b, int a ){
	set_dissolve_blender( r, g, b, a );
}

void Bitmap::addBlender( int r, int g, int b, int a ){
	set_add_blender( r, g, b, a );
}

void Bitmap::burnBlender( int r, int g, int b, int a ){
	set_burn_blender( r, g, b, a );
}

void Bitmap::colorBlender( int r, int g, int b, int a ){
	set_color_blender( r, g, b, a );
}

void Bitmap::differenceBlender( int r, int g, int b, int a ){
	set_difference_blender( r, g, b, a );
}

void Bitmap::dodgeBlender( int r, int g, int b, int a ){
	set_dodge_blender( r, g, b, a );
}

void Bitmap::hueBlender( int r, int g, int b, int a ){
	set_hue_blender( r, g, b, a );
}

void Bitmap::luminanceBlender( int r, int g, int b, int a ){
	set_luminance_blender( r, g, b, a );
}

void Bitmap::invertBlender( int r, int g, int b, int a ){
	set_invert_blender( r, g, b, a );
}

void Bitmap::screenBlender( int r, int g, int b, int a ){
	set_screen_blender( r, g, b, a );
}

void Bitmap::replaceColor(int original, int replaced){
    int height = getHeight();
    int width = getWidth();
    BITMAP * bitmap = getData().getBitmap();
    for (int i = 0; i < height; ++i ){
        for( int j = 0; j < width; ++j ){
            /* use getPixel/putPixel? */
            int pix = getpixel(bitmap, j,i);
            if (pix == original){
                putpixel(bitmap, j, i, replaced);
            }
        }
    }
}

void Bitmap::drawingMode( int mode ){
	// drawing_mode( DRAW_MODE_TRANS, NULL, 0, 0 );
	switch( mode ){
		case MODE_TRANS : {
			drawing_mode( DRAW_MODE_TRANS, NULL, 0, 0 );
			break;
		}
		case MODE_SOLID : {
			drawing_mode( DRAW_MODE_SOLID, NULL, 0, 0 );
			break;
		}
	}
}

int Bitmap::setGraphicsMode( int mode, int width, int height ){
    int ok = ::set_gfx_mode(mode, width, height, 0, 0);
    if ( ok == 0 ){
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
            Screen = new Bitmap( ::screen );
            Scaler = new Bitmap(width, height);
            /*
            if ( width != 0 && height != 0 && (width != SCALE_X || height != SCALE_Y) ){
                Scaler = new Bitmap(width, height);
                // Buffer = new Bitmap(SCALE_X, SCALE_Y);
            }
            */
        }
    }
    return ok;
}

void Bitmap::shutdown(){
    delete Screen;
    Screen = NULL;
    delete Scaler;
    Scaler = NULL;
    delete Buffer;
    Buffer = NULL;
}


/*
const int Bitmap::getWidth() const{
	return getBitmap()->w;
}

const int Bitmap::getHeight() const{
	return getBitmap()->h;
}
*/

int Bitmap::getPixel( const int x, const int y ) const{
	if ( x >= 0 && x < getData().getBitmap()->w && y >= 0 && y < getData().getBitmap()->h )
		return _getpixel16( getData().getBitmap(), x, y );
	return -1;
}

void Bitmap::readLine( vector< int > & vec, int y ){
	if ( y >= 0 && y < getData().getBitmap()->h ){
		for ( int q = 0; q < getData().getBitmap()->w; q++ ){
			// int col = my_bitmap->line[ y ][ q ];
			int col = _getpixel16( getData().getBitmap(), q, y );
			vec.push_back( col );
		}
	}
}

int Bitmap::setGfxModeText(){
	return setGraphicsMode(GFX_TEXT, 0, 0);
}
	
int Bitmap::setGfxModeFullscreen(int x, int y){
	return setGraphicsMode(GFX_AUTODETECT_FULLSCREEN, x, y);
}

int Bitmap::setGfxModeWindowed(int x, int y){
	return setGraphicsMode(GFX_AUTODETECT_WINDOWED, x, y);
}
	
int Bitmap::makeColor( int r, int g, int b ){
	return ::makecol16( r, g, b );
}
	
int Bitmap::darken( int color, double factor ){
	int r = (int)((double)::getr16( color ) / factor);
	int g = (int)((double)::getg16( color ) / factor);
	int b = (int)((double)::getb16( color ) / factor);

	return makeColor( r, g, b );
}
	
void Bitmap::hsvToRGB( float h, float s, float v, int * r, int * g, int * b ){
	::hsv_to_rgb( h, s, v, r, g, b );
}
        
void Bitmap::rgbToHSV(int r, int g, int b, float * h, float * s, float * v){
    ::rgb_to_hsv(r, g, b, h, s, v);
}


int Bitmap::addColor( int color1, int color2 ){
	return makeColor( getr( color1 ) + getr( color2 ),
			  getg( color1 ) + getg( color2 ),
			  getb( color1 ) + getb( color2 ) );
}
	
void Bitmap::putPixelNormal(int x, int y, int col) const {
    BITMAP * dst = getData().getBitmap();
    ::putpixel(dst, x, y, col);
}

/* FIXME: its pretty slow to keep setting the drawing mode. Either
 * 1) dont use translucent putpixel that much or
 * 2) somehow cache the drawing mode so that its not reset every time
 */
void TranslucentBitmap::putPixelNormal(int x, int y, int color) const {
    drawingMode(MODE_TRANS);
    Bitmap::putPixelNormal(x, y, color);
    drawingMode(MODE_SOLID);
}
	
void Bitmap::putPixel( int x, int y, int col ) const{
    BITMAP * dst = getData().getBitmap();
    if (dst->clip && ((x < dst->cl) || (x >= dst->cr) || (y < dst->ct) || (y >= dst->cb))){
        return;
    }
    if ( x >= 0 && x < getData().getBitmap()->w && y >= 0 && y < getData().getBitmap()->h )
        _putpixel16( getData().getBitmap(), x, y, col );
}
	
/*
void Bitmap::printf( int x, int y, int color, FONT * f, const char * str, ... ) const{

	char buf[512];
	va_list ap;

	va_start(ap, str);
	uvszprintf(buf, sizeof(buf), str, ap);
	va_end(ap);

	textout_ex( getBitmap(), f, buf, x, y, color, -1);
}

void Bitmap::printf( int x, int y, int color, const Font * const f, const char * str, ... ) const{

	char buf[512];
	va_list ap;

	va_start(ap, str);
	uvszprintf(buf, sizeof(buf), str, ap);
	va_end(ap);

	textout_ex( getBitmap(), f->getInternalFont(), buf, x, y, color, -1);
}
	
void Bitmap::printf( int x, int y, int color, const Font * const f, const string & str ) const{
	printf( x, y, color, f, str.c_str() );
}

void Bitmap::printf( int x, int y, int color, const Font & f, const string & str ) const{
	printf( x, y, color, &f, str );
}
	
void Bitmap::printfNormal( int x, int y, int color, const char * str, ... ) const{

	char buf[512];
	va_list ap;

	va_start(ap, str);
	uvszprintf(buf, sizeof(buf), str, ap);
	va_end(ap);

	textout_ex( getBitmap(), font, buf, x, y, color, -1);

}

void Bitmap::printfNormal( int x, int y, int color, const string & str ) const{
	printfNormal( x, y, color, "%s", str.c_str() );
}
*/

void Bitmap::triangle( int x1, int y1, int x2, int y2, int x3, int y3, int color ) const{
	::triangle( getData().getBitmap(), x1, y1, x2, y2, x3, y3, color );
}
	
void Bitmap::ellipse( int x, int y, int rx, int ry, int color ) const {
	::ellipse( getData().getBitmap(), x, y, rx, ry, color );
}

void TranslucentBitmap::ellipse(int x, int y, int rx, int ry, int color) const {
    drawingMode(MODE_TRANS);
    Bitmap::ellipse(x, y, rx, ry, color);
    drawingMode(MODE_SOLID);
}

void Bitmap::ellipseFill( int x, int y, int rx, int ry, int color ) const {
	::ellipsefill( getData().getBitmap(), x, y, rx, ry, color );
}

void Bitmap::rectangle( int x1, int y1, int x2, int y2, int color ) const{
    ::rect( getData().getBitmap(), x1, y1, x2, y2, color );
}

void TranslucentBitmap::rectangle( int x1, int y1, int x2, int y2, int color ) const {
    drawingMode(MODE_TRANS);
    Bitmap::rectangle(x1, y1, x2, y2, color);
    drawingMode(MODE_SOLID);
}
	
void Bitmap::rectangleFill( int x1, int y1, int x2, int y2, int color ) const{
    ::rectfill( getData().getBitmap(), x1, y1, x2, y2, color );
}

void TranslucentBitmap::rectangleFill( int x1, int y1, int x2, int y2, int color ) const{
    drawingMode(MODE_TRANS);
    Bitmap::rectangleFill(x1, y1, x2, y2, color);
    drawingMode(MODE_SOLID);
}

/*
int Bitmap::getPixel( int x, int y ){
	if ( x >= 0 && x < my_bitmap->w && y >= 0 && y <= my_bitmap->h )
		return _getpixel16( my_bitmap, x, y );
	return -1;
}
*/

/*
int Bitmap::makeColor( int r, int g, int b ){
	return makecol16( r, g, b );
}
*/
	
void Bitmap::hLine( const int x1, const int y, const int x2, const int color ) const{
    ::hline( getData().getBitmap(), x1, y, x2, color );
}

void TranslucentBitmap::hLine( const int x1, const int y, const int x2, const int color ) const{
    drawingMode(MODE_TRANS);
    ::hline(getData().getBitmap(), x1, y, x2, color);
    drawingMode(MODE_SOLID);
}
	
void Bitmap::vLine( const int y1, const int x, const int y2, const int color ) const{
	::vline( getData().getBitmap(), x, y1, y2, color );
}
	
void Bitmap::polygon( const int * verts, const int nverts, const int color ) const{
	::polygon( getData().getBitmap(), nverts, verts, color );
}

void Bitmap::arc(const int x, const int y, const double ang1, const double ang2, const int radius, const int color ) const{
	::arc( getData().getBitmap(), x, y, ::ftofix(ang1), ::ftofix(ang2), radius, color );
}

/*
void Bitmap::clear(){
	this->fill( 0 );
}
*/

void Bitmap::fill( int color ) const{
	::clear_to_color( getData().getBitmap(), color );
}

/*
void TranslucentBitmap::fill(int color) const {
    rectangleFill(0, 0, getWidth(), getHeight(), color);
}
*/
	
void Bitmap::draw( const int x, const int y, const Bitmap & where ) const {
	paintown_draw_sprite_ex16( where.getData().getBitmap(), getData().getBitmap(), x, y, Bitmap::SPRITE_NORMAL, Bitmap::SPRITE_NO_FLIP );
	// ::draw_sprite( where.getData().getBitmap(), getBitmap(), x, y );
}
	
void Bitmap::draw(const int x, const int y, const Filter & filter, const Bitmap & where) const {
    paintown_draw_sprite_filter_ex16(where.getData().getBitmap(), getData().getBitmap(), x, y, filter);
}

void Bitmap::drawHFlip( const int x, const int y, const Bitmap & where ) const {
	paintown_draw_sprite_ex16( where.getData().getBitmap(), getData().getBitmap(), x, y, Bitmap::SPRITE_NORMAL, Bitmap::SPRITE_H_FLIP );
	// ::draw_sprite_h_flip( where.getBitmap(), getBitmap(), x, y );
}

void Bitmap::drawVFlip( const int x, const int y, const Bitmap & where ) const {
	paintown_draw_sprite_ex16( where.getData().getBitmap(), getData().getBitmap(), x, y, Bitmap::SPRITE_NORMAL, Bitmap::SPRITE_V_FLIP );
	// ::draw_sprite_h_flip( where.getBitmap(), getBitmap(), x, y );
}

void Bitmap::drawHVFlip( const int x, const int y, const Bitmap & where ) const {
	paintown_draw_sprite_ex16( where.getData().getBitmap(), getData().getBitmap(), x, y, Bitmap::SPRITE_NORMAL, Bitmap::SPRITE_V_FLIP | Bitmap::SPRITE_H_FLIP );
}

/*
void Bitmap::drawLit( const int x, const int y, const int level, const Bitmap & where ) const{
	::draw_lit_sprite( where.getBitmap(), getBitmap(), x, y, level );
}
*/

void Bitmap::drawTrans( const int x, const int y, const Bitmap & where ) const{
	// ::draw_trans_sprite( where.getBitmap(), getBitmap(), x, y );
	paintown_draw_sprite_ex16( where.getData().getBitmap(), getData().getBitmap(), x, y, Bitmap::SPRITE_TRANS, Bitmap::SPRITE_NO_FLIP );
}
	
void Bitmap::drawTransHFlip( const int x, const int y, const Bitmap & where ) const {
	paintown_draw_sprite_ex16( where.getData().getBitmap(), getData().getBitmap(), x, y, Bitmap::SPRITE_TRANS, Bitmap::SPRITE_H_FLIP );
}

void Bitmap::drawTransVFlip( const int x, const int y, const Bitmap & where ) const {
	paintown_draw_sprite_ex16( where.getData().getBitmap(), getData().getBitmap(), x, y, Bitmap::SPRITE_TRANS, Bitmap::SPRITE_V_FLIP );
}
	
void Bitmap::drawTransHVFlip( const int x, const int y, const Bitmap & where ) const {
	paintown_draw_sprite_ex16( where.getData().getBitmap(), getData().getBitmap(), x, y, Bitmap::SPRITE_TRANS, Bitmap::SPRITE_V_FLIP | Bitmap::SPRITE_H_FLIP );
}

void Bitmap::drawRotate( const int x, const int y, const int angle, const Bitmap & where ){
	::fixed fang = itofix( (360 - angle) % 360 * 256 / 360 );
	::rotate_sprite( where.getData().getBitmap(), getData().getBitmap(), x, y, fang ); 
}
	
void Bitmap::drawPivot( const int centerX, const int centerY, const int x, const int y, const int angle, const Bitmap & where ){
    ::fixed fang = ftofix( (double)((360 - angle) % 360) * 256.0 / 360.0 );
    ::pivot_sprite( where.getData().getBitmap(), getData().getBitmap(), x, y, centerX, centerY, fang ); 
}
	
void Bitmap::drawPivot( const int centerX, const int centerY, const int x, const int y, const int angle, const double scale, const Bitmap & where ){
    ::fixed fscale = ftofix(scale);
    ::fixed fang = ftofix( (double)((360 - angle) % 360) * 256.0 / 360.0 );
    ::pivot_scaled_sprite( where.getData().getBitmap(), getData().getBitmap(), x, y, centerX, centerY, fang, fscale ); 
}

void Bitmap::drawStretched( const int x, const int y, const int new_width, const int new_height, const Bitmap & who ){
	BITMAP * bmp = who.getData().getBitmap();
	::masked_stretch_blit( getData().getBitmap(), bmp, 0, 0, getData().getBitmap()->w, getData().getBitmap()->h, x,y, new_width, new_height );
}

void Bitmap::light(int x, int y, int width, int height, int start_y, int focus_alpha, int edge_alpha, int focus_color, int edge_color) const {
    paintown_light16(getData().getBitmap(), x, y, width, height, start_y, focus_alpha, edge_alpha, focus_color, edge_color);
}
        
void Bitmap::applyTrans(const int color) const {
    paintown_applyTrans16(getData().getBitmap(), color);
}

void Bitmap::StretchBy2( const Bitmap & where ){
	BITMAP * bmp = where.getData().getBitmap();

	if ( where.getWidth() == getWidth() && where.getHeight() == getHeight() ){
		::blit( getData().getBitmap(), bmp, 0, 0, 0, 0, getData().getBitmap()->w, getData().getBitmap()->h );
		return;
	}

	if ( where.getWidth() != getWidth()*2 ||
		where.getHeight() != getHeight()*2 ){
			cout<<"Wrong dimensions"<<endl;
			cout<<"My:  "<< getWidth() << " " << getHeight() << endl;
			cout<<"Him: "<<where.getWidth()<< " " << where.getHeight()<<endl;
			return;
	}
	// debug
	::stretch_blit( getData().getBitmap(), bmp, 0, 0, getData().getBitmap()->w, getData().getBitmap()->h, 0, 0, bmp->w, bmp->h );
	// fblend_2x_stretch( my_bitmap, bmp, 0, 0, 0, 0, my_bitmap->w, my_bitmap->h);
	// scale2x_allegro( my_bitmap, bmp, 2 );
	// debug

}

void Bitmap::StretchBy4( const Bitmap & where ){

	BITMAP * bmp = where.getData().getBitmap();
	if ( where.getWidth() != getWidth()*4 ||
		where.getHeight() != getHeight()*4 ){
			cout<<"Wrong dimensions"<<endl;
			cout<<"My:  "<< getWidth() << " " << getHeight() << endl;
			cout<<"Him: "<<where.getWidth()<< " " << where.getHeight()<<endl;
			cout<<"Scaled: "<<getWidth()*4<<" "<< getHeight()*4<<endl;
			return;
	}
	// fblend_2x_stretch( my_bitmap, bmp, 0, 0, 0, 0, my_bitmap->w, my_bitmap->h);
	// scale4x_allegro( my_bitmap, bmp, 2 );
	::stretch_blit( getData().getBitmap(), bmp, 0, 0, getData().getBitmap()->w, getData().getBitmap()->h, 0, 0, bmp->w, bmp->h );

}

void Bitmap::Stretch( const Bitmap & where ) const {
	Stretch( where, 0, 0, getData().getBitmap()->w, getData().getBitmap()->h, 0, 0, where.getData().getBitmap()->w, where.getData().getBitmap()->h );
	/*
	BITMAP * bmp = where.getBitmap();
	::stretch_blit( getBitmap(), bmp, 0, 0, getBitmap()->w, getBitmap()->h, 0, 0, bmp->w, bmp->h );
	*/
}
	
void Bitmap::Stretch( const Bitmap & where, const int sourceX, const int sourceY, const int sourceWidth, const int sourceHeight, const int destX, const int destY, const int destWidth, const int destHeight ) const {
	BITMAP * bmp = where.getData().getBitmap();
	::stretch_blit( getData().getBitmap(), bmp, sourceX, sourceY, sourceWidth, sourceHeight, destX, destY, destWidth, destHeight );
}

void Bitmap::Blit( const int x, const int y, const Bitmap & where ) const {
	BITMAP * bmp = where.getData().getBitmap();
	/*
	acquire_bitmap( bmp );
	acquire_bitmap( my_bitmap );
	*/
	::blit( getData().getBitmap(), bmp, 0, 0, x, y, getData().getBitmap()->w, getData().getBitmap()->h );
	/*
	release_bitmap( my_bitmap );
	release_bitmap( bmp );
	*/
}

void Bitmap::Blit( const int mx, const int my, const int wx, const int wy, const Bitmap & where ) const {
	BITMAP * bmp = where.getData().getBitmap();
	::blit( getData().getBitmap(), bmp, mx, my, wx, wy, getData().getBitmap()->w, getData().getBitmap()->h );
}

void Bitmap::Blit( const int mx, const int my, const int width, const int height, const int wx, const int wy, const Bitmap & where ) const {
	BITMAP * bmp = where.getData().getBitmap();
	::blit( getData().getBitmap(), bmp, mx, my, wx, wy, width, height );
}

void Bitmap::BlitMasked( const int mx, const int my, const int width, const int height, const int wx, const int wy, const Bitmap & where ) const {
	BITMAP * bmp = where.getData().getBitmap();
	::masked_blit( getData().getBitmap(), bmp, mx, my, wx, wy, width, height );
}

void Bitmap::Blit( const Bitmap & where ) const {
    this->Blit( 0, 0, where );
}

void Bitmap::BlitToScreen(const int upper_left_x, const int upper_left_y) const {
    if ( Scaler == NULL ){
        this->Blit( upper_left_x, upper_left_y, *Bitmap::Screen );
    } else {
        if (upper_left_x != 0 || upper_left_y != 0){
            Bitmap buffer = temporaryBitmap(getWidth(), getHeight());
            buffer.clear();
            this->Blit(upper_left_x, upper_left_y, buffer);
            buffer.Stretch(*Scaler);
        } else {
            this->Stretch(*Scaler);
        }
        Scaler->Blit( 0, 0, 0, 0, *Screen );
    }
}

void Bitmap::BlitAreaToScreen(const int upper_left_x, const int upper_left_y) const {

    if ( Scaler != NULL ){
        double mult_x = (double) Scaler->getWidth() / (double) SCALE_X;
        double mult_y = (double) Scaler->getHeight() / (double) SCALE_Y;

        int x = (int)(upper_left_x * mult_x);
        int y = (int)(upper_left_y * mult_y);
        int w = (int)(this->getWidth() * mult_x);
        int h = (int)(this->getHeight() * mult_y);

        // printf("ux %d uy %d uw %d uh %d. x %d y %d w %d h %d\n", upper_left_x, upper_left_y, getWidth(), getHeight(), x, y, w, h );

        this->Stretch( *Scaler, 0, 0, this->getWidth(), this->getHeight(), x, y, w, h );

        Bitmap tmp(*Scaler, x, y, w, h );
        tmp.Blit( x, y, *Screen );

        // Scaler->Blit( x, y, w, h, *Screen );
    } else {
        this->Blit( upper_left_x, upper_left_y, *Screen );
    }
}

void LitBitmap::draw( const int x, const int y, const Bitmap & where ) const {
	// ::draw_sprite_ex( where.getBitmap(), getBitmap(), x, y, SPRITE_LIT );
	// ::draw_sprite( where.getBitmap(), getBitmap(), x, y );
	// Bitmap::draw( x, y, where );
	paintown_draw_sprite_ex16( where.getData().getBitmap(), getData().getBitmap(), x, y, Bitmap::SPRITE_LIT, Bitmap::SPRITE_NO_FLIP );
}
	
void LitBitmap::drawHFlip( const int x, const int y, const Bitmap & where ) const {
	paintown_draw_sprite_ex16(where.getData().getBitmap(), getData().getBitmap(), x, y, Bitmap::SPRITE_LIT, Bitmap::SPRITE_H_FLIP);
}

void LitBitmap::drawVFlip( const int x, const int y, const Bitmap & where ) const {
	paintown_draw_sprite_ex16(where.getData().getBitmap(), getData().getBitmap(), x, y, Bitmap::SPRITE_LIT, Bitmap::SPRITE_V_FLIP);
}

void LitBitmap::drawHVFlip( const int x, const int y, const Bitmap & where ) const {
	paintown_draw_sprite_ex16(where.getData().getBitmap(), getData().getBitmap(), x, y, Bitmap::SPRITE_LIT, Bitmap::SPRITE_V_FLIP | Bitmap::SPRITE_H_FLIP);
}

/* this function should be in allegro but its not yet so just store it
 * here for now
 */
#define PAINTOWN_DLS_BLENDER BLENDER_FUNC
#define PAINTOWN_DTS_BLENDER BLENDER_FUNC
#define PAINTOWN_MAKE_DLS_BLENDER(a) _blender_func16
#define PAINTOWN_MAKE_DTS_BLENDER() _blender_func16
#define PAINTOWN_PIXEL_PTR unsigned short*
#define PAINTOWN_OFFSET_PIXEL_PTR(p,x) ((PAINTOWN_PIXEL_PTR) (p) + (x))
#define PAINTOWN_INC_PIXEL_PTR(p) ((p)++)
#define PAINTOWN_INC_PIXEL_PTR_EX(p,d)  ((p) += d)
#define PAINTOWN_GET_MEMORY_PIXEL(p)    (*(p))
#define PAINTOWN_IS_SPRITE_MASK(b,c)    ((unsigned long) (c) == (unsigned long) (b)->vtable->mask_color)
#define PAINTOWN_DLSX_BLEND(b,n)       ((*(b))(_blender_col_16, (n), _blender_alpha))
#define PAINTOWN_GET_PIXEL(p)           bmp_read16((uintptr_t) (p))
#define PAINTOWN_DTS_BLEND(b,o,n)       ((*(b))((n), (o), _blender_alpha))
#define PAINTOWN_PUT_PIXEL(p,c)         bmp_write16((uintptr_t) (p), (c))
#define PAINTOWN_PUT_MEMORY_PIXEL(p,c)  (*(p) = (c))
#define PAINTOWN_SET_ALPHA(a)           (_blender_alpha = (a))

/* defined at allegro/include/internal/aintern.h:457 */
extern EXTERNAL_VARIABLE BLENDER_FUNC _blender_func16;
/* defined at allegro/include/internal/aintern.h:466 */
extern EXTERNAL_VARIABLE int _blender_col_16;
/* defined at allegro/include/internal/aintern.h:470 */
extern EXTERNAL_VARIABLE int _blender_alpha;

static void paintown_draw_sprite_ex16( BITMAP * dst, BITMAP * src, int dx, int dy, int mode, int flip ){
    int x, y, w, h;
    int x_dir = 1, y_dir = 1;
    int dxbeg, dybeg;
    int sxbeg, sybeg;
    PAINTOWN_DLS_BLENDER lit_blender;
    PAINTOWN_DTS_BLENDER trans_blender;

    ASSERT(dst);
    ASSERT(src);

    if ( flip & Bitmap::SPRITE_V_FLIP ){
        y_dir = -1;
    }
    if ( flip & Bitmap::SPRITE_H_FLIP ){
        x_dir = -1;
    }

    if (dst->clip) {
        int tmp;

        tmp = dst->cl - dx;
        sxbeg = ((tmp < 0) ? 0 : tmp);
        dxbeg = sxbeg + dx;

        tmp = dst->cr - dx;
        w = ((tmp > src->w) ? src->w : tmp) - sxbeg;
        if (w <= 0)
            return;

        if ( flip & Bitmap::SPRITE_H_FLIP ){
            /* use backward drawing onto dst */
            sxbeg = src->w - (sxbeg + w);
            dxbeg += w - 1;
        }

        tmp = dst->ct - dy;
        sybeg = ((tmp < 0) ? 0 : tmp);
        dybeg = sybeg + dy;

        tmp = dst->cb - dy;
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

    lit_blender = PAINTOWN_MAKE_DLS_BLENDER(0);
    trans_blender = PAINTOWN_MAKE_DTS_BLENDER();

    if (dst->id & (BMP_ID_VIDEO | BMP_ID_SYSTEM)) {
        bmp_select(dst);

#if 0
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
#endif

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

        switch (mode){
            case Bitmap::SPRITE_NORMAL : {
                                             for (y = 0; y < h; y++) {
                                                 PAINTOWN_PIXEL_PTR s = PAINTOWN_OFFSET_PIXEL_PTR(src->line[sybeg + y], sxbeg);
                                                 PAINTOWN_PIXEL_PTR d = PAINTOWN_OFFSET_PIXEL_PTR(bmp_write_line(dst, dybeg + y * y_dir), dxbeg);

                                                 for (x = w - 1; x >= 0; PAINTOWN_INC_PIXEL_PTR(s), PAINTOWN_INC_PIXEL_PTR_EX(d,x_dir), x--) {
                                                     unsigned long c = PAINTOWN_GET_MEMORY_PIXEL(s);
                                                     if (!PAINTOWN_IS_SPRITE_MASK(src, c)) {
                                                         PAINTOWN_PUT_MEMORY_PIXEL(d, c);
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
                                                      PAINTOWN_PUT_MEMORY_PIXEL(d, c);
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
                                                        PAINTOWN_PUT_MEMORY_PIXEL(d, c);
                                                    }
                                                }
                                            }
                                            break;
                                        }
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
}

/* mix pixels with the given color in with each non-masking pixel in dst */
static void paintown_applyTrans16(BITMAP * dst, const int color){
    int y1 = 0;
    int y2 = dst->h;
    int x1 = 0;
    int x2 = dst->w - 1;
    
    if (dst->clip){
        y1 = dst->ct;
        y2 = dst->cb - 1;
        x1 = dst->cl;
        x2 = dst->cr - 1;
    }

    PAINTOWN_DTS_BLENDER trans_blender;
    trans_blender = PAINTOWN_MAKE_DTS_BLENDER();
    if (dst->id & (BMP_ID_VIDEO | BMP_ID_SYSTEM)) {
    } else {
        for (int y = y1; y < y2; y++) {
            PAINTOWN_PIXEL_PTR d = PAINTOWN_OFFSET_PIXEL_PTR(bmp_write_line(dst, y), x1);
            for (int x = x2; x >= x1; PAINTOWN_INC_PIXEL_PTR_EX(d,1), x--) {
                unsigned long c = PAINTOWN_GET_MEMORY_PIXEL(d);
                if (!PAINTOWN_IS_SPRITE_MASK(dst, c)) {
                    c = PAINTOWN_DTS_BLEND(trans_blender, c, color);
                    PAINTOWN_PUT_MEMORY_PIXEL(d, c);
                }
            }
        }
    }
}

/* ultra special-case for drawing a light (like from a lamp).
 * center of light is x,y and shines in a perfect isosolese triangle.
 */
static void paintown_light16(BITMAP * dst, const int x, const int y, const int width, const int height, const int start_y, const int focus_alpha, const int edge_alpha, const int focus_color, const int edge_color){

    int dxbeg = x - width;
    int x_dir = 1;
    unsigned char * alphas = new unsigned char[width];
    int * colors = new int[width];
    for (int i = 0; i < width; i++){
        alphas[i] = (unsigned char)((double)(edge_alpha - focus_alpha) * (double)i / (double)width + focus_alpha);
    }
    Util::blend_palette(colors, width, focus_color, edge_color);

    int min_y, max_y, min_x, max_x;
    if (dst->clip){
        min_y = dst->ct;
        max_y = dst->cb - 1;
        min_x = dst->cl;
        max_x = dst->cr - 1;
    } else {
        min_y = y < 0 ? 0 : y;
        max_y = dst->h - 1;
        min_x = (x-width) < 0 ? 0 : x-width;
        max_x = (x+width) > dst->w-1 ? dst->w-1 : x+width;
    }
            
    int dybeg = y;

    /* tan(theta) = y / x */
    double xtan = (double) height / (double) width;
    PAINTOWN_DTS_BLENDER trans_blender;
    trans_blender = PAINTOWN_MAKE_DTS_BLENDER();
    if (dst->id & (BMP_ID_VIDEO | BMP_ID_SYSTEM)) {
    } else {
        for (int sy = start_y; sy < height; sy++) {
            if (dybeg + sy < min_y || dybeg + sy > max_y){
                continue;
            }
            /* x = y / tan(theta) */
            int top_width = (int)((double) sy / xtan);
            if (top_width == 0){
                continue;
            }
            dxbeg = x - top_width;
            PAINTOWN_PIXEL_PTR d = PAINTOWN_OFFSET_PIXEL_PTR(bmp_write_line(dst, dybeg + sy), dxbeg);

            for (int sx = -top_width; sx <= top_width; PAINTOWN_INC_PIXEL_PTR_EX(d,x_dir), sx++) {
                if (sx + x < min_x || sx + x > max_x){
                    continue;
                }
                unsigned long c = PAINTOWN_GET_MEMORY_PIXEL(d);
                /* TODO:
                 * converting to a double and calling fabs is overkill, just
                 * write an integer abs() function.
                 */
                int sx_abs = (int) fabs((double) sx);
                PAINTOWN_SET_ALPHA(alphas[sx_abs]);
                int color = colors[sx_abs];
                c = PAINTOWN_DTS_BLEND(trans_blender, c, color);
                PAINTOWN_PUT_MEMORY_PIXEL(d, c);
            }
        }
    }
    delete[] alphas;
    delete[] colors;
}

/* there is a bug in this function that causes memory corruption. if that bug
 * is ever found then use this method, otherwise don't use it.
 */
static void paintown_draw_sprite_ex16_old( BITMAP * dst, BITMAP * src, int dx, int dy, int mode, int flip ){
    int x, y, w, h;
    int x_dir = 1, y_dir = 1;
    int dxbeg, dybeg;
    int sxbeg, sybeg;
    PAINTOWN_DLS_BLENDER lit_blender;
    PAINTOWN_DTS_BLENDER trans_blender;

    ASSERT(dst);
    ASSERT(src);

    if ( flip & Bitmap::SPRITE_V_FLIP ){
        y_dir = -1;
    }
    if ( flip & Bitmap::SPRITE_H_FLIP ){
        x_dir = -1;
    }

    if (dst->clip) {
        int tmp;

        tmp = dst->cl - dx;
        sxbeg = ((tmp < 0) ? 0 : tmp);
        dxbeg = sxbeg + dx;

        tmp = dst->cr - dx;
        w = ((tmp > src->w) ? src->w : tmp) - sxbeg;
        if (w <= 0)
            return;

        if ( flip & Bitmap::SPRITE_H_FLIP ){
            /* use backward drawing onto dst */
            sxbeg = src->w - (sxbeg + w);
            dxbeg += w - 1;
        }

        tmp = dst->ct - dy;
        sybeg = ((tmp < 0) ? 0 : tmp);
        dybeg = sybeg + dy;

        tmp = dst->cb - dy;
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

    lit_blender = PAINTOWN_MAKE_DLS_BLENDER(0);
    trans_blender = PAINTOWN_MAKE_DTS_BLENDER();

    if (dst->id & (BMP_ID_VIDEO | BMP_ID_SYSTEM)) {
        bmp_select(dst);

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
    }
}

static void paintown_draw_sprite_filter_ex16(BITMAP * dst, BITMAP * src, int dx, int dy, const Bitmap::Filter & filter){
    int x, y, w, h;
    int x_dir = 1, y_dir = 1;
    int dxbeg, dybeg;
    int sxbeg, sybeg;

    if (true /* dst->clip*/ ) {
        int tmp;

        tmp = dst->cl - dx;
        sxbeg = ((tmp < 0) ? 0 : tmp);
        dxbeg = sxbeg + dx;

        tmp = dst->cr - dx;
        w = ((tmp > src->w) ? src->w : tmp) - sxbeg;
        if (w <= 0)
            return;

        tmp = dst->ct - dy;
        sybeg = ((tmp < 0) ? 0 : tmp);
        dybeg = sybeg + dy;

        tmp = dst->cb - dy;
        h = ((tmp > src->h) ? src->h : tmp) - sybeg;
        if (h <= 0)
            return;
    }

    unsigned int mask = Bitmap::makeColor(255, 0, 255);
    // int bpp = src->format->BytesPerPixel;
    for (y = 0; y < h; y++) {
        PAINTOWN_PIXEL_PTR s = PAINTOWN_OFFSET_PIXEL_PTR(src->line[sybeg + y], sxbeg);
        PAINTOWN_PIXEL_PTR d = PAINTOWN_OFFSET_PIXEL_PTR(bmp_write_line(dst, dybeg + y * y_dir), dxbeg);

        for (x = w - 1; x >= 0; PAINTOWN_INC_PIXEL_PTR(s), PAINTOWN_INC_PIXEL_PTR_EX(d,x_dir), x--) {
            unsigned long c = PAINTOWN_GET_MEMORY_PIXEL(s);
            if (!PAINTOWN_IS_SPRITE_MASK(src, c)) {
                PAINTOWN_PUT_MEMORY_PIXEL(d, filter.filter(c));
            } else {
                PAINTOWN_PUT_MEMORY_PIXEL(d, mask);
            }
        }
    }
}
