#include <allegro.h>
#include "bitmap.h"
#include <stdarg.h>
#include <vector>
#include <string>
#include <iostream>
// #include <fblend.h>

#ifdef WINDOWS
#include <winalleg.h>
#endif

using namespace std;

#ifndef debug
#define debug cout<<"File: "<<__FILE__<<" Line: "<<__LINE__<<endl;
#endif

const int Bitmap::MaskColor = MASK_COLOR_16;

const int Bitmap::MODE_TRANS = 0;
const int Bitmap::MODE_SOLID = 1;

Bitmap * Bitmap::Screen;

Bitmap::Bitmap():
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
error( false ){
	
	if ( deep_copy ){
		BITMAP * his = who;
		setBitmap( create_bitmap( his->w, his->h ) );
		::blit( his, getBitmap(), 0, 0, 0, 0, his->w, his->h );
		own = new int;
		*own = 1;
	} else {
		setBitmap( who );
		own = NULL;
	}
}
	
Bitmap::Bitmap( const char * load_file ):
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
error( false ){
	internalLoadFile( load_file.c_str() );
}

Bitmap::Bitmap( const char * load_file, int sx, int sy ):
error( false ){
	path = load_file;
	BITMAP * temp = load_bitmap( load_file, NULL );
	// my_bitmap = load_bitmap( load_file, NULL );
	setBitmap( create_bitmap( sx, sy ) );
	// clear( my_bitmap );
	clear();
	if ( !temp ){
		cout<<"Could not load "<<load_file<<endl;
		error = true;
	} else {
		stretch_blit( temp, getBitmap(), 0, 0, temp->w, temp->h, 0, 0, getBitmap()->w, getBitmap()->h );
		destroy_bitmap( temp );
	}
	// own = true;
	own = new int;
	*own = 1;
}

Bitmap::Bitmap( const char * load_file, int sx, int sy, double accuracy ):
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
error( false ){
	path = copy.getPath();
	BITMAP * temp = copy.getBitmap();
	setBitmap( create_bitmap( sx, sy ) );
	// clear( my_bitmap );
	clear();
	stretch_blit( temp, getBitmap(), 0, 0, temp->w, temp->h, 0, 0, getBitmap()->w, getBitmap()->h );
	// own = true;
	// own = copy.own;
	own = new int;
	*own = 1;
}

Bitmap::Bitmap( const Bitmap & copy, int sx, int sy, double accuracy ):
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
error( false ){

	path = copy.getPath();
	if ( deep_copy ){
		BITMAP * his = copy.getBitmap();
		setBitmap( create_bitmap( his->w, his->h ) );
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

	::line( getBitmap(), x1, y1, x2, y2, color );
	
}
	
void Bitmap::transBlender( int r, int g, int b, int a ){
	set_trans_blender( r, g, b, a );
}
	
void Bitmap::multiplyBlender( int r, int g, int b, int a ){
	set_multiply_blender( r, g, b, a );
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
	return ::set_gfx_mode( GFX_TEXT, 0, 0, 0, 0 );
}
	
int Bitmap::setGfxModeFullscreen( int x, int y ){
	return ::set_gfx_mode( GFX_AUTODETECT_FULLSCREEN, x, y, 0, 0 );
}

int Bitmap::setGfxModeWindowed( int x, int y ){
	return ::set_gfx_mode( GFX_AUTODETECT_WINDOWED, x, y, 0, 0 );
}
	
int Bitmap::makeColor( int r, int g, int b ){
	return ::makecol16( r, g, b );
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

/*
void Bitmap::clear(){
	this->fill( 0 );
}
*/

void Bitmap::fill( int color ) const{
	::clear_to_color( getBitmap(), color );
}
	
void Bitmap::draw( const int x, const int y, const Bitmap & where ) const {
	::draw_sprite( where.getBitmap(), getBitmap(), x, y );
}
	
void Bitmap::drawHFlip( const int x, const int y, const Bitmap & where ) const {
	::draw_sprite_h_flip( where.getBitmap(), getBitmap(), x, y );
}
	
void Bitmap::drawLit( const int x, const int y, const int level, const Bitmap & where ) const{
	::draw_lit_sprite( where.getBitmap(), getBitmap(), x, y, level );
}

void Bitmap::drawTrans( const int x, const int y, const Bitmap & where ) const{
	::draw_trans_sprite( where.getBitmap(), getBitmap(), x, y );
}

void Bitmap::drawRotate( const int x, const int y, const int angle, const Bitmap & where ){
	::fixed fang = itofix( (360 - angle) % 360 * 256 / 360 );
	::rotate_sprite( where.getBitmap(), getBitmap(), x, y, fang ); 
}

void Bitmap::drawStretched( const int x, const int y, const int new_width, const int new_height, const Bitmap & who ){
	BITMAP * bmp = who.getBitmap();
	::masked_stretch_blit( getBitmap(), bmp, 0, 0, getBitmap()->w, getBitmap()->h, x,y, new_height, new_width );
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

void Bitmap::Stretch( const Bitmap & where ){
	BITMAP * bmp = where.getBitmap();
	::stretch_blit( getBitmap(), bmp, 0, 0, getBitmap()->w, getBitmap()->h, 0, 0, bmp->w, bmp->h );
}
	
void Bitmap::Blit( const string & xpath ){
	Bitmap duh( xpath );
	duh.Blit( *this );
}

void Bitmap::Blit( const int x, const int y, const Bitmap & where ){
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

void Bitmap::Blit( const int mx, const int my, const int wx, const int wy, const Bitmap & where ){
	BITMAP * bmp = where.getBitmap();
	::blit( getBitmap(), bmp, mx, my, wx, wy, getBitmap()->w, getBitmap()->h );
}

void Bitmap::Blit( const int mx, const int my, const int width, const int height, const int wx, const int wy, Bitmap & where ){
	BITMAP * bmp = where.getBitmap();
	::blit( getBitmap(), bmp, mx, my, wx, wy, width, height );
}
	
void Bitmap::Blit( const Bitmap & where ){
	this->Blit( 0, 0, where );
}

void Bitmap::BlitToScreen(){
	this->Blit( *Bitmap::Screen );
}
