#ifndef _paintown_bitmap_h_
#define _paintown_bitmap_h_

#include "exceptions/exception.h"
#include "pointer.h"
#include "parameter.h"
#include <string>
#include <vector>
#include <iostream>

#ifdef USE_ALLEGRO
#include "allegro/bitmap.h"
#endif
#ifdef USE_SDL
#include "sdl/bitmap.h"
#endif
#ifdef USE_ALLEGRO5
#include "allegro5/bitmap.h"
#endif

namespace Storage{
    class File;
}

namespace Graphics{

enum ImageFormat{
    FormatPNG,
    FormatBMP,
    FormatJPG,
    FormatPCX,
    FormatTGA,
    FormatTIF,
    FormatXPM,
    FormatGIF,
    FormatUnknown
};

ImageFormat identifyImage(const char * data, int length);

class TranslucentBitmap;
class LitBitmap;
class Bitmap;

extern Util::Parameter<Bitmap*> screenParameter;

extern const int SPRITE_NORMAL;
extern const int SPRITE_LIT;
extern const int SPRITE_TRANS;

extern const int SPRITE_NO_FLIP;
extern const int SPRITE_V_FLIP;
extern const int SPRITE_H_FLIP;
        
/*
extern int SCALE_X;
extern int SCALE_Y;
*/

class BitmapException: public Exception::Base {
public:
    BitmapException(const std::string & file, int line, const std::string & reason):
    Base(file, line),
    reason(reason){
    }

    BitmapException(const BitmapException & copy):
    Base(copy),
    reason(copy.reason){
    }

    virtual void throwSelf() const {
        throw *this;
    }

    virtual ~BitmapException() throw () {
    }
    
protected:
    virtual const std::string getReason() const {
        return reason;
    }

    std::string reason;
};

/* create a color from components */
Color makeColor(int r, int g, int b);
Color darken(Color color, double factor);
void hsvToRGB( float h, float s, float v, int * r, int * g, int * b );

int setGfxModeText();
int setGfxModeFullscreen( int x, int y );
int setGfxModeWindowed( int x, int y );
int setGraphicsMode(int mode, int width, int height);

/* get color components */
int getRed(Color x);
int getBlue(Color x);
int getGreen(Color x);

Color MaskColor();

enum QualityFilter{
    NoFilter,
    HqxFilter,
    XbrFilter
};

QualityFilter qualityFilterName(const std::string & type);

class Bitmap{
private:
	
        /* these constructors don't really matter, get rid of them at some point */
        Bitmap( const Bitmap & copy, int sx, int sy, double accuracy );
	Bitmap( const char * load_file, int sx, int sy, double accuracy );
public:

        /* equivalent to a GPU shader */
        class Filter{
        public:
            virtual Color filter(Color pixel) const = 0;
            virtual ~Filter(){
            }
        };
        	
	/* default constructor makes 10x10 bitmap */
	Bitmap();
	Bitmap( int x, int y );
	Bitmap( const char * load_file );
        Bitmap(const char * data, int length);
	Bitmap( const std::string & load_file );
	Bitmap( const char * load_file, int sx, int sy );

        /* Load a bitmap from an abstract file */
        Bitmap(Storage::File & file);

        /* 4/24/2010: remove this at some point */
#ifdef USE_ALLEGRO
	explicit Bitmap( BITMAP * who, bool deep_copy = false );
#endif
#ifdef USE_SDL
	explicit Bitmap(SDL_Surface * who, bool deep_copy = false );
#endif
#ifdef USE_ALLEGRO5
	explicit Bitmap(ALLEGRO_BITMAP * who, bool deep_copy = false );
#endif
	Bitmap( const Bitmap & copy, bool deep_copy = false );
	Bitmap( const Bitmap & copy, int sx, int sy );
	Bitmap( const Bitmap & copy, int x, int y, int width, int height );
	virtual ~Bitmap();

