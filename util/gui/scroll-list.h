#ifndef _gui_scroll_list_h
#define _gui_scroll_list_h

#include <string>
#include <vector>

#include "coordinate.h"

#include "../pointer.h"

class Font;

namespace Gui{

/* pure virtual class */
class ScrollItem{
public:
    ScrollItem();

    /* distance is how far from the selected item we are. 0 means we are the selected item.
     * distance can be positive or negative.
     */
    virtual void draw(int x, int y, const Graphics::Bitmap & where, const Font & font, int distance) const = 0;

    /* size in pixels, used for justification */
    virtual int size(const Font & font) const = 0;

    virtual ~ScrollItem();
};

class ScrollList {
public:
    ScrollList();

    ScrollList(const ScrollList &);

    virtual ~ScrollList();

    //! copy
    ScrollList & operator=(const ScrollList &);

    //! Logic
    virtual void act();

    //! Render
    virtual void render(const Graphics::Bitmap &, const Font & font);

    //! Add item
    virtual void addItem(const Util::ReferenceCount<ScrollItem> & item);

    //! Add vector of text
    virtual void addItems(const std::vector<Util::ReferenceCount<ScrollItem> > & items);
    
    //! Set Position
    virtual void setPosition(const Gui::Coordinate &);
    
    //! Set font spacing
    inline virtual void setFontSpacing(int x, int y){
        this->fontSpacingX = x;
        this->fontSpacingY = y;
    }
    
    //! Set font spacing X
    inline virtual void setFontSpacingX(int x){
        this->fontSpacingX = x;
    }
    
    //! Set font spacing X
    inline virtual void setFontSpacingY(int y){
        this->fontSpacingY = y;
    }

    //! Next
    virtual bool next();

    //! Previous
    virtual bool previous();

    //! Set current index
    virtual bool setCurrentIndex(unsigned int index);
    
    //! Get current index
    virtual inline unsigned int getCurrentIndex() const {
        return this->currentIndex;
    }

    /*
    //! Set gradient
    virtual inline void setGradient(bool use){
	this->useGradient = use;
    }
    
    //! Get gradient
    virtual inline bool gradientActive() const {
	return this->useGradient;
    }
    */
    virtual inline double getScrollMotion() const {
        return scrollMotion;
    }

    virtual inline void setScrollMotion(double what){
        if (what < 1.01){
            what = 1.01;
        }
        scrollMotion = what;
    }
    
    //! Set highlight
    virtual inline void setHighlight(bool use){
        this->useHighlight = use;
    }
    
    //! Get highlight
    virtual inline bool highlightActive() const {
        return this->useHighlight;
    }
    
    //! Set wrap
    virtual inline void setWrap(bool wrap){
        this->allowWrap = wrap;
    }
    
    //! Get wrap
    virtual inline bool getWrap() const {
        return this->allowWrap;
    }
    
private:
    /* smooth drawing */
    void doDraw(int x, int y, int middle_x, int min_y, int max_y, const Font & font, int current, int selected, const Graphics::Bitmap & area, int direction);

    //! Text
    std::vector<Util::ReferenceCount<ScrollItem> > text;

    //! Current index
    unsigned int currentIndex;

    //! Coordinates (size of)
    Gui::Coordinate position;

    //! Font Spacing
    int fontSpacingX, fontSpacingY;

    //! Current position for smooth scrolling
    int currentPosition;

    //! Scroll wait
    int scrollWait;

    /* speed at which the menu scrolls */
    double scrollMotion;

    //! Gradient for selected cursor
    // Effects::Gradient selectedGradient;

    //! Use gradient
    // bool useGradient;

    //! Use highlight
    bool useHighlight;
    
    //! Is wrappable
    bool allowWrap;

    /* how much to scroll by */
    double scroll;
};

}

#endif
