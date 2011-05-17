#ifndef _gui_scroll_list_h
#define _gui_scroll_list_h

#include <string>
#include <vector>

#include "coordinate.h"

#include "../gradient.h"
#include "../pointer.h"

class Font;

namespace Gui{

/* pure virtual class */
class ScrollItem{
public:
    ScrollItem();

    virtual void draw(int x, int y, const Graphics::Bitmap & where, const Font & font) const = 0;

    /* size in pixels, used for justification */
    virtual int size() const = 0;

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

    //! Set gradient
    virtual inline void setGradient(bool use){
	this->useGradient = use;
    }
    
    //! Get gradient
    virtual inline bool gradientActive() const {
	return this->useGradient;
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

    //! Gradient for selected cursor
    Effects::Gradient selectedGradient;

    //! Use gradient
    bool useGradient;

    //! Use highlight
    bool useHighlight;
    
    //! Is wrappable
    bool allowWrap;
};

}

#endif