        virtual TranslucentBitmap translucent() const;
        /* will call transBlender() with the supplied values for you */
        virtual TranslucentBitmap translucent(int red, int green, int blue, int alpha) const;
        virtual LitBitmap lit() const;

	virtual void save( const std::string & str ) const;

	Bitmap & operator=( const Bitmap & );

	virtual void load( const std::string & str );

	virtual int getWidth() const;
	virtual int getHeight() const;

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

        /* replace all pixels that have value `original' with `replaced' */
        void replaceColor(const Color & original, const Color & replaced);
        void set8BitMaskColor(const Color & color);
        Color get8BitMaskColor();

        /* Blend between source pixel and destination pixel.
         * Source and dest should be 0-255.
         * source = 64, dest = 128
         * pixel = source_pixel * source / 255 + dest_pixel * dest / 255
         */
	static void alphaBlender(int source, int dest);
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
        /* for testing */
	static void setFakeGraphicsMode(int width, int height);

        /* clean up any remaining state */
        static void shutdown();

	// static void drawingMode( int type );

        virtual Color blendColor(const Color & input) const;
        /* sets the blend state. try not to use these function unless
         * absolutely necessary.
         */
        virtual void startDrawing() const;
        virtual void endDrawing() const;

	void acquire();
	void release();

        void updateOnResize();

        /* Resizes the bitmap and stretches the old pixels to the new bitmap */
	void resize(const int width, const int height);

        /* Just resizes the bitmap without copying pixels */
        void updateSize(const int width, const int height);

        /* Makes this bitmap the same as the parameter.
         * Width/height is copied and then a Blit is called.
         */
        void copy(const Bitmap & him);

	void debugSelf() const;

        /* convert to a grey scale version */
        virtual Bitmap greyScale();

	virtual void triangle( int x1, int y1, int x2, int y2, int x3, int y3, Color color ) const;

        /* draws an equilateral triangle centered at (x,y) pointing at `angle'
         * where each side has `size' pixels using the color.
         */
        virtual void equilateralTriangle(int x, int y, int angle, int size, Color color) const;
	virtual void ellipse( int x, int y, int rx, int ry, Color color ) const;
	virtual void ellipseFill( int x, int y, int rx, int ry, Color color ) const;

        virtual void light(int x, int y, int width, int height, int start_y, int focus_alpha, int edge_alpha, int focus_color, Color edge_color) const;
        virtual void applyTrans(const Color color) const;

	virtual void border( int min, int max, Color color ) const;
	virtual void rectangle( int x1, int y1, int x2, int y2, Color color ) const;
	virtual void rectangleFill( int x1, int y1, int x2, int y2, Color color ) const;
	virtual void circleFill( int x, int y, int radius, Color color ) const;
	virtual void circle( int x, int y, int radius, Color color ) const;
	virtual void circle(int x, int y, int radius, int thickness, Color color) const;
	virtual void line( const int x1, const int y1, const int x2, const int y2, const Color color ) const;
		
	virtual void floodfill( const int x, const int y, const Color color ) const;
	virtual void horizontalLine( const int x1, const int y, const int x2, const Color color ) const;
	virtual void hLine( const int x1, const int y, const int x2, const Color color ) const;
	virtual void vLine( const int y1, const int x, const int y2, const Color color ) const;
	virtual void polygon( const int * verts, const int nverts, const Color color ) const;
	virtual void arc(const int x, const int y, const double ang1, const double ang2, const int radius, const Color color ) const;
	virtual void arcFilled(const int x, const int y, const double ang1, const double ang2, const int radius, const Color color ) const;

        /* x/y is in the center of the bitmap */
	virtual void drawCenter(const int x, const int y, const Bitmap & where) const;

	virtual void draw(const int x, const int y, const Bitmap & where) const;
	virtual void draw(const int x, const int y, Filter * filter, const Bitmap & where) const;
	virtual void draw(const int x, const int y, const int startWidth, const int startHeight, const int width, const int height, const Bitmap & where) const;
	virtual void draw(const int x, const int y, const int startWidth, const int startHeight, const int width, const int height, Filter * filter, const Bitmap & where) const;
	virtual void drawCharacter( const int x, const int y, const int color, const int background, const Bitmap & where ) const;

