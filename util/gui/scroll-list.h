#ifndef _gui_scroll_list_h
#define _gui_scroll_list_h

#include <string>
#include <vector>

#include "coordinate.h"

#include "../gradient.h"

class Font;

namespace Gui{

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

    //! Add text
    virtual void addText(const std::string &);

    //! Add vector of text (replaces current text in the text vector)
    virtual void addText(const std::vector< std::string > &);

    //! Next
    virtual bool next();

    //! Previous
    virtual bool previous();

    //! Get current index
    virtual inline unsigned int getCurrentIndex(){
        return this->currentIndex;
    }

private:

    //! Text
    std::vector<std::string> text;

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
};

}

#endif
