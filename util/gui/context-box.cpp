#include "util/bitmap.h"
#include "util/trans-bitmap.h"

#include "context-box.h"
#include "util/font.h"
#include <math.h>

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

ContextItem::ContextItem(const ContextBox & parent):
parent(parent){
}

ContextItem::~ContextItem(){
}

bool ContextItem::isAdjustable(){
    return false;
}
int ContextItem::getLeftColor(){
    return 0;
}
int ContextItem::getRightColor(){
    return 0;
}

void ContextItem::draw(int x, int y, const Graphics::Bitmap & where, const Font & font, int distance) const {
    if (distance == 0){
        Graphics::Bitmap::transBlender(0, 0, 0, parent.getFadeAlpha());
        font.printf(x, y, parent.getSelectedColor(), where.translucent(), getName(), 0);
    } else {
        int alpha = parent.getFadeAlpha() - fabs((double) distance) * 35;
        if (alpha < 0){
            alpha = 0;
        }
        Graphics::Bitmap::transBlender(0, 0, 0, alpha);
        font.printf(x, y, Graphics::makeColor(255, 255, 255), where.translucent(), getName(), 0);
    }
}

int ContextItem::size(const Font & font) const {
    return font.textLength(getName().c_str());
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
    switch ( fadeState ){
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

void ContextBox::calculateText(const Font & vFont){
    /*
    if (context.empty()){
        return;
    } 
    
    // const Font & vFont = Font::getFont(font, fontWidth, fontHeight);
    
    cursorCenter = (location.getY() + (int)location.getHeight()/2) - vFont.getHeight()/2;//(position.y + (int)position.height/2) - vFont.getHeight()/2;
    
    if (cursorLocation == cursorCenter){
        scrollWait = 4;
    } else {
	if (scrollWait <= 0){
	    cursorLocation = (cursorLocation + cursorCenter)/2;
	    scrollWait = 4;
	} else {
	    scrollWait--;
	}
    }
    */
}

/* draws the text, fading the items according to the distance from the
 * current selection.
 */
void ContextBox::doDraw(int x, int y, int middle_x, int min_y, int max_y, const Font & font, int current, int selected, const Graphics::Bitmap & area, int direction){
#if 0
    while (y < max_y && y > min_y){
        int pick = current;
        while (pick < 0){
            pick += context.size();
        }
        pick = pick % context.size();

        ContextItem * option = context[pick];
        const int startx = middle_x - font.textLength(option->getName().c_str())/2;

        /* draw current selection, make it glow */
        if (current == selected){
            Graphics::Bitmap::transBlender(0, 0, 0, fadeAlpha);
            Graphics::TranslucentBitmap translucent(area);
            const int color = useGradient ? selectedGradient.current() : selectedGradientStart();
            font.printf(x + startx, y, color, translucent, option->getName(), 0 );
            if (option->isAdjustable()){
                const int triangleSize = 14;
                int cx = startx - 15;
                int cy = (int)(y + (font.getHeight()/FONT_SPACER) / 2 + 2);

                /* do the triangles need to be translucent? */
                translucent.equilateralTriangle(cx, cy, 180, triangleSize, option->getLeftColor());

                cx = (x + startx + font.textLength(option->getName().c_str()))+15;
                translucent.equilateralTriangle(cx, cy, 0, triangleSize, option->getRightColor());
            }
        } else {
            /* draw some other item, and fade it */
            int count = (int) fabs((double) current - (double) selected);
            /* TODO: maybe scale by the number of total items instead of using 35 */
            int textAlpha = fadeAlpha - (count * 35);
            if (textAlpha < 0){
                textAlpha = 0;
            }
            Graphics::Bitmap::transBlender(0, 0, 0, textAlpha);
            const int color = Graphics::makeColor(255,255,255);
            font.printf(x + startx, y, color, area.translucent(), option->getName(), 0);
        }

        if (context.size() == 1){
            return;
        }

        current += direction;
        y += direction * font.getHeight() / FONT_SPACER;
    }
#endif
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
    /*
    if (context.empty()){
        return;
    }
    */
    // const Font & vFont = Font::getFont(font, fontWidth, fontHeight);
    const int x1 = board.getArea().getX()+(int)(board.getTransforms().getRadius()/2);
    const int y1 = board.getArea().getY()+2;//(board.getArea().radius/2);
    const int x2 = board.getArea().getX2()-(int)(board.getTransforms().getRadius()/2);
    const int y2 = board.getArea().getY2()-2;//(board.getArea().radius/2);
            
    Graphics::Bitmap area(bmp, x1, y1, x2 - x1, y2 - y1);

    // int min_y = location.getX() - font.getHeight() - y1;
    // int max_y = location.getX2() + font.getHeight() - y1;

    list->render(area, font);

#if 0
    /* draw from the current selection down */
    doDraw(0, cursorLocation - y1, area.getWidth() / 2, min_y, max_y, font, current, current, area, 1);

    /* draw above the current selection */
    doDraw(0, cursorLocation - y1 - font.getHeight() / FONT_SPACER, area.getWidth() / 2, min_y, max_y, font, current - 1, current, area, -1);
#endif

#if 0
    int currentOption = current;
    int count = 0;
    /* draw the current selection and everything below it */
    while (locationY < location.getX2() + vFont.getHeight()){
        const int startx = (location.getWidth()/2)-(vFont.textLength(context[currentOption]->getName().c_str())/2);
        if (count == 0){
            Graphics::Bitmap::transBlender(0, 0, 0, fadeAlpha);
            Graphics::TranslucentBitmap translucent(area);
            // Bitmap::drawingMode( Bitmap::MODE_TRANS );
            const int color = useGradient ? selectedGradient.current() : selectedGradientStart();
            vFont.printf(location.getX() + startx - x1, locationY - y1, color, translucent, context[currentOption]->getName(), 0 );
            if (context[currentOption]->isAdjustable()){
                const int triangleSize = 14;
                int cx = (location.getX() + startx) - 15;
                int cy = (int)(locationY + (vFont.getHeight()/FONT_SPACER) / 2 + 2);

                /*
                int cx1 = cx + triangleSize / 2;
                int cy1 = cy - triangleSize / 2;
                int cx2 = cx - triangleSize;
                int cy2 = cy;
                int cx3 = cx + triangleSize / 2;
                int cy3 = cy + triangleSize / 2;
                */

                /* do the triangles need to be translucent? */
                // translucent.triangle(cx1, cy1, cx2, cy2, cx3, cy3, context[currentOption]->getLeftColor());
                translucent.equilateralTriangle(cx, cy, 180, triangleSize, context[currentOption]->getLeftColor());

                cx = (location.getX()+startx + vFont.textLength(context[currentOption]->getName().c_str()))+15;
                translucent.equilateralTriangle(cx, cy, 0, triangleSize, context[currentOption]->getLeftColor());
                // translucent.triangle( cx - triangleSize / 2, cy - triangleSize / 2, cx + triangleSize, cy, cx - triangleSize / 2, cy + triangleSize / 2, context[currentOption]->getRightColor() );
            }
            // Bitmap::drawingMode(Bitmap::MODE_SOLID);
        } else {
            int textAlpha = fadeAlpha - (count * 35);
            if (textAlpha < 0){
                textAlpha = 0;
            }
            Graphics::Bitmap::transBlender(0, 0, 0, textAlpha);
            // Bitmap::drawingMode( Bitmap::MODE_TRANS );
            const int color = Graphics::makeColor(255,255,255);
            vFont.printf(location.getX() + startx - x1, locationY - y1, color, area.translucent(), context[currentOption]->getName(), 0 );
            // Bitmap::drawingMode( Bitmap::MODE_SOLID );
        }
        if (context.size() == 1){
            // area.setClipRect(0, 0, bmp.getWidth(), bmp.getHeight());
            return;
        }
        currentOption++;
        if (currentOption == (int)context.size()){
            currentOption = 0;
        }
        locationY += (int)(vFont.getHeight()/FONT_SPACER);
        count++;
        /*if (context.size() < 2 && count == 2){
            break;
        }*/
    }
    locationY = cursorLocation - (int)(vFont.getHeight()/FONT_SPACER);
    currentOption = current;
    currentOption--;
    count = 0;

    /* this draws the stuff above the current selection */
    while (locationY > location.getX() - vFont.getHeight()){
        if (currentOption < 0){
            currentOption = context.size()-1;
        }
        const int startx = (location.getWidth()/2)-(vFont.textLength(context[currentOption]->getName().c_str())/2);
        int textAlpha = fadeAlpha - (count * 35);
        if (textAlpha < 0){
            textAlpha = 0;
        }
        Graphics::Bitmap::transBlender(0, 0, 0, textAlpha);
        const int color = Graphics::makeColor(255,255,255);
        vFont.printf(location.getX() + startx - x1, locationY - y1, color, area.translucent(), context[currentOption]->getName(), 0 );
        currentOption--;
        locationY -= (int)(vFont.getHeight()/FONT_SPACER);
        count++;
        /*if (context.size() < 2 && count == 1){
            break;
        }*/
    }
    // bmp.setClipRect(0, 0, bmp.getWidth(), bmp.getHeight());
#endif
}

}