        /* flip horizontally */
	virtual void drawHFlip(const int x, const int y, const Bitmap & where) const;
	virtual void drawHFlip(const int x, const int y, Filter * filter, const Bitmap & where) const;
	virtual void drawHFlip(const int x, const int y, const int startWidth, const int startHeight, const int width, const int height, const Bitmap & where) const;
	virtual void drawHFlip(const int x, const int y, const int startWidth, const int startHeight, const int width, const int height, Filter * filter, const Bitmap & where) const;
        /* flip vertically */
	virtual void drawVFlip( const int x, const int y, const Bitmap & where ) const;
	virtual void drawVFlip( const int x, const int y, Filter * filter, const Bitmap & where ) const;

        /* flip horizontal and vertical */
	virtual void drawHVFlip( const int x, const int y, const Bitmap & where ) const;
	virtual void drawHVFlip( const int x, const int y, Filter * filter, const Bitmap & where ) const;
	// virtual void drawTrans( const int x, const int y, const Bitmap & where ) const;
	// virtual void drawTrans( const int x, const int y, Filter * filter, const Bitmap & where ) const;
	// virtual void drawTransHFlip( const int x, const int y, const Bitmap & where ) const;
	// virtual void drawTransHFlip( const int x, const int y, Filter * filter, const Bitmap & where ) const;
	// virtual void drawTransVFlip( const int x, const int y, const Bitmap & where ) const;
	// virtual void drawTransVFlip( const int x, const int y, Filter * filter, const Bitmap & where ) const;
	// virtual void drawTransHVFlip( const int x, const int y, const Bitmap & where ) const;
	// virtual void drawTransHVFlip( const int x, const int y, Filter * filter, const Bitmap & where ) const;
	virtual void drawMask( const int x, const int y, const Bitmap & where );
	virtual void drawStretched( const int x, const int y, const int new_width, const int new_height, const Bitmap & who ) const;
	virtual void drawStretched(const Bitmap & who) const;

        /* middle of the bitmap is at x, y */
	virtual void drawRotateCenter(const int x, const int y, const int angle, const Bitmap & where);
        /* upper left hand corner is at x, y*/
	virtual void drawRotate(const int x, const int y, const int angle, const Bitmap & where);
	virtual void drawPivot( const int centerX, const int centerY, const int x, const int y, const int angle, const Bitmap & where );
	virtual void drawPivot( const int centerX, const int centerY, const int x, const int y, const int angle, const double scale, const Bitmap & where );

        /* Stretch using the hqx algorithm */
        virtual void StretchHqx(const Bitmap & where) const;
	virtual void StretchHqx(const Bitmap & where, const int sourceX, const int sourceY, const int sourceWidth, const int sourceHeight, const int destX, const int destY, const int destWidth, const int destHeight) const;
        
        /* Xbr is defined here:
         * http://board.byuu.org/viewtopic.php?f=10&t=2248
         */
        virtual void StretchXbr(const Bitmap & where) const;
	virtual void StretchXbr(const Bitmap & where, const int sourceX, const int sourceY, const int sourceWidth, const int sourceHeight, const int destX, const int destY, const int destWidth, const int destHeight) const;

        /* Just copies pixels (fast) */
	virtual void Stretch(const Bitmap & where) const;
	virtual void Stretch(const Bitmap & where, const int sourceX, const int sourceY, const int sourceWidth, const int sourceHeight, const int destX, const int destY, const int destWidth, const int destHeight ) const;
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

        void roundRect(int radius, int x1, int y1, int x2, int y2, Color color) const;
        void roundRectFill(int radius, int x1, int y1, int x2, int y2, Graphics::Color color) const;

        /* returned a scaled version of this bitmap of the given dimensions */
        virtual Bitmap scaleTo(const int width, const int height) const;

