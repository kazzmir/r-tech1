#ifndef _paintown_font_h
#define _paintown_font_h

struct FONT;

#include <string>
#include <vector>
#include "bitmap.h"
#include "ftalleg.h"

using namespace std;

/* handle allegro fonts and true type fonts */
class Font{
public:
	Font();
	virtual ~Font();

	virtual void setSize( const int x, const int y ) = 0;
	virtual const int getSizeX() const = 0;
	virtual const int getSizeY() const = 0;
	
	virtual const int textLength( const char * text ) const = 0;

	virtual const int getHeight( const string & str ) const = 0;
	virtual const int getHeight() const = 0;

	virtual void printf( int x, int y, int color, const Bitmap & work, const string & str, int marker, ... ) const = 0;

	static const Font & getDefaultFont();
	static const Font & getFont( const string & name, const int x = 32, const int y = 32 );

	/* store all the freetype fonts forever */
	static vector< ftalleg::freetype * > cacheFreeType; 
};

class AllegroFont: public Font {
public:
	AllegroFont( const FONT * const font );
	AllegroFont( const AllegroFont & copy );
	virtual ~AllegroFont();

	virtual const int getHeight() const;
	virtual const int getHeight( const string & str ) const;
	virtual const int textLength( const char * text ) const;
	
	virtual void printf( int x, int y, int color, const Bitmap & work, const string & str, int marker, ... ) const;
	
	virtual void setSize( const int x, const int y );
	virtual const int getSizeX() const;
	virtual const int getSizeY() const;

private:
	inline const FONT * const getInternalFont() const{
		return font;
	}

	const FONT * const font;
};

class FreeTypeFont: public Font {
public:
	FreeTypeFont( const string & filename );
	FreeTypeFont( const FreeTypeFont & copy );
	virtual ~FreeTypeFont();

	virtual const int getHeight() const;
	virtual const int getHeight( const string & str ) const;
	virtual const int textLength( const char * text ) const;
	
	virtual void printf( int x, int y, int color, const Bitmap & work, const string & str, int marker, ... ) const;
	
	virtual void setSize( const int x, const int y );
	virtual const int getSizeX() const;
	virtual const int getSizeY() const;

private:
	ftalleg::freetype * font;
	int sizeX;
	int sizeY;
};

#endif
