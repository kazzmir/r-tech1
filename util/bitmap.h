/* if you include this file then it must be the first line in file (after any
 * #ifdef's of course. windows will complain loudly if <windows.h> ends up being
 * included before allegro.h and this file fixes things so the order is correct.
 */
#ifndef _paintown_bitmap_h_
#define _paintown_bitmap_h_

#ifdef USE_ALLEGRO
#include "allegro/bitmap.h"
#endif
#ifdef USE_SDL
#include "sdl/bitmap.h"
#endif

#include <string>
#include <vector>
#include <iostream>

class TranslucentBitmap;

class Bitmap{
private:
	static Bitmap * Screen;
	
        /* these constructors don't really matter, get rid of them at some point */
        Bitmap( const Bitmap & copy, int sx, int sy, double accuracy );
	Bitmap( const char * load_file, int sx, int sy, double accuracy );
public:

        static int SCALE_X;
        static int SCALE_Y;
	
	/* default constructor makes 10x10 bitmap */
	Bitmap();
	Bitmap( int x, int y );
	Bitmap( const char * load_file );
	Bitmap( const std::string & load_file );
	Bitmap( const char * load_file, int sx, int sy );

        /* 4/24/2010: remove this at some point */
#ifdef USE_ALLEGRO
	explicit Bitmap( BITMAP * who, bool deep_copy = false );
#endif
#ifdef USE_SDL
	explicit Bitmap(SDL_Surface * who, bool deep_copy = false );
#endif
	Bitmap( const Bitmap & copy, bool deep_copy = false );
	Bitmap( const Bitmap & copy, int sx, int sy );
	Bitmap( const Bitmap & copy, int x, int y, int width, int height );
	virtual ~Bitmap();

        virtual TranslucentBitmap translucent() const;

	virtual void save( const std::string & str );

	Bitmap & operator=( const Bitmap & );

	virtual void load( const std::string & str );

	int getWidth() const;
	int getHeight() const;

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

        static Bitmap memoryPCX(unsigned char * const data, const int length, const bool mask = true);

	void detach();

	static void transBlender( int r, int g, int b, int a );
	static void multiplyBlender( int r, int g, int b, int a );
	static void dissolveBlender( int r, int g, int b, int a );
	static void addBlender( int r, int g, int b, int a );
	static void burnBlender( int r, int g, int b, int a );
	static void colorBlender( int r, int g, int b, int a );
	static void differenceBlender( int r, int g, int b, int a );
	static void dodgeBlender( int r, int g, int b, int a );
	static void hueBlender( int r, int g, int b, int a );
	static void luminanceBlender( int r, int g, int b, int a );
	static void invertBlender( int r, int g, int b, int a );
	static void screenBlender( int r, int g, int b, int a );
	static int setGraphicsMode( int mode, int width, int height );

	static void drawingMode( int type );

	void acquire();
	void release();

	void resize( const int width, const int height );
	
	void debugSelf() const;

        /* convert to a grey scale version */
        virtual Bitmap greyScale();

	virtual void triangle( int x1, int y1, int x2, int y2, int x3, int y3, int color ) const;
	virtual void ellipse( int x, int y, int rx, int ry, int color ) const;
	virtual void ellipseFill( int x, int y, int rx, int ry, int color ) const;

        virtual void light(int x, int y, int width, int height, int start_y, int focus_alpha, int edge_alpha, int focus_color, int edge_color) const;
        virtual void applyTrans(const int color);

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

	virtual void draw(const int x, const int y, const Bitmap & where) const;
	virtual void draw(const int x, const int y, const int startWidth, const int startHeight, const int width, const int height, const Bitmap & where) const;
	virtual void drawCharacter( const int x, const int y, const int color, const int background, const Bitmap & where ) const;
        /* to draw lit use LitBitmap */
	// virtual void drawLit( const int x, const int y, const int level, const Bitmap & where ) const;
	virtual void drawHFlip(const int x, const int y, const Bitmap & where) const;
	virtual void drawHFlip(const int x, const int y, const int startWidth, const int startHeight, const int width, const int height, const Bitmap & where) const;
	virtual void drawVFlip( const int x, const int y, const Bitmap & where ) const;
	virtual void drawHVFlip( const int x, const int y, const Bitmap & where ) const;
	virtual void drawTrans( const int x, const int y, const Bitmap & where ) const;
	virtual void drawTransHFlip( const int x, const int y, const Bitmap & where ) const;
	virtual void drawTransVFlip( const int x, const int y, const Bitmap & where ) const;
	virtual void drawTransHVFlip( const int x, const int y, const Bitmap & where ) const;
	virtual void drawMask( const int x, const int y, const Bitmap & where );
	virtual void drawStretched( const int x, const int y, const int new_width, const int new_height, const Bitmap & who );
	virtual void drawRotate( const int x, const int y, const int angle, const Bitmap & where );
	virtual void drawPivot( const int centerX, const int centerY, const int x, const int y, const int angle, const Bitmap & where );
	virtual void drawPivot( const int centerX, const int centerY, const int x, const int y, const int angle, const double scale, const Bitmap & where );

