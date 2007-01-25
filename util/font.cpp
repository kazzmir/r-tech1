#include <allegro.h>
#include "font.h"

Font::Font( FONT * f ):
my_font( f ){
}
	
Font::Font( const Font & f ):
my_font( f.getInternalFont() ){
}
	
const int Font::textLength( const char * text ){
	return text_length( getInternalFont(), text );
}

const int Font::getHeight() const{
	return text_height( getInternalFont() );
}
