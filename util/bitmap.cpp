#include <allegro.h>
#include "bitmap.h"
#include "lit_bitmap.h"
#include <stdarg.h>
#include <vector>
#include <string>
#include <iostream>
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

int Bitmap::SCALE_X;
int Bitmap::SCALE_Y;

static void paintown_draw_sprite_ex16( BITMAP * dst, BITMAP * src, int dx, int dy, int mode, int flip );

const int Bitmap::MaskColor = MASK_COLOR_16;

const int Bitmap::MODE_TRANS = 0;
const int Bitmap::MODE_SOLID = 1;
	
const int Bitmap::SPRITE_NO_FLIP = 0;
const int Bitmap::SPRITE_V_FLIP = 1;
const int Bitmap::SPRITE_H_FLIP = 2;
	
const int Bitmap::SPRITE_NORMAL = 1;
const int Bitmap::SPRITE_LIT = 2;
const int Bitmap::SPRITE_TRANS = 3;

Bitmap * Bitmap::Screen = NULL;
Bitmap * Scaler = NULL;
Bitmap * Buffer = NULL;

Bitmap::Bitmap():
own( NULL ),
error( false ){
	setBitmap( create_bitmap( 10, 10 ) );
	if ( ! getBitmap() ){
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
error( false ){
	if ( x < 1 ){
		x = 1;
	}
	if ( y < 1 ){
		y = 1;
	}
	setBitmap( create_bitmap( x, y ) );
	if ( ! getBitmap() ){
		error = true;
		cerr << "Could not create bitmap!" << endl;
	} else {
		clear();
		own = new int;
		*own = 1;
	}
}

/* If a BITMAP is given to us, we didnt make it so we dont own it */
Bitmap::Bitmap( BITMAP * who, bool deep_copy ):
own( NULL ),
error( false ){
	
	if ( deep_copy ){
		BITMAP * his = who;
		setBitmap( create_bitmap( his->w, his->h ) );
		if ( ! getBitmap() ){
			cout << "Could not create bitmap" << endl;
			error = true;
		}
		::blit( his, getBitmap(), 0, 0, 0, 0, his->w, his->h );
		own = new int;
		*own = 1;
	} else {
		setBitmap( who );
		own = NULL;
	}
}
	
Bitmap::Bitmap( const char * load_file ):
own( NULL ),
error( false ){
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
error( false ){
	internalLoadFile( load_file.c_str() );
}

Bitmap::Bitmap( const char * load_file, int sx, int sy ):
own( NULL ),
error( false ){
	path = load_file;
	BITMAP * temp = load_bitmap( load_file, NULL );
	// my_bitmap = load_bitmap( load_file, NULL );
	setBitmap( create_bitmap( sx, sy ) );
	// clear( my_bitmap );
	if ( !temp || ! getBitmap() ){
		cout<<"Could not load "<<load_file<<endl;
		error = true;
	} else {
		clear();
		stretch_blit( temp, getBitmap(), 0, 0, temp->w, temp->h, 0, 0, getBitmap()->w, getBitmap()->h );
		destroy_bitmap( temp );
	}
	// own = true;
	own = new int;
	*own = 1;
}

Bitmap::Bitmap( const char * load_file, int sx, int sy, double accuracy ):
own( NULL ),
error( false ){
	path = load_file;
	BITMAP * temp = load_bitmap( load_file, NULL );
	own = new int;
	*own = 1;
	if ( !temp ){
		cout<<"Could not load "<<load_file<<endl;
		setBitmap( create_bitmap( sx, sy ) );
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
			setBitmap( create_bitmap( fx, fy ) );
			
			stretch_blit( temp, getBitmap(), 0, 0, temp->w, temp->h, 0, 0, getBitmap()->w, getBitmap()->h );
			
			destroy_bitmap( temp );
		} else setBitmap( temp );
	}
	// own = true;
}

Bitmap::Bitmap( const Bitmap & copy, int sx, int sy ):
own( NULL ),
error( false ){
	path = copy.getPath();
	BITMAP * temp = copy.getBitmap();
	setBitmap( create_bitmap( sx, sy ) );
	if ( ! getBitmap() ){
		error = true;
		cout << "Could not copy bitmap" << endl;
	}
	// clear( my_bitmap );
	clear();
	stretch_blit( temp, getBitmap(), 0, 0, temp->w, temp->h, 0, 0, getBitmap()->w, getBitmap()->h );
	// own = true;
	// own = copy.own;
	own = new int;
	*own = 1;
}

Bitmap::Bitmap( const Bitmap & copy, int sx, int sy, double accuracy ):
own( NULL ),
error( false ){
	path = copy.getPath();
	BITMAP * temp = copy.getBitmap();
	own = new int;
	*own = 1;
	if ( temp->w > sx || temp->h > sy ){
		double bx = (double)temp->w / (double)sx;
		double by = (double)temp->h / (double)sy;
		double use;
		use = bx > by ? bx : by;
		int fx = (int)(temp->w / use);
		int fy = (int)(temp->h / use);
		setBitmap( create_bitmap( fx, fy ) );
		if ( ! getBitmap() ){
			allegro_message("Could not create bitmap\n");
			// own = false;
			// own = copy.own;
			// *own++;
			error = true;
			return;
		}
	
		stretch_blit( temp, getBitmap(), 0, 0, temp->w, temp->h, 0, 0, getBitmap()->w, getBitmap()->h );
		// destroy_bitmap( temp );
	} else {
		setBitmap( create_bitmap( temp->w, temp->h ) );
		blit( temp, getBitmap(), 0, 0, 0, 0, temp->w, temp->h );
		// own = new int
		// own = true;
	}
}

Bitmap::Bitmap( const Bitmap & copy, bool deep_copy ):
own( NULL ),
error( false ){

	path = copy.getPath();
	if ( deep_copy ){
		BITMAP * his = copy.getBitmap();
		setBitmap( create_bitmap( his->w, his->h ) );
		if ( ! getBitmap() ){
			cout << "Could not create bitmap" << endl;
			error = true;
		}
		::blit( his, getBitmap(), 0, 0, 0, 0, his->w, his->h );
		own = new int;
		*own = 1;
	} else {
		setBitmap( copy.getBitmap() );
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
error( false ){
	path = copy.getPath();
	BITMAP * his = copy.getBitmap();
	if ( x < 0 )
		x = 0;
	if ( y < 0 )
		y = 0;
	if ( width > his->w )
		width = his->w;
	if ( height > his->h )
		height = his->h;
	setBitmap( create_sub_bitmap( his, x, y, width, height ) );

	if ( ! getBitmap() ){
		cout<<"Could not create sub-bitmap"<<endl;
		setBitmap( create_bitmap( 10, 10 ) );
		// clear( my_bitmap );
		clear();
	}
	// own = true;
	own = new int;
	*own = 1;
}
	
void Bitmap::load( const string & str ){
	releaseInternalBitmap();
	internalLoadFile( str.c_str() );
}
	
void Bitmap::internalLoadFile( const char * load_file ){
	path = load_file;
	setBitmap( load_bitmap( load_file, NULL ) );
	if ( ! getBitmap() ){
		cout<<"Could not load "<<load_file<<". Using default"<<endl;
		setBitmap( create_bitmap( 10, 10 ) );
		if ( ! getBitmap() ){
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
	
void Bitmap::save( const string & str ){
	save_bitmap( str.c_str(), getBitmap(), NULL );
}

/* decrement bitmap reference counter and free memory if counter hits 0 */
void Bitmap::releaseInternalBitmap(){
	if ( own ){
		(*own)--;
		if ( *own == 0 ){
			delete own;
			destroy_bitmap( getBitmap() );
			own = NULL;
		}
	}
}

const int Bitmap::getWidth() const{
	return getBitmap()->w;
}

const int Bitmap::getHeight() const{
	return getBitmap()->h;
}
	
int Bitmap::getRed( int x ){
	return ::getr( x );
}

int Bitmap::getBlue( int x ){
	return ::getg( x );
}

int Bitmap::getGreen( int x ){
	return ::getb( x );
}

void Bitmap::setClipRect( int x1, int y1, int x2, int y2 ) const
{
	::set_clip_rect( getBitmap(), x1, y1, x2, y2 );
}

/* resize the internal bitmap. not guaranteed to destroy the internal bitmap */
void Bitmap::resize( const int width, const int height ){

	/* if internal bitmap is already the proper size, do nothing */
	if ( getWidth() == width && getHeight() == height ){
		return;
	}

	BITMAP * b = create_bitmap( width, height );
	::stretch_blit( getBitmap(), b, 0, 0, getBitmap()->w, getBitmap()->h, 0, 0, b->w, b->h );

	releaseInternalBitmap();

	own = new int;
	setBitmap( b );
	*own = 1;
}

/*
const string & Bitmap::getPath() const{
	return path;
}
*/

void Bitmap::debugSelf() const{
	cout<<"Bitmap: "<<endl;
	cout<<"Self = "<< getBitmap() <<endl;
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
	setBitmap( copy.getBitmap() );
	// own = false;
	own = copy.own;
	if ( own )
		*own++;

	return *this;
}

Bitmap::~Bitmap(){
	/*
	if ( own ){
		(*own)--;

		if ( *own == 0 ){
			destroy_bitmap( getBitmap() );
			delete own;
		} else {
			// cout<<"Not destroying bitmap "<<this<<endl;
		}
	}
	*/
	releaseInternalBitmap();
}
	
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
	acquire_bitmap( getBitmap() );
}

void Bitmap::release(){
	release_bitmap( getBitmap() );
}

void Bitmap::circleFill( int x, int y, int radius, int color ) const{
	::circlefill( getBitmap(), x, y, radius, color );
}
	
void Bitmap::circle( int x, int y, int radius, int color ) const{
	::circle( getBitmap(), x, y, radius, color );
}
	
void Bitmap::line( const int x1, const int y1, const int x2, const int y2, const int color ) const{

	::fastline( getBitmap(), x1, y1, x2, y2, color );
	
}
	
void Bitmap::floodfill( const int x, const int y, const int color ) const {
	::floodfill( getBitmap(), x, y, color );
}
	
void Bitmap::drawCharacter( const int x, const int y, const int color, const int background, const Bitmap & where ) const {
	::draw_character_ex( where.getBitmap(), getBitmap(), x, y, color, background );
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
	int ok = ::set_gfx_mode( mode, width, height, 0, 0 );
        if ( Screen != NULL ){
            delete Screen;
        }
        if ( Scaler != NULL ){
            delete Scaler;
        }
        if ( Buffer != NULL ){
            delete Buffer;
        }
	if ( ok == 0 ){
		Screen = new Bitmap( ::screen );
	}
        if ( width != 0 && height != 0 && (width != SCALE_X | height != SCALE_Y) ){
            Scaler = new Bitmap(width, height);
            Buffer = new Bitmap(SCALE_X, SCALE_Y);
        }
	return ok;
}

double Bitmap::getScale(){
    /* the game is pretty much hard coded to run at 320 scaled upto 640
     * and then scaled to whatever the user wants, but as long as
     * 320 and 640 remain this number will be 2.
     * maybe calculate this at some point
     */
    return 2;
    /*
    if (Scaler != NULL && Buffer != NULL){
        double x1 = Scaler->getWidth();
        double x2 = Buffer->getWidth();
        return x2 / x1;
    }
    return 1;
    */
}

/*
const int Bitmap::getWidth() const{
	return getBitmap()->w;
}

const int Bitmap::getHeight() const{
	return getBitmap()->h;
}
*/

const int Bitmap::getPixel( const int x, const int y ) const{
	if ( x >= 0 && x < getBitmap()->w && y >= 0 && y < getBitmap()->h )
		return _getpixel16( getBitmap(), x, y );
	return -1;
}

void Bitmap::readLine( vector< int > & vec, int y ){
	if ( y >= 0 && y < getBitmap()->h ){
		for ( int q = 0; q < getBitmap()->w; q++ ){
			// int col = my_bitmap->line[ y ][ q ];
			int col = _getpixel16( getBitmap(), q, y );
			vec.push_back( col );
		}
	}
}

int Bitmap::setGfxModeText(){
	return setGraphicsMode( GFX_TEXT, 0, 0 );
}
	
int Bitmap::setGfxModeFullscreen( int x, int y ){
	return setGraphicsMode( GFX_AUTODETECT_FULLSCREEN, x, y );
}

int Bitmap::setGfxModeWindowed( int x, int y ){
	return setGraphicsMode( GFX_AUTODETECT_WINDOWED, x, y );
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
	
int Bitmap::addColor( int color1, int color2 ){
	return makeColor( getr( color1 ) + getr( color2 ),
			  getg( color1 ) + getg( color2 ),
			  getb( color1 ) + getb( color2 ) );
}
	
void Bitmap::putPixel( int x, int y, int col ) const{
	if ( x >= 0 && x < getBitmap()->w && y >= 0 && y < getBitmap()->h )
		_putpixel16( getBitmap(), x, y, col );
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
	::triangle( getBitmap(), x1, y1, x2, y2, x3, y3, color );
}
	
void Bitmap::ellipse( int x, int y, int rx, int ry, int color ) const {
	::ellipse( getBitmap(), x, y, rx, ry, color );
}

void Bitmap::ellipseFill( int x, int y, int rx, int ry, int color ) const {
	::ellipsefill( getBitmap(), x, y, rx, ry, color );
}

void Bitmap::rectangle( int x1, int y1, int x2, int y2, int color ) const{
	::rect( getBitmap(), x1, y1, x2, y2, color );
}
	
void Bitmap::border( int min, int max, int color ) const {
	int w = getWidth();
	int h = getHeight();
	for ( int i = min; i < max; i++ ){
		rectangle( i, i, w - 1 - i, h - 1 - i, color );
	}
}

void Bitmap::rectangleFill( int x1, int y1, int x2, int y2, int color ) const{
	::rectfill( getBitmap(), x1, y1, x2, y2, color );
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
	::hline( getBitmap(), x1, y, x2, color );
}
	
void Bitmap::horizontalLine( const int x1, const int y, const int x2, const int color ) const{
	this->hLine( x1, y, x2, color );
}

void Bitmap::vLine( const int y1, const int x, const int y2, const int color ) const{
	::vline( getBitmap(), x, y1, y2, color );
}
	
void Bitmap::polygon( const int * verts, const int nverts, const int color ) const{
	::polygon( getBitmap(), nverts, verts, color );
}

void Bitmap::arc(const int x, const int y, const double ang1, const double ang2, const int radius, const int color ) const{
	::arc( getBitmap(), x, y, ::ftofix(ang1), ::ftofix(ang2), radius, color );
}

/*
void Bitmap::clear(){
	this->fill( 0 );
}
*/

void Bitmap::fill( int color ) const{
	::clear_to_color( getBitmap(), color );
}
	
void Bitmap::draw( const int x, const int y, const Bitmap & where ) const {
	paintown_draw_sprite_ex16( where.getBitmap(), getBitmap(), x, y, Bitmap::SPRITE_NORMAL, Bitmap::SPRITE_NO_FLIP );
	// ::draw_sprite( where.getBitmap(), getBitmap(), x, y );
}
	
void Bitmap::drawHFlip( const int x, const int y, const Bitmap & where ) const {
	paintown_draw_sprite_ex16( where.getBitmap(), getBitmap(), x, y, Bitmap::SPRITE_NORMAL, Bitmap::SPRITE_H_FLIP );
	// ::draw_sprite_h_flip( where.getBitmap(), getBitmap(), x, y );
}

void Bitmap::drawVFlip( const int x, const int y, const Bitmap & where ) const {
	paintown_draw_sprite_ex16( where.getBitmap(), getBitmap(), x, y, Bitmap::SPRITE_NORMAL, Bitmap::SPRITE_V_FLIP );
	// ::draw_sprite_h_flip( where.getBitmap(), getBitmap(), x, y );
}
	
void Bitmap::drawLit( const int x, const int y, const int level, const Bitmap & where ) const{
	::draw_lit_sprite( where.getBitmap(), getBitmap(), x, y, level );
}

void Bitmap::drawTrans( const int x, const int y, const Bitmap & where ) const{
	// ::draw_trans_sprite( where.getBitmap(), getBitmap(), x, y );
	paintown_draw_sprite_ex16( where.getBitmap(), getBitmap(), x, y, Bitmap::SPRITE_TRANS, Bitmap::SPRITE_NO_FLIP );
}
	
void Bitmap::drawTransVFlip( const int x, const int y, const Bitmap & where ) const {
	paintown_draw_sprite_ex16( where.getBitmap(), getBitmap(), x, y, Bitmap::SPRITE_TRANS, Bitmap::SPRITE_V_FLIP );
}

void Bitmap::drawRotate( const int x, const int y, const int angle, const Bitmap & where ){
	::fixed fang = itofix( (360 - angle) % 360 * 256 / 360 );
	::rotate_sprite( where.getBitmap(), getBitmap(), x, y, fang ); 
}

void Bitmap::drawStretched( const int x, const int y, const int new_width, const int new_height, const Bitmap & who ){
	BITMAP * bmp = who.getBitmap();
	::masked_stretch_blit( getBitmap(), bmp, 0, 0, getBitmap()->w, getBitmap()->h, x,y, new_width, new_height );
}

void Bitmap::drawMask( const int _x, const int _y, const Bitmap & where ){
	int mask = Bitmap::MaskColor;
	for ( int x = 0; x < getWidth(); x++ ){
		for ( int y = 0; y < getHeight(); y++ ){
			if ( getPixel( x,y ) == mask ){
				where.putPixel( x+_x, y+_y, mask );
			}
		}
	}
}

void Bitmap::StretchBy2( const Bitmap & where ){
	BITMAP * bmp = where.getBitmap();

	if ( where.getWidth() == getWidth() && where.getHeight() == getHeight() ){
		::blit( getBitmap(), bmp, 0, 0, 0, 0, getBitmap()->w, getBitmap()->h );
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
	::stretch_blit( getBitmap(), bmp, 0, 0, getBitmap()->w, getBitmap()->h, 0, 0, bmp->w, bmp->h );
	// fblend_2x_stretch( my_bitmap, bmp, 0, 0, 0, 0, my_bitmap->w, my_bitmap->h);
	// scale2x_allegro( my_bitmap, bmp, 2 );
	// debug

}

void Bitmap::StretchBy4( const Bitmap & where ){

	BITMAP * bmp = where.getBitmap();
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
	::stretch_blit( getBitmap(), bmp, 0, 0, getBitmap()->w, getBitmap()->h, 0, 0, bmp->w, bmp->h );

}

void Bitmap::Stretch( const Bitmap & where ) const {
	Stretch( where, 0, 0, getBitmap()->w, getBitmap()->h, 0, 0, where.getBitmap()->w, where.getBitmap()->h );
	/*
	BITMAP * bmp = where.getBitmap();
	::stretch_blit( getBitmap(), bmp, 0, 0, getBitmap()->w, getBitmap()->h, 0, 0, bmp->w, bmp->h );
	*/
}
	
void Bitmap::Stretch( const Bitmap & where, const int sourceX, const int sourceY, const int sourceWidth, const int sourceHeight, const int destX, const int destY, const int destWidth, const int destHeight ) const {
	BITMAP * bmp = where.getBitmap();
	::stretch_blit( getBitmap(), bmp, sourceX, sourceY, sourceWidth, sourceHeight, destX, destY, destWidth, destHeight );
}

void Bitmap::Blit( const string & xpath ) const {
	Bitmap duh( xpath );
	duh.Blit( *this );
}

void Bitmap::Blit( const int x, const int y, const Bitmap & where ) const {
	BITMAP * bmp = where.getBitmap();
	/*
	acquire_bitmap( bmp );
	acquire_bitmap( my_bitmap );
	*/
	::blit( getBitmap(), bmp, 0, 0, x, y, getBitmap()->w, getBitmap()->h );
	/*
	release_bitmap( my_bitmap );
	release_bitmap( bmp );
	*/
}

void Bitmap::Blit( const int mx, const int my, const int wx, const int wy, const Bitmap & where ) const {
	BITMAP * bmp = where.getBitmap();
	::blit( getBitmap(), bmp, mx, my, wx, wy, getBitmap()->w, getBitmap()->h );
}

void Bitmap::Blit( const int mx, const int my, const int width, const int height, const int wx, const int wy, Bitmap & where ) const {
	BITMAP * bmp = where.getBitmap();
	::blit( getBitmap(), bmp, mx, my, wx, wy, width, height );
}
	
void Bitmap::Blit( const Bitmap & where ) const {
	this->Blit( 0, 0, where );
}

void Bitmap::BlitToScreen() const {
	// this->Blit( *Bitmap::Screen );
	this->BlitToScreen( 0, 0 );
}
	
void Bitmap::BlitToScreen(const int upper_left_x, const int upper_left_y) const {
    if ( Scaler == NULL ){
        this->Blit( upper_left_x, upper_left_y, *Bitmap::Screen );
    } else {
        this->Blit( upper_left_x, upper_left_y, *Buffer );
        Buffer->Stretch( *Scaler );
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

LitBitmap::LitBitmap( const Bitmap & b ):
Bitmap( b ){
}

LitBitmap::LitBitmap():
Bitmap(){
}
	
LitBitmap::~LitBitmap(){
}

void LitBitmap::draw( const int x, const int y, const Bitmap & where ) const {
	// ::draw_sprite_ex( where.getBitmap(), getBitmap(), x, y, SPRITE_LIT );
	// ::draw_sprite( where.getBitmap(), getBitmap(), x, y );
	// Bitmap::draw( x, y, where );
	paintown_draw_sprite_ex16( where.getBitmap(), getBitmap(), x, y, Bitmap::SPRITE_LIT, Bitmap::SPRITE_NO_FLIP );
}
	
void LitBitmap::drawHFlip( const int x, const int y, const Bitmap & where ) const {
	// ::draw_sprite_h_flip_ex( where.getBitmap(), getBitmap(), x, y, SPRITE_LIT );
	paintown_draw_sprite_ex16( where.getBitmap(), getBitmap(), x, y, Bitmap::SPRITE_LIT, Bitmap::SPRITE_H_FLIP );
	// Bitmap::drawHFlip( x, y, where );
	// ::draw_sprite_h_flip_ex( where.getBitmap(), getBitmap(), x, y, SPRITE_LIT );
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
			    case Bitmap::SPRITE_TRANS : {
			    		c = PAINTOWN_DTS_BLEND(trans_blender, PAINTOWN_GET_PIXEL(d), c);
					break;
			    }
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
	                    case Bitmap::SPRITE_TRANS : {
			    		c = PAINTOWN_DTS_BLEND(trans_blender, PAINTOWN_GET_PIXEL(d), c);
					break;
			    }
			 }
		    }
	       PAINTOWN_PUT_MEMORY_PIXEL(d, c);
	    }
	 }
      }
   }
}
