#ifndef _paintown_gui_widget_h
#define _paintown_gui_widget_h

#include "rectarea.h"
#include "coordinate.h"

namespace Graphics{
class Bitmap;
}
class Token;
class Font;

namespace Gui{
    
struct ColorInfo{
    ColorInfo():
        body(0),
        /* alpha 0 is invisible, 255 is opaque. set something in the middle as default */
        bodyAlpha(128),
        border(0),
        borderAlpha(128){
        }

    int body;
    int bodyAlpha;
    int border;
    int borderAlpha;
};

/*! Trasformations for widgets 
 *  Eventually this could be expanded and used as something to perform changes on widgets but for now it remains to be just a place holder
 *  ************************
 *  radius - rounded corners
 */
class Transformations{
public:
    Transformations();
    Transformations(const Transformations &);
    virtual ~Transformations();
    
    Transformations & operator=(const Transformations &);
    
    virtual inline void setRadius(double radius){
	this->radius = radius;
    }
    virtual inline double getRadius() const{
	return this->radius;
    }
private:
    double radius;
};

class Widget{
    public:
        Widget();
        Widget( const Widget &);
        virtual ~Widget();
        
        // copy
        Widget &operator=( const Widget &);
        
        void setCoordinates(const Token * token);
        void setColors(const Token * token);
        
        //! New position data
        Coordinate location;
        
        //! Colors
        ColorInfo colors;
	
	//! Transformations
	Transformations transforms;
        
        // Logic
        virtual void act(const Font &)=0;
        
        // Render
        virtual void render(const Graphics::Bitmap &) = 0;
        /* default behavior is just to call render() */
        virtual void render(const Graphics::Bitmap &, const Font &);
    
    protected:
        void arc( const Graphics::Bitmap &, int x, int y, double startAngle, int radius, int color );
        void roundRect( const Graphics::Bitmap &, int radius, int x1, int y1, int x2, int y2, int color );
        void roundRectFill( const Graphics::Bitmap &, int radius, int x1, int y1, int x2, int y2, int color );
        
        void checkWorkArea();
        
        Graphics::Bitmap *workArea;
};

}

#endif
