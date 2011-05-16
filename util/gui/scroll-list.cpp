#include "../bitmap.h"
#include "scroll-list.h"
#include "../font.h"

using namespace Gui;

static const int GradientMax = 50;

static int selectedGradientStart(){
    static int color = Graphics::makeColor(19, 167, 168);
    return color;
}

static int selectedGradientEnd(){
    static int color = Graphics::makeColor(27, 237, 239);
    return color;
}

ScrollList::ScrollList():
currentIndex(0),
fontSpacingX(0),
fontSpacingY(0),
currentPosition(0),
scrollWait(0),
selectedGradient(GradientMax, selectedGradientStart(), selectedGradientEnd()),
useGradient(false),
useHighlight(false),
allowWrap(true){}

ScrollList::ScrollList(const ScrollList & copy):
currentIndex(copy.currentIndex),
fontSpacingX(copy.fontSpacingX),
fontSpacingY(copy.fontSpacingY),
currentPosition(copy.currentPosition),
scrollWait(copy.scrollWait),
selectedGradient(GradientMax, selectedGradientStart(), selectedGradientEnd()),
useGradient(copy.useGradient),
useHighlight(copy.useHighlight),
allowWrap(true){}

ScrollList::~ScrollList(){}

ScrollList & ScrollList::operator=(const ScrollList & copy){
    return *this;
}

void ScrollList::act(){
}

void ScrollList::render(const Graphics::Bitmap &, const Font & font){
}

void ScrollList::addText(const std::string & text){
    this->text.push_back(text);
}

void ScrollList::addText(const std::vector<std::string> & texts){
    this->text.insert(text.end(), texts.begin(), texts.end());
}

void ScrollList::setPosition(const Gui::Coordinate & location){
    this->position = location;
}

bool ScrollList::next(){
    currentIndex++;
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
