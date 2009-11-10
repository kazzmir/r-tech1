#include <allegro.h>
#include "font.h"
#include "init.h"
#include "factory/font_factory.h"

using namespace std;

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
	
int AllegroFont::textLength( const char * text ) const{
	return text_length( getInternalFont(), text );
}

int AllegroFont::getHeight( const string & str ) const {
	return getHeight();
}

int AllegroFont::getHeight() const {
	return text_height( getInternalFont() );
}

void AllegroFont::setSize( const int x, const int y ){
}

int AllegroFont::getSizeX() const {
	return 0;
}

int AllegroFont::getSizeY() const {
	return 0;
}
	
void AllegroFont::printf( int x, int y, int xSize, int ySize, int color, const Bitmap & work, const string & str, int marker, ... ) const {
    char buf[512];
    va_list ap;

    va_start(ap, marker);
    uvszprintf(buf, sizeof(buf), str.c_str(), ap);
    va_end(ap);

    textout_ex( work.getBitmap(), getInternalFont(), buf, x, y, color, -1 );
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
    Font & font = *FontFactory::getFont(name, x, y);
    /* sanity check */
    if (font.getHeight("A") == 0){
        return getDefaultFont();
    }
    return font;
}

FreeTypeFont::FreeTypeFont( const string & str ):
sizeX( 16 ),
sizeY( 16 ),
own(true){
	this->font = new ftalleg::freetype( str, getSizeX(), getSizeY() );
}

int FreeTypeFont::getHeight( const string & str ) const {
	return this->font->getHeight( str );
}

int FreeTypeFont::getHeight() const {
	return getHeight( "A" );
}

int FreeTypeFont::textLength( const char * text ) const {
	return this->font->getLength( string( text ) );
}

void FreeTypeFont::printf( int x, int y, int xSize, int ySize, int color, const Bitmap & work, const string & str, int marker, ... ) const {
    char buf[512];
    va_list ap;

    va_start(ap, marker);
    vsnprintf(buf, sizeof(buf), str.c_str(), ap);
    va_end(ap);

    int old_x = 0;
    int old_y = 0;
    this->font->getSize(&old_x, &old_y);

    this->font->setSize(xSize, ySize);

    this->font->render( x, y, color, work, ftalleg::freetype::ftLeft, string( buf ), 0 );

    this->font->setSize(old_x, old_y);

}
	
void FreeTypeFont::printf( int x, int y, int color, const Bitmap & work, const string & str, int marker, ... ) const {
    char buf[512];
    va_list ap;

    va_start(ap, marker);
    vsnprintf(buf, sizeof(buf), str.c_str(), ap);
    va_end(ap);

    this->font->render( x, y, color, work, ftalleg::freetype::ftLeft, string( buf ), 0 );
}

void FreeTypeFont::setSize( const int x, const int y ){
	this->sizeX = x;
	this->sizeY = y;
	this->font->setSize( this->sizeX, this->sizeY );
}

int FreeTypeFont::getSizeX() const {
	return this->sizeX;
}

int FreeTypeFont::getSizeY() const {
	return this->sizeY;
}

FreeTypeFont::~FreeTypeFont(){
	// cout << "Delete font " << this->font << endl;
    if (own){
        delete this->font;
    }
}
