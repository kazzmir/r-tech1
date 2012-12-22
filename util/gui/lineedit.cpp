#include "util/graphics/bitmap.h"
#include "util/font.h"
#include "lineedit.h"
#include "util/debug.h"
#include <iostream>

using namespace Gui;

static Global::stream_type & debug(int level){
    Global::debug(level) << "[line edit] ";
    return Global::debug(level);
}

LineEdit::LineEdit() : 
currentSetFont(0), 
hAlignment(T_Middle),
hAlignMod(T_Middle),
vAlignment(T_Middle),
inputTypeValue(inputGeneral),
changed_(0),
autoResizable(0),
textX(0),
textY(0),
cursorX(0),
cursorY(0),
cursorIndex(0),
textColor(Graphics::makeColor(0, 0, 0)),
textSizeH(0),
limit(0),
blinkRate(500),
blink(false),
focused(false),
changeCounter(0){
    cursorTime.reset();
}

LineEdit::~LineEdit(){
    if (focused){
        input.disable();
    }
}
    
void LineEdit::hookKey(int key, void (*callback)(void *), void * arg){
    input.addBlockingHandle(key, callback, arg);
}
	
bool LineEdit::didChanged(unsigned long long & counter){
    bool did = counter < changeCounter;
    counter = changeCounter;
    return did;
}

// If the font size changes
void LineEdit::fontChange(){
    changed();
}

// Update
void LineEdit::act(const Font & font){
    if (cursorTime.msecs() >= blinkRate){
        cursorTime.reset();
        blink = !blink;
        changed();
    }
    /*
    if ((blinkRate * 2) <= cursorTime.msecs()){
        cursorTime.reset();
        changed();
    }
    */

    if (input.doInput()){
        changed();
        cursorIndex = input.getText().size();
    }

    if (changed_){
        textSizeH = currentSetFont->getHeight();
        if (autoResizable) {
            location.setDimensions(textSizeH+2, currentSetFont->textLength(input.getText().c_str()) + 4);
        } else {
            if (hAlignMod==T_Left){
                if (currentSetFont->textLength(input.getText().c_str())>location.getWidth()){
                    hAlignment = T_Right;
                } else {
                    hAlignment = T_Left;
                }
            }
        }

        switch (hAlignment) {
            case T_Left:
                textX = 2;
                cursorX = textX + currentSetFont->textLength(input.getText().substr(0,cursorIndex).c_str()) + 1;
                break;
            case T_Middle:
                textX = (location.getWidth()/2) - (currentSetFont->textLength(input.getText().c_str())/2);
                cursorX = (textX) + currentSetFont->textLength(input.getText().substr(0,cursorIndex).c_str()) + 1;
                break;
            case T_Right:
                textX = location.getWidth() - currentSetFont->textLength(input.getText().c_str());//(position.width - 1)-2;
                cursorX = location.getWidth() - currentSetFont->textLength(input.getText().substr(0, input.getText().length()-cursorIndex).c_str());
                break;
            case T_Bottom:
            case T_Top:
                break;
        }

        switch (vAlignment) {
            case T_Top:
                textY = 1;
                cursorY = 1;
                break;
            case T_Middle:
                textY = cursorY = (location.getHeight() - textSizeH-(5))/2;
                break;
            case T_Bottom:
                textY = (location.getHeight() - 1) - textSizeH - 1;
                cursorY = textY - textSizeH;
                break;
            case T_Right:
            case T_Left:
                break;
        }

        //textY++;
        //textX++;
        stable();
    }
}

// Draw
void LineEdit::render(const Graphics::Bitmap & work){

    Util::ReferenceCount<Graphics::Bitmap> workArea = checkWorkArea(work);

    /* Check if we are using a rounded box */
    if (transforms.getRadius() > 0){
        workArea->translucent(0, 0, 0, colors.bodyAlpha).roundRectFill((int)transforms.getRadius(), 0, 0, location.getWidth()-1, location.getHeight()-1, colors.body );
        workArea->translucent(0, 0, 0, colors.borderAlpha).roundRect((int)transforms.getRadius(), 0, 0, location.getWidth()-1, location.getHeight()-1, colors.border);
    } else {
        workArea->translucent(0, 0, 0, colors.bodyAlpha).rectangleFill(0, 0, location.getWidth()-1, location.getHeight()-1, colors.body);
        workArea->translucent(0, 0, 0, colors.borderAlpha).rectangle(0, 0, location.getWidth()-1, location.getHeight()-1, colors.border);
    }

    if (currentSetFont){
        currentSetFont->printf(textX, textY, textColor, *workArea, input.getText(), 0);
    }

    if (focused){
        if (blink){
            workArea->line(cursorX, cursorY, cursorX, cursorY+textSizeH-5, textColor);
        }
    }
}

// Set text
void LineEdit::setText(const std::string & text){
    input.setText(text);
    if (limit!=0) {
        if (input.getText().length() > limit) {
            while (input.getText().length() > limit){
                // currentSetText.erase(currentSetText.begin()+currentSetText.length()-1);
            }
        }
    }
    cursorIndex = input.getText().length();
    changed();
}


//! Get text
const std::string LineEdit::getText(){
    return input.getText();
}
		
//! Clear text
void LineEdit::clearText(){
    input.clearInput();
    cursorIndex=0;
    changed();
}

//! Set text limit
void LineEdit::setLimit(unsigned int l){
    limit = l;
    if (limit!=0){
        if (input.getText().length()>limit){
            while (input.getText().length()>limit){
                // currentSetText.erase(currentSetText.begin()+currentSetText.length()-1);
            }
        }
    }
    cursorIndex = input.getText().length();
    changed();
}
		
// Set Horizontal Alignment
void LineEdit::setHorizontalAlign(const textAlign & a){
    hAlignment = hAlignMod = a;
    changed();
}
		
// Set Vertical Alignment
void LineEdit::setVerticalAlign(const textAlign & a){
    vAlignment = a;
    changed();
}

//! Set the type of input default general
void LineEdit::setInputType(const inputType i){
    inputTypeValue = i;
}
		
// Set textColor
void LineEdit::setTextColor(const Graphics::Color color){
    textColor = color;
}

//! Set textColor
void LineEdit::setCursorColor(const Graphics::Color color){
    textColor = color;
}

// Set font
void LineEdit::setFont(const Font *f){
    currentSetFont = f;
    if (currentSetFont) changed();
}

// Set autoResizeable
void LineEdit::setAutoResize(bool r){
    autoResizable = r;
}

// Set the cursor blink rate in miliseconds (default 500)
void LineEdit::setCursorBlinkRate(unsigned int msecs){
    blinkRate = msecs;
}

//! set Focus
void LineEdit::setFocused(bool focus){
    focused = focus;
    if (focus){
        input.enable();
    } else {
        input.disable();
    }
}

//! check Focus
bool LineEdit::isFocused(){
    return focused;
}
