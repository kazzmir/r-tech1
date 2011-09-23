#include "util/bitmap.h"
#include "util/trans-bitmap.h"

#include "context-box.h"
#include "util/font.h"
#include <math.h>

#include "util/token.h"

static const double FONT_SPACER = 1.3;
static const int GradientMax = 50;

static Graphics::Color selectedGradientStart(){
    static Graphics::Color color = Graphics::makeColor(19, 167, 168);
    return color;
}

static Graphics::Color selectedGradientEnd(){
    static Graphics::Color color = Graphics::makeColor(27, 237, 239);
    return color;
}

using namespace std;

namespace Gui{

Effects::Gradient standardGradient(){
    Effects::Gradient standard(GradientMax, selectedGradientStart(), selectedGradientEnd());
    return standard;
}

Effects::Gradient modifiedGradient(Graphics::Color low, Graphics::Color high){
    Effects::Gradient modified(GradientMax, low, high);
    return modified;
}

ListValues::ListValues():
interpolate(true),
lowColor(selectedGradientStart()),
highColor(selectedGradientEnd()),
selectedColor(selectedGradientStart()),
selectedAlpha(255),
otherColor(Graphics::makeColor(255, 255, 255)),
otherAlpha(255),
fade(true){
}

ListValues::ListValues(const ListValues & copy):
interpolate(copy.interpolate),
lowColor(copy.lowColor),
highColor(copy.highColor),
selectedColor(copy.selectedColor),
selectedAlpha(copy.selectedAlpha),
otherColor(copy.otherColor),
otherAlpha(copy.otherAlpha),
fade(copy.fade){
}

ListValues::~ListValues(){
}

const ListValues & ListValues::operator=(const ListValues & copy){
    interpolate = copy.interpolate;
    lowColor = copy.lowColor;
    highColor = copy.highColor;
    selectedColor = copy.selectedColor;
    selectedAlpha = copy.selectedAlpha;
    otherColor = copy.otherColor;
    otherAlpha = copy.otherAlpha;
    fade = copy.fade;
    
    return *this;
}

static int clamp(int in){
    if (in < 0){
        return 0;
    }
    if (in > 255){
        return 255;
    }
    return in;
}

void ListValues::getValues(const Token * token){
    TokenView view = token->view();
    while (view.hasMore()){
        const Token * token;
        view >> token;
        try{
            int red = 0, green = 0, blue = 0, alpha = 0;
            if (token->match("interpolate-selected", interpolate)){
            } else if (token->match("color-low", red, green, blue)){
                lowColor = Graphics::makeColor(clamp(red), clamp(green), clamp(blue));
            } else if (token->match("color-high", red, green, blue)){
                highColor = Graphics::makeColor(clamp(red), clamp(green), clamp(blue));
            } else if (token->match("selected-color", red, green, blue)){
                selectedColor = Graphics::makeColor(clamp(red), clamp(green), clamp(blue));
            } else if (token->match("selected-color-alpha", alpha)){
                selectedAlpha = clamp(alpha);
            } else if (token->match("other-color", red, green, blue)){
                otherColor = Graphics::makeColor(clamp(red), clamp(green), clamp(blue));
            } else if (token->match("other-color-alpha", alpha)){
                otherAlpha = clamp(alpha);
            } else if (token->match("distance-fade", fade)){
            }
        } catch (const TokenException & ex){
            // Output something
        }
    }
}

ContextItem::ContextItem(const std::string & name, const ContextBox & parent):
text(name),
parent(parent){
}

ContextItem::~ContextItem(){
}

void ContextItem::draw(int x, int y, const Graphics::Bitmap & where, const Font & font, int distance) const {
    if (distance == 0){
        if (parent.getListValues().getInterpolate()){
            Graphics::Bitmap::transBlender(0, 0, 0, parent.getFadeAlpha());
            font.printf(x, y, parent.getSelectedColor(), where.translucent(), getText(), 0);
        } else {
            Graphics::Bitmap::transBlender(0, 0, 0, parent.getListValues().getSelectedAlpha());
            font.printf(x, y, parent.getListValues().getSelectedColor(), where.translucent(), getText(), 0);
        }
    } else {
        if (parent.getListValues().getDistanceFade()){
            int alpha = parent.getFadeAlpha() - fabs((double) distance) * 35;
            if (alpha < 0){
                alpha = 0;
            }
            Graphics::Bitmap::transBlender(0, 0, 0, alpha);
            font.printf(x, y, parent.getListValues().getOtherColor(), where.translucent(), getText(), 0);
        } else {
            Graphics::Bitmap::transBlender(0, 0, 0, parent.getListValues().getOtherAlpha());
            font.printf(x, y, parent.getListValues().getOtherColor(), where.translucent(), getText(), 0);
        }
    }
}
    
void ContextItem::setText(const LanguageString & t){
    text = t;
}

int ContextItem::size(const Font & font) const {
    return font.textLength(getText().c_str());
}

ContextBox::ContextBox():
fadeState(NotActive),
list(new ScrollList()),
/*
fontWidth(0),
fontHeight(0),
*/
fadeSpeed(12),
fadeAlpha(0),
cursorCenter(0),
cursorLocation(0),
scrollWait(4),
selectedGradient(standardGradient()),
useGradient(true),
renderOnlyText(false){
}
ContextBox::ContextBox( const ContextBox & copy ):
fadeState(NotActive),
list(new ScrollList()),
selectedGradient(standardGradient()),
renderOnlyText(false){
    this->list = copy.list;
    // this->context = copy.context;
    /*
    this->font = copy.font;
    this->fontWidth = copy.fontWidth;
    this->fontHeight = copy.fontHeight;
    */
    this->fadeSpeed = copy.fadeSpeed;
    this->fadeAlpha = copy.fadeAlpha;
    this->cursorCenter = copy.cursorCenter;
    this->cursorLocation = copy.cursorLocation;
    this->scrollWait = copy.scrollWait;
    this->useGradient = copy.useGradient;
    this->renderOnlyText = copy.renderOnlyText;
}
ContextBox::~ContextBox(){
}
ContextBox & ContextBox::operator=( const ContextBox & copy){
    this->fadeState = NotActive;
    this->list = copy.list;
    // this->context = copy.context;
    /*
    this->font = copy.font;
    this->fontWidth = copy.fontWidth;
    this->fontHeight = copy.fontHeight;
    */
    this->fadeSpeed = copy.fadeSpeed;
    this->fadeAlpha = copy.fadeAlpha;
    this->cursorCenter = copy.cursorCenter;
    this->cursorLocation = copy.cursorLocation;
    this->scrollWait = copy.scrollWait;
    this->useGradient = copy.useGradient;
    this->renderOnlyText = copy.renderOnlyText;
    return *this;
}

void ContextBox::act(const Font & font){
    // update board
    board.act(font);
    
    // Calculate text info
    // calculateText(font);
    list->act();
    
    // do fade
    doFade();
    
    // Update gradient
    selectedGradient.update();
}

void ContextBox::render(const Graphics::Bitmap & work){
}

void ContextBox::render(const Graphics::Bitmap & work, const Font & font){
    if (!renderOnlyText){
	board.render(work);
    }
    drawText(work, font);
}

bool ContextBox::next(const Font & font){
    if (fadeState == FadeOut){
	return false;
    }

    list->next();

    /*
    cursorLocation += (int)(font.getHeight()/FONT_SPACER);

    if (current < context.size()-1){
        current++;
    } else {
        current = 0;
    }
    */
    return true;
}

bool ContextBox::previous(const Font & font){
    if (fadeState == FadeOut){
	return false;
    }

    list->previous();

    /*
    cursorLocation -= (int)(font.getHeight()/FONT_SPACER);

    if (current > 0){
        current--;
    } else {
        current = context.size()-1;
    }
    */
    return true;
}
        
Graphics::Color ContextBox::getSelectedColor() const {
    return useGradient ? selectedGradient.current() : selectedGradientStart();
}

void ContextBox::setListValues(const Gui::ListValues & values){
    this->values = values;
    if (useGradient){
        selectedGradient = modifiedGradient(values.getLowColor(), values.getHighColor());
    }
}

void ContextBox::adjustLeft(){
}

void ContextBox::adjustRight(){
}

void ContextBox::open(){
    // Set the fade stuff
    fadeState = FadeIn;
    //board.position = position;
    board.location = location;
    board.transforms = transforms;
    board.colors = colors;
    board.open();
    fadeAlpha = 0;
    cursorLocation = 0;
}

void ContextBox::close(){
    fadeState = FadeOut;
    board.close();
    fadeAlpha = 255;
    cursorLocation = 480;
}


void ContextBox::doFade(){
    switch (fadeState){
        case FadeIn: {
            if (fadeAlpha < 255){
                fadeAlpha += (fadeSpeed+2);
            }

            if (fadeAlpha >= 255){
                fadeAlpha = 255;
                if (board.isActive()){
                    fadeState = Active;
                }
            }
            break;
        }
        case FadeOut: {
            if (fadeAlpha > 0){
                fadeAlpha -= (fadeSpeed+2);
            }

            if (fadeAlpha <= 0){
                fadeAlpha = 0;
                if (!board.isActive()){
                    fadeState = NotActive;
                }
            }
            break;
        }
        case Active:
        case NotActive:
        default:
            break;
    }
}

void ContextBox::setList(const std::vector<Util::ReferenceCount<ContextItem> > & list){
    this->list->clearItems();
    for (vector<Util::ReferenceCount<ContextItem> >::const_iterator it = list.begin(); it != list.end(); it++){
        const Util::ReferenceCount<ContextItem> & item = *it;
        this->list->addItem(item.convert<ScrollItem>());
    }
}
        
void ContextBox::addItem(const Util::ReferenceCount<ContextItem> & item){
    this->list->addItem(item.convert<ScrollItem>());
}

void ContextBox::setListType(const ListType & type){
    switch (type){
        case Normal:{
            Util::ReferenceCount<ScrollListInterface> newList = new NormalList();
            newList->addItems(list->getItems());
            list = newList;
            break;
        }
        case Scroll:{
            Util::ReferenceCount<ScrollListInterface> newList = new ScrollList();
            newList->addItems(list->getItems());
            list = newList;
            break;
        }
        default:
            break;
    }
}

void ContextBox::setListWrap(bool wrap){
    list->setWrap(wrap);
}

void ContextBox::drawText(const Graphics::Bitmap & bmp, const Font & font){
    const int x1 = board.getArea().getX()+(int)(board.getTransforms().getRadius()/2);
    const int y1 = board.getArea().getY()+2;//(board.getArea().radius/2);
    const int x2 = board.getArea().getX2()-(int)(board.getTransforms().getRadius()/2);
    const int y2 = board.getArea().getY2()-2;//(board.getArea().radius/2);
            
    Graphics::Bitmap area(bmp, x1, y1, x2 - x1, y2 - y1);

    list->render(area, font);
}

}