	virtual void Stretch( const Bitmap & where ) const;
	virtual void Stretch( const Bitmap & where, const int sourceX, const int sourceY, const int sourceWidth, const int sourceHeight, const int destX, const int destY, const int destWidth, const int destHeight ) const;
	virtual void StretchBy2( const Bitmap & where );
	virtual void StretchBy4( const Bitmap & where );
	virtual void Blit( const std::string & xpath ) const;
	virtual void Blit( const Bitmap & where ) const;
	virtual void Blit( const int x, const int y, const Bitmap & where ) const;
	virtual void Blit( const int mx, const int my, const int wx, const int wy, const Bitmap & where ) const;
	virtual void Blit( const int mx, const int my, const int width, const int height, const int wx, const int wy, const Bitmap & where ) const;
        virtual void BlitMasked( const int mx, const int my, const int width, const int height, const int wx, const int wy, const Bitmap & where ) const;
	virtual void BlitToScreen() const;
	virtual void BlitAreaToScreen(const int upper_left_x, const int upper_left_y) const;
	virtual void BlitToScreen(const int upper_left_x, const int upper_left_y) const;
        virtual void BlitFromScreen(const int x, const int y) const;

        /* try to call Global::getScreenWidth/Height() instead of these directly */
        static int getScreenWidth();
        static int getScreenHeight();

	virtual void fill( int color ) const;

	inline void clear() const{
		this->fill( 0 );
	}
	
        inline void clearToMask() const{
		this->fill(MaskColor());
	}

	bool getError();

	inline const BitmapData & getData() const {
            return data;
	}
	
        inline BitmapData & getData(){
            return data;
	}
	
	virtual void readLine( std::vector< int > & vec, int y );
	int getPixel( const int x, const int y ) const;

        /* uses _putpixel16 underneath which ignores translucent behavior */
	void putPixel( int x, int y, int col ) const;
        /* respects the current trans mode */
	virtual void putPixelNormal(int x, int y, int col) const;

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

	inline const std::string & getPath() const{
		return path;
	}

        /* produce a temporary bitmap that is not guaranteed to be preserved
         * after your function returns. do *not* hold references to this bitmap
         * and if you make a temporary bitmap, do *not* call functions that
         * might also make temporary bitmaps.
         */
        static Bitmap temporaryBitmap(int w, int h);

        /* call this method to delete all temporary bitmaps.
         * don't call this unless you know what you are doing
         */
        static void cleanupTemporaryBitmaps();

        static double getScale();
	static int setGfxModeText();
	static int setGfxModeFullscreen( int x, int y );
	static int setGfxModeWindowed( int x, int y );

	static int makeColor( int r, int g, int b );
	static int darken( int color, double factor );
	static void hsvToRGB( float h, float s, float v, int * r, int * g, int * b );

        /*
         * Convert color values between the HSV and RGB color spaces. The RGB values
         * range from 0 to 255, hue is from 0 to 360, and saturation and value are
         * from 0 to 1.
         */
        static void rgbToHSV(int r, int g, int b, float * h, float * s, float * v);

        /* convert cymk to rgb. values should be in the range 0-255 */
        static void cymkToRGB(int c, int y, int m, int k, int * r, int * g, int * b);

	static int getRed( int x );
	static int getBlue( int x );
	static int getGreen( int x );

	/* Add two RGB16 colors together
         *  r = c1.r + c2.r
         *  g = c1.g + c2.g
         *  b = c1.b + c2.b
	 */
	static int addColor( int color1, int color2 );

	/*
	inline static int makeColor( int r, int g, int b ){
		return makecol16( r, g, b );
	}
	*/

	// static const int MaskColor = MASK_COLOR_16;
	static int MaskColor();
	static const int MODE_TRANS;
	static const int MODE_SOLID;

	static const int SPRITE_NO_FLIP;
	static const int SPRITE_V_FLIP;
	static const int SPRITE_H_FLIP;

	static const int SPRITE_NORMAL;
	static const int SPRITE_LIT;
	static const int SPRITE_TRANS;

protected:

        /* release a reference count, and possibly destroy data */
        void releaseInternalBitmap();

        /* really destroy private data */
        void destroyPrivateData();

        /*
        inline void setBitmap( BITMAP * bitmap ){
            if ( bitmap == NULL ){
                std::cout << "*FATAL* Setting null bitmap" << std::endl;
            }
            _my_bitmap = bitmap;
        }
        */

        void internalLoadFile( const char * load_file );

        /* implementation specific data */
        BitmapData data;
        int * own;
        // bool own;
        bool error;
        std::string path;
        static Bitmap * temporary_bitmap;
};

#endif
