#ifndef _paintown_font_h
#define _paintown_font_h

struct FONT;

class Font{
public:
	Font( FONT * f );
	Font( const Font & f );

	inline FONT * getInternalFont() const{
		return my_font;
	}
	
	const int textLength( const char * text ) const;

	const int getHeight() const;

protected:
	FONT * my_font;
};

const Font getFont( int index );
const Font getDefaultFont();

#endif
