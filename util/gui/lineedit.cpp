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

void drawBox(int radius, int x, int y, int width, int height, const Gui::ColorInfo & colors, const Graphics::Bitmap & where){
    if (radius > 0){
        if (colors.bodyAlpha < 255){
            Graphics::Bitmap::transBlender(0,0,0,colors.bodyAlpha);
            where.translucent().roundRectFill(radius, x, y, x+width, y+height, colors.body);
        } else {
            where.roundRectFill(radius, x, y, x+width, y+height, colors.body);
        }
        
        if (colors.borderAlpha < 255){
            Graphics::Bitmap::transBlender(0,0,0,colors.borderAlpha);
            where.translucent().roundRect(radius, x, y, x+width, y+height, colors.border);
        } else {
            where.roundRect(radius, x, y, x+width, y+height, colors.border);
        }
    } else {
        if (colors.bodyAlpha < 255){
            Graphics::Bitmap::transBlender(0,0,0,colors.bodyAlpha);
            where.translucent().rectangleFill(x, y, x+width, y+height, colors.body);
        } else {
            where.rectangleFill(x, y, x+width, y+height, colors.body);
        }
        if (colors.borderAlpha < 255){
            Graphics::Bitmap::transBlender(0,0,0,colors.borderAlpha);
            where.translucent().rectangle(x, y, x+width, y+height, colors.border);
        } else {
            where.rectangle(x, y, x+width, y+height, colors.border);
        }
    }
}

LineEdit::LineEdit():
blinkRate(60),
cursorTime(0),
changed(false){
    colors.body = Graphics::makeColor(0,0,60);
    colors.border = Graphics::makeColor(0,0,20);
    colors.bodyAlpha = colors.borderAlpha = 150;
    textColor = Graphics::makeColor(255,255,255);
    cursorColor = Graphics::makeColor(128,128,128);
    transforms.setRadius(10);
    input.enable();
}

LineEdit::~LineEdit(){
}

void LineEdit::setTextColor(const Graphics::Color color){
    textColor = color;
}

void LineEdit::setCursorColor(const Graphics::Color color){
    cursorColor = color;
}

void LineEdit::act(const Font &){
    if (input.doInput()){
        changed = true;
    } else {
        changed = false;
    }
    cursorTime++;
    if (cursorTime > blinkRate*2){
        cursorTime = 0;
    }
}

void LineEdit::draw(const Font & font, const Graphics::Bitmap & work){
    
    Graphics::Bitmap temp = Graphics::Bitmap(work, location.getX(), location.getY(), location.getWidth(), location.getHeight());
    
    drawBox(10, 0, 0, location.getWidth(), location.getHeight(), colors, temp);
    
    Graphics::Bitmap textTemp = Graphics::Bitmap(temp, 10, 0, temp.getWidth()-10, temp.getHeight());
    int fontLength = font.textLength(input.getText().c_str());
    if (fontLength >= textTemp.getWidth()){
        font.printf(textTemp.getWidth()-10 - fontLength, 0, textColor, textTemp, input.getText(), 0);
        renderCursor(textTemp.getWidth()-11, 0, font, textTemp);
    } else {
        font.printf(0, 0, textColor, textTemp, input.getText(), 0);
        renderCursor(fontLength+1, 0, font, textTemp);
    }
}

void LineEdit::render(const Graphics::Bitmap &){
}

void LineEdit::toggleEnabled(){
    if (input.isEnabled()){
        input.disable();
    } else {
        input.enable();
    }
}

void LineEdit::setFocused(bool enabled){
    if (enabled){
        input.disable();
    } else {
        input.enable();
    }
}

void LineEdit::addHook(int key, void (*callback)(void *), void * arg){
    input.addBlockingHandle(key, callback, arg);
}

bool LineEdit::didChanged(unsigned long long &){
    return changed;
}

void LineEdit::renderCursor(int x, int y, const Font & font, const Graphics::Bitmap & work){
    if (cursorTime <= blinkRate){
        font.printf(x, y, cursorColor, work, "|", 0);
    }
}