        /* scale the image by the given ratios, i.e: a ratio of 1 means do nothing.
         * less than 1 will make it smaller and larger than 1 will make it larger.
         */
        virtual Bitmap scaleBy(const double widthRatio, const double heightRatio) const;

        /* try to call Global::getScreenWidth/Height() instead of these directly */
        static int getScreenWidth();
        static int getScreenHeight();

	virtual void fill(Color color) const;

	virtual void clear() const;
	
        inline void clearToMask() const{
		this->fill(MaskColor());
	}

	bool getError();

	inline const Util::ReferenceCount<BitmapData> & getData() const {
            return data;
	}
	
        inline Util::ReferenceCount<BitmapData> getData(){
            return data;
	}

        void setData(Util::ReferenceCount<BitmapData> data){
            this->data = data;
        }
	
	virtual void readLine( std::vector<Color> & vec, int y );
	Color getPixel( const int x, const int y ) const;

        /* lock video bitmap to memory */
        void lock() const;
        void lock(int x, int y, int width, int height) const;
        void unlock() const;

        /* true if the point is within the bounds of the bitmap */
        bool inRange(int x, int y) const;

        /* uses _putpixel16 underneath which ignores translucent behavior */
	void putPixel( int x, int y, Color col ) const;
        /* respects the current trans mode */
	virtual void putPixelNormal(int x, int y, Color col) const;

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
	void getClipRect( int & x1, int & y1, int & x2, int & y2 ) const;

	inline const std::string & getPath() const{
		return path;
	}

        /* produce a temporary bitmap that is not guaranteed to be preserved
         * after your function returns. do *not* hold references to this bitmap
         * and if you make a temporary bitmap, do *not* call functions that
         * might also make temporary bitmaps.
         */
        static Bitmap temporaryBitmap(int w, int h);
        static Bitmap temporaryBitmap2(int w, int h);

        /* call this method to delete all temporary bitmaps.
         * don't call this unless you know what you are doing
         */
        static void cleanupTemporaryBitmaps();

        static double getScale();

	
        /*
         * Convert color values between the HSV and RGB color spaces. The RGB values
         * range from 0 to 255, hue is from 0 to 360, and saturation and value are
         * from 0 to 1.
         */
        static void rgbToHSV(int r, int g, int b, float * h, float * s, float * v);

        /* convert cymk to rgb. values should be in the range 0-255 */
        static void cymkToRGB(int c, int y, int m, int k, int * r, int * g, int * b);

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
        /*
	static const int MODE_TRANS;
	static const int MODE_SOLID;
        */

#ifdef USE_ALLEGRO5
        void convertToVideo();
#endif
protected:
        /* release a reference count, and possibly destroy data */
        // void releaseInternalBitmap();

        /* really destroy private data */
        // void destroyPrivateData();

        /*
        inline void setBitmap( BITMAP * bitmap ){
            if ( bitmap == NULL ){
                std::cout << "*FATAL* Setting null bitmap" << std::endl;
            }
            _my_bitmap = bitmap;
        }
        */

        void loadFromMemory(const char * data, int length);

        void internalLoadFile( const char * load_file );

        /* implementation specific data */
        Util::ReferenceCount<BitmapData> data;
        // int * own;
        bool mustResize;
        // bool own;
        bool error;
        std::string path;
        static Bitmap * temporary_bitmap;
        static Bitmap * temporary_bitmap2;
        Color bit8MaskColor;

        /* only used by allegro5 for now */
        int width, height;
};

/* 8-bit bitmap that supports a palette */
class PaletteBitmap: public Bitmap {
public:
    PaletteBitmap();
    virtual ~PaletteBitmap();

protected:
    Color palette[256];
};

Bitmap memoryPCX(unsigned char * const data, const int length, const bool mask = true);

/* creates a bitmap that can be used as a buffer for the screen.
 * on opengl/allegro5 systems this will return the current backbuffer
 */
Bitmap * getScreenBuffer();

void blend_palette(Color * pal, int mp, Color sc, Color ec);
// bool sameColors(Color color1, Color color2);

}

#endif
