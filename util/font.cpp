#include <allegro.h>
#include "font.h"
#include "init.h"

Font::Font( FONT * f ):
my_font( f ){
}
	
Font::Font( const Font & f ):
my_font( f.getInternalFont() ){
}
	
const int Font::textLength( const char * text ) const{
	return text_length( getInternalFont(), text );
}

const int Font::getHeight() const{
	return text_height( getInternalFont() );
}

const Font getFont( int index ){
	return Font( (FONT *) Global::all_fonts[ index ].dat );
}

const Font getDefaultFont(){
	return Font( font );
}
