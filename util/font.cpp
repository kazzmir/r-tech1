#include <allegro.h>
#include "font.h"
#include "init.h"
#include "factory/font_factory.h"

Font::Font(){
}

Font::~Font(){
}

AllegroFont::AllegroFont( const FONT * const font ):
font( font ){
}
	
AllegroFont::AllegroFont( const AllegroFont & copy ):
font( copy.getInternalFont() ){
}

AllegroFont::~AllegroFont(){
}
	
const int AllegroFont::textLength( const char * text ) const{
	return text_length( getInternalFont(), text );
}

const int AllegroFont::getHeight( const string & str ) const {
	return getHeight();
}

const int AllegroFont::getHeight() const {
	return text_height( getInternalFont() );
}

void AllegroFont::setSize( const int x, const int y ){
}

const int AllegroFont::getSizeX() const {
	return 0;
}

const int AllegroFont::getSizeY() const {
	return 0;
}
	
void AllegroFont::printf( int x, int y, int color, const Bitmap & work, const string & str, int marker, ... ) const {
	char buf[512];
	va_list ap;

	va_start(ap, marker);
	uvszprintf(buf, sizeof(buf), str.c_str(), ap);
	va_end(ap);

	textout_ex( work.getBitmap(), getInternalFont(), buf, x, y, color, -1 );
}

const Font & Font::getDefaultFont(){
	// return getFont( "tmp/comic.ttf" );
	return getFont( "bios", 16, 16 );
}
	
const Font & Font::getFont( const string & name, const int x, const int y ){
	return *FontFactory::getFont( name, x, y );
}

FreeTypeFont::FreeTypeFont( const string & str ):
sizeX( 16 ),
sizeY( 16 ){
	this->font = new ftalleg::freetype( str, getSizeX(), getSizeY() );
}

const int FreeTypeFont::getHeight( const string & str ) const {
	return this->font->getHeight( str );
}

const int FreeTypeFont::getHeight() const {
	return getHeight( "A" );
}

const int FreeTypeFont::textLength( const char * text ) const {
	return this->font->getLength( string( text ) );
}
	
void FreeTypeFont::printf( int x, int y, int color, const Bitmap & work, const string & str, int marker, ... ) const {
	char buf[512];
	va_list ap;

	va_start(ap, marker);
	uvszprintf(buf, sizeof(buf), str.c_str(), ap);
	va_end(ap);

	this->font->render( x, y, color, work.getBitmap(), ftalleg::freetype::ftLeft, string( buf ), 0 );
}

void FreeTypeFont::setSize( const int x, const int y ){
	this->sizeX = x;
	this->sizeY = y;
	this->font->setSize( this->sizeX, this->sizeY );
}

const int FreeTypeFont::getSizeX() const {
	return this->sizeX;
}

const int FreeTypeFont::getSizeY() const {
	return this->sizeY;
}

FreeTypeFont::~FreeTypeFont(){
	// cout << "Delete font " << this->font << endl;
	delete this->font;
}
