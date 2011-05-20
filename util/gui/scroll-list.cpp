#include "../bitmap.h"
#include "../trans-bitmap.h"
#include "scroll-list.h"
#include "../font.h"
#include <math.h>
#include "../debug.h"

namespace Gui{

static const double FONT_SPACER = 1.3;
// static const int GradientMax = 50;

const double EPSILON = 0.01;
const int SCROLL_WAIT = 4;

/*
static int selectedGradientStart(){
    static int color = Graphics::makeColor(19, 167, 168);
    return color;
}

static int selectedGradientEnd(){
    static int color = Graphics::makeColor(27, 237, 239);
    return color;
}
*/
    
ScrollItem::ScrollItem(){
}

ScrollItem::~ScrollItem(){
}

ScrollList::ScrollList():
currentIndex(0),
fontSpacingX(0),
fontSpacingY(0),
currentPosition(0),
scrollWait(0),
scrollMotion(1.2),
// selectedGradient(GradientMax, selectedGradientStart(), selectedGradientEnd()),
// useGradient(false),
useHighlight(false),
allowWrap(true),
scroll(0),
justification(CenterJustify){}

ScrollList::ScrollList(const ScrollList & copy):
currentIndex(copy.currentIndex),
fontSpacingX(copy.fontSpacingX),
fontSpacingY(copy.fontSpacingY),
currentPosition(copy.currentPosition),
scrollWait(copy.scrollWait),
// selectedGradient(GradientMax, selectedGradientStart(), selectedGradientEnd()),
// useGradient(copy.useGradient),
useHighlight(copy.useHighlight),
allowWrap(true),
scroll(0){}

ScrollList::~ScrollList(){}

ScrollList & ScrollList::operator=(const ScrollList & copy){
    return *this;
}

void ScrollList::act(){
    if (scrollWait == 0){
        if (scroll > EPSILON){
            // scroll -= SCROLL_MOTION;
            scroll /= scrollMotion;
        } else if (scroll < -EPSILON){
            // scroll += SCROLL_MOTION;
            scroll /= scrollMotion;
        } else {
            scroll = 0;
            currentPosition = currentIndex;
        }
    } else {
        scrollWait -= 1;
    }

    /*
    if (scrollWait > 0){
        scrollWait -= 1;
    } else {
        currentPosition = currentIndex;
    }
    */
}

int ScrollList::justify(int left, int right, int size) const {
    switch (justification){
        case LeftJustify: return left;
        case RightJustify: return right - size;
        case CenterJustify: return (left + right) / 2 - size / 2;
    }
    return 0;
}

/* this is the smooth scroll stuff from context-box */
void ScrollList::doDraw(int x, int y, int min_y, int max_y, const Font & font, int current, int selected, const Graphics::Bitmap & area, int direction) const {
    while (y < max_y && y > min_y){
        /* circuluar */
        int pick = current;
        while (pick < 0){
            pick += text.size();
        }
        pick = pick % text.size();

        Util::ReferenceCount<ScrollItem> option = text[pick];
        /* center justification */
        const int startx = justify(1, area.getWidth() - 1, option->size(font));

        /* the selected option will have a distance of 0 */
        int distance = current - selected;
        option->draw(x + startx, y, area, font, distance);

        if (text.size() == 1){
            return;
        }

        current += direction;
        y += direction * font.getHeight() / FONT_SPACER;
    }
}

void ScrollList::render(const Graphics::Bitmap & where, const Font & font) const {
    /* middle of the bitmap offset by the scroll amount. */
    int y = where.getHeight() / 2 + scroll * font.getHeight() / 2 - font.getHeight() / 2;

    /* allow options to be drawn a little off the bitmap */
    int min_y = 0 - font.getHeight() / FONT_SPACER;
    int max_y = where.getHeight();

    /* draw down starting from the current selection */
    doDraw(0, y, min_y, max_y, font, currentIndex, currentIndex, where, 1);

    /* then draw up, skipping the current selection */
    doDraw(0, y - font.getHeight() / FONT_SPACER, min_y, max_y, font, currentIndex - 1, currentIndex, where, -1);
}

void ScrollList::addItem(const Util::ReferenceCount<ScrollItem> & text){
    this->text.push_back(text);
}

void ScrollList::addItems(const std::vector<Util::ReferenceCount<ScrollItem> > & texts){
    this->text.insert(text.end(), texts.begin(), texts.end());
}

void ScrollList::setPosition(const Gui::Coordinate & location){
    this->position = location;
}

bool ScrollList::next(){
    /* FIXME: probably if the current index goes past the boundary we shouldn't scroll */
    currentIndex++;
    scroll = 1;
    if (scrollWait == 0){
        scrollWait = SCROLL_WAIT;
    }
    if (currentIndex >= text.size()){
        if (allowWrap){
            currentIndex = 0;
            return true;
        } else {
            currentIndex = text.size()-1;
            return false;
        }
    }
    return true;
}

bool ScrollList::previous(){
    scroll = -1;
    if (scrollWait == 0){
        scrollWait = SCROLL_WAIT;
    }
    if (currentIndex > 0){
        currentIndex--;
    } else if (currentIndex == 0){
        if (allowWrap){
            currentIndex = text.size()-1;
            return true;
        } else {
            return false;
        }
    }
    return true;
}

bool ScrollList::setCurrentIndex(unsigned int index){
    if (index >= text.size()){
        return false;
    }
    currentIndex = index;
    return true;
}

}
