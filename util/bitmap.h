#ifndef _bitmap_h_
#define _bitmap_h_

#include <string>
#include <vector>

#include <iostream>

#ifdef WINDOWS
#define BITMAP dummyBITMAP
#include <windows.h>
#undef BITMAP
#endif

struct BITMAP;
struct FONT;
class Font;

using namespace std;

class Bitmap{
public:

	static Bitmap * Screen;
	
	/* default constructor makes 10x10 bitmap */
	Bitmap();
	Bitmap( int x, int y );
	Bitmap( const char * load_file );
	Bitmap( const string & load_file );
	Bitmap( const char * load_file, int sx, int sy );
	Bitmap( const char * load_file, int sx, int sy, double accuracy );
	Bitmap( BITMAP * who, bool deep_copy = false );
	Bitmap( const Bitmap & copy, bool deep_copy = false );
	Bitmap( const Bitmap & copy, int sx, int sy );
	Bitmap( const Bitmap & copy, int sx, int sy, double accuracy );
	Bitmap( const Bitmap & copy, int x, int y, int width, int height );
	virtual ~Bitmap();

	virtual void save( const string & str );

	Bitmap & operator=( const Bitmap & );

	virtual void load( const string & str );

	const int getWidth() const;
	const int getHeight() const;

	/*
	inline const int getWidth() const{
		return getBitmap()->w;
	}

	inline const int getHeight() const{
		return getBitmap()->h;
	}
	*/

	/*
	inline const int getWidth() const{
		return my_bitmap->w;
	}

	inline const int getHeight() const{
		return my_bitmap->h;
	}
	*/

	void detach();

	static void transBlender( int r, int g, int b, int a );
	static void multiplyBlender( int r, int g, int b, int a );
	static void dissolveBlender( int r, int g, int b, int a );
	static int setGraphicsMode( int mode, int width, int height );

	static void drawingMode( int type );

	void acquire();
	void release();

	void resize( const int width, const int height );
	
	void debugSelf() const;

	/*
	void printf( int x, int y, int color, FONT * f, const char * str, ... ) const;
	void printf( int x, int y, int color, const Font * const f, const char * str, ... ) const;
	void printf( int x, int y, int color, const Font * const f, const string & str ) const;
	void printf( int x, int y, int color, const Font & f, const string & str ) const;
	void printfNormal( int x, int y, int color, const char * str, ... ) const;
	void printfNormal( int x, int y, int color, const string & str ) const;
	*/

	virtual void triangle( int x1, int y1, int x2, int y2, int x3, int y3, int color ) const;
	virtual void ellipse( int x, int y, int rx, int ry, int color ) const;
	virtual void ellipseFill( int x, int y, int rx, int ry, int color ) const;

	virtual void border( int min, int max, int color ) const;
	virtual void rectangle( int x1, int y1, int x2, int y2, int color ) const;
	virtual void rectangleFill( int x1, int y1, int x2, int y2, int color ) const;
	virtual void circleFill( int x, int y, int radius, int color ) const;
	virtual void circle( int x, int y, int radius, int color ) const;
	virtual void line( const int x1, const int y1, const int x2, const int y2, const int color ) const;
		
	virtual void floodfill( const int x, const int y, const int color ) const;
	virtual void horizontalLine( const int x1, const int y, const int x2, const int color ) const;
	virtual void hLine( const int x1, const int y, const int x2, const int color ) const;
	virtual void vLine( const int y1, const int x, const int y2, const int color ) const;
	virtual void polygon( const int * verts, const int nverts, const int color ) const;
	virtual void arc(const int x, const int y, const double ang1, const double ang2, const int radius, const int color ) const;

	virtual void draw( const int x, const int y, const Bitmap & where ) const;
	virtual void drawCharacter( const int x, const int y, const int color, const int background, const Bitmap & where ) const;
	virtual void drawLit( const int x, const int y, const int level, const Bitmap & where ) const;
	virtual void drawHFlip( const int x, const int y, const Bitmap & where ) const;
	virtual void drawVFlip( const int x, const int y, const Bitmap & where ) const;
	virtual void drawTrans( const int x, const int y, const Bitmap & where ) const;
	virtual void drawTransVFlip( const int x, const int y, const Bitmap & where ) const;
	virtual void drawMask( const int x, const int y, const Bitmap & where );
	virtual void drawStretched( const int x, const int y, const int new_width, const int new_height, const Bitmap & who );
	virtual void drawRotate( const int x, const int y, const int angle, const Bitmap & where );

	virtual void Stretch( const Bitmap & where );
	virtual void Stretch( const Bitmap & where, const int sourceX, const int sourceY, const int sourceWidth, const int sourceHeight, const int destX, const int destY, const int destWidth, const int destHeight );
	virtual void StretchBy2( const Bitmap & where );
	virtual void StretchBy4( const Bitmap & where );
	virtual void Blit( const string & xpath ) const;
	virtual void Blit( const Bitmap & where ) const;
	virtual void Blit( const int x, const int y, const Bitmap & where ) const;
	virtual void Blit( const int mx, const int my, const int wx, const int wy, const Bitmap & where ) const;
	virtual void Blit( const int mx, const int my, const int width, const int height, const int wx, const int wy, Bitmap & where ) const;
	virtual void BlitToScreen() const;
	virtual void fill( int color ) const;

	inline void clear() const{
		this->fill( 0 );
	}

	bool getError();

	inline BITMAP * getBitmap() const{
		return _my_bitmap;
	}

	virtual void readLine( vector< int > & vec, int y );
	const int getPixel( const int x, const int y ) const;

	void putPixel( int x, int y, int col ) const;

	/*
	inline int getPixel( int x, int y ) const{
		if ( x >= 0 && x < my_bitmap->w && y >= 0 && y <= my_bitmap->h )
			return _getpixel16( my_bitmap, x, y );
		return -1;
	}
	
	inline void putPixel( int x, int y, int col ) const{
		if ( x >= 0 && x < my_bitmap->w && y >= 0 && y <= my_bitmap->h )
			_putpixel16( my_bitmap, x, y, col );
	}
	*/
	
	void setClipRect( int x1, int y1, int x2, int y2 ) const;

	inline const string & getPath() const{
		return path;
	}

	static int setGfxModeText();
	static int setGfxModeFullscreen( int x, int y );
	static int setGfxModeWindowed( int x, int y );

	static int makeColor( int r, int g, int b );
	static int darken( int color, double factor );
	static void hsvToRGB( float h, float s, float v, int * r, int * g, int * b );

	static int getRed( int x );
	static int getBlue( int x );
	static int getGreen( int x );

	/* Add two colors together
	 */
	static int addColor( int color1, int color2 );

	/*
	inline static int makeColor( int r, int g, int b ){
		return makecol16( r, g, b );
	}
	*/

	// static const int MaskColor = MASK_COLOR_16;
	static const int MaskColor;
	static const int MODE_TRANS;
	static const int MODE_SOLID;

	static const int SPRITE_NO_FLIP;
	static const int SPRITE_V_FLIP;
	static const int SPRITE_H_FLIP;

	static const int SPRITE_NORMAL;
	static const int SPRITE_LIT;
	static const int SPRITE_TRANS;

protected:

	void releaseInternalBitmap();

	inline void setBitmap( BITMAP * bitmap ){
		if ( bitmap == NULL ){
			cout << "*FATAL* Setting null bitmap" << endl;
		}
		_my_bitmap = bitmap;
	}

	void internalLoadFile( const char * load_file );

	BITMAP * _my_bitmap;
	int * own;
	// bool own;
	bool error;
	string path;

};

#endif
