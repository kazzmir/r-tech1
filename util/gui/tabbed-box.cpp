#include "../bitmap.h"
#include "../trans-bitmap.h"
#include "tabbed-box.h"

#include "menu/menu.h"

#include "util/font.h"

#include "context-box.h"

using namespace Gui;

#if 0
/* FIXME add rounded tabs */
static void roundTab( const Bitmap & work, int radius, int x1, int y1, int x2, int y2, int color, bool bottom = true ){
    const int width = x2 - x1;
    const int height = y2 - y1;
    radius = Mid(0, radius, Min((x1+width - x1)/2, (y1+height - y1)/2));
    
    work.circleFill(x1+radius, y1+radius, radius, color);
    work.circleFill((x1+width)-radius, y1+radius, radius, color);
    work.circleFill(x1+radius, (y1+height)-radius, radius, color);
    work.circleFill((x1+width)-radius, (y1+height)-radius, radius, color);
    work.rectangleFill( x1+radius, y1, x2-radius, y1+radius, color);
    work.rectangleFill( x1, y1+radius, x2, y2-radius, color);
    work.rectangleFill( x1+radius, y2-radius, x2-radius, y2, color);
    
    work.line(x1+radius, y1, x1+width-radius, y1, color);
    work.line(x1+radius, y1+height, x1+width-radius,y1+height, color);
    work.line(x1, y1+radius,x1, y1+height-radius, color);
    work.line(x1+width, y1+radius,x1+width, y1+height-radius, color);

    arc(work, x1+radius, y1+radius, S_PI-1.115, radius, color);
    arc(work, x1+radius + (width - radius *2), y1+radius, -S_PI/2 +0.116, radius, color);
    arc(work, x1+width-radius, y1+height-radius, -0.110, radius ,color);
    arc(work, x1+radius, y1+height-radius, S_PI/2-0.119, radius, color);
}
#endif

Tab::Tab():
// context(new ContextBox()),
active(false){
    // Set alpha to 0 as we are not interested in the box
    context.setRenderOnlyText(true);
}

Tab::~Tab(){
    // delete context;
}
    
void Tab::addOption(const Util::ReferenceCount<ContextItem> & item){
    context.addItem(item);
}

void Tab::render(const Graphics::Bitmap & area, const Font & font){
    context.render(area, font);
}
    
void Tab::act(const Font & font){
    context.act(font);
}
    
void Tab::setList(const std::vector<Util::ReferenceCount<ContextItem> > & list){
    context.setList(list);
}

void Tab::setName(const std::string & name){
    this->name = name;
}

void Tab::close(){
    context.close();
}

void Tab::open(){
    context.open();
}

void Tab::previous(const Font & font){
    context.previous(font);
}

void Tab::next(const Font & font){
    context.next(font);
}

void Tab::adjustLeft(){
    context.adjustLeft();
}

void Tab::adjustRight(){
    context.adjustRight();
}

int Tab::getCurrentIndex(){
    return context.getCurrentIndex();
}

TabbedBox::TabbedBox():
current(0),
fontWidth(24),
fontHeight(24),
inTab(false),
tabWidthMax(0),
tabFontColor(Graphics::makeColor(255,255,255)),
currentTabFontColor(Graphics::makeColor(0,0,255)){
    activeTabFontColor = new Effects::Gradient(50, tabFontColor, currentTabFontColor);
}

TabbedBox::TabbedBox(const TabbedBox & b):
activeTabFontColor(NULL){
    this->location = b.location;
}

TabbedBox::~TabbedBox(){
    for (std::vector<Gui::Tab *>::iterator i = tabs.begin(); i != tabs.end(); ++i){
        Gui::Tab * tab = *i;
        if (tab){
            delete tab;
        }
    }
    
    if (activeTabFontColor){
        delete activeTabFontColor;
    }
}

TabbedBox &TabbedBox::operator=( const TabbedBox &copy){
    location = copy.location;

    return *this;
}

// Logic
void TabbedBox::act(const Font & font){
    if (!tabs.empty()){
        // const Font & vFont = Font::getFont(font, fontWidth, fontHeight);
        //tabWidthMax = location.getWidth()/tabs.size();
        const int width = font.textLength(tabs[current]->name.c_str()) + 5;
        if (tabs.size() > 1){
            tabWidthMax = (location.getWidth() - width) / (tabs.size() - 1);
        } else {
            tabWidthMax = location.getWidth() - width;
        }
    } else {
        return;
    }
    if (!tabs[current]->active){
        tabs[current]->active = true;
    }
    tabs[current]->act(font);
    if (inTab){
        if (activeTabFontColor){
            activeTabFontColor->update();
        }
    }
}
        
void TabbedBox::render(const Graphics::Bitmap & work){
    /* nothing */
}

// Render
void TabbedBox::render(const Graphics::Bitmap & work, const Font & font){
    const int tabHeight = fontHeight + 5;
    // checkWorkArea();
    Graphics::Bitmap area(work, location.getX(), location.getY(), location.getWidth(), location.getHeight());
    // Check if we are using a rounded box
    if (transforms.getRadius() > 0){
        //roundRectFill( *workArea, (int)transforms.getRadius(), 0, 0, location.getWidth()-1, location.getHeight()-1, colors.body );
        //roundRect( *workArea, (int)transforms.getRadius(), 0, 0, location.getWidth()-1, location.getHeight()-1, colors.border );
    } else {
        area.translucent().rectangleFill(0, tabHeight+1, location.getWidth()-1, location.getHeight()-1, colors.body );
        //area.translucent().rectangle(0, tabHeight, location.getWidth()-1, location.getHeight()-1, colors.border );
        area.translucent().vLine(tabHeight,0,location.getHeight()-1,colors.border);
        area.translucent().hLine(0,location.getHeight()-1,location.getWidth()-1,colors.border);
        area.translucent().vLine(tabHeight,location.getWidth()-1,location.getHeight()-1,colors.border);
    }
    
    tabs[current]->render(area, font);
    
    renderTabs(area, font);
    
    /* FIXME: only render the background in translucent mode, the text should
     * not be translucent
     */
    // workArea->draw(location.getX(), location.getY(), work);
}

// Add tab
void TabbedBox::addTab(const std::string & name, const std::vector<Util::ReferenceCount<ContextItem> > & list){
    for (std::vector<Tab *>::iterator i = tabs.begin(); i != tabs.end(); ++i){
        Tab * tab = *i;
        if (tab->name == name){
            return;
        }
    }
    Tab * tab = new Tab();
    tab->name = name;
    tab->setList(list);
    // tab->context->setFont(font, fontWidth, fontHeight);
    addTab(tab);
    /*
    const int modifier = fontHeight * .35;
    tab->context.location.setPosition(Gui::AbsolutePoint(0, fontHeight + modifier));
    tab->context.location.setPosition2(Gui::AbsolutePoint(location.getWidth(), location.getHeight()- modifier));
    tab->context.open();
    tabs.push_back(tab);
    */
}

void TabbedBox::addTab(Tab * tab){
    const int modifier = fontHeight * .35;
    tab->getContext().location.setPosition(Gui::AbsolutePoint(0, fontHeight + modifier));
    tab->getContext().location.setPosition2(Gui::AbsolutePoint(location.getWidth(), location.getHeight()- modifier));
    tab->open();
    tabs.push_back(tab);
}

void TabbedBox::moveTab(int direction){
    tabs[current]->close();
    tabs[current]->active = false;
    current = (unsigned int) ((int)current + direction + (int) tabs.size()) % tabs.size();
    /*
    if (current == 0){
        current = tabs.size()-1;
    } else {
        current--;
    }
    */
    tabs[current]->open();
    tabs[current]->active = true;
}

void TabbedBox::up(const Font & font){
    if (tabs.size() == 0){
        return;
    }
    if (!inTab){
        moveTab(-1);
    } else {
        tabs[current]->previous(font);
    }
}

void TabbedBox::down(const Font & font){
    if (tabs.size() == 0){
        return;
    }
    if (!inTab){
        moveTab(1);
    } else {
        tabs[current]->next(font);
    }
}

void TabbedBox::left(const Font & font){
    if (tabs.size() == 0){
        return;
    }
    if (!inTab){
        moveTab(-1);
    } else {
        tabs[current]->adjustLeft();
    }
}

void TabbedBox::right(const Font & font){
    if (tabs.size() == 0){
        return;
    }
    if (!inTab){
        moveTab(1);
    } else {
        tabs[current]->adjustRight();
    }
}

void TabbedBox::toggleTabSelect(){
    inTab = !inTab;
}

unsigned int TabbedBox::getCurrentIndex() const {
    if (tabs.size() == 0){
        return 0;
    }
    return this->tabs[current]->getCurrentIndex();
}

void TabbedBox::setTabFontColor(Graphics::Color color){
    tabFontColor = color;
    if (activeTabFontColor){
        delete activeTabFontColor;
    }
    activeTabFontColor = new Effects::Gradient(50, tabFontColor, currentTabFontColor);
}

void TabbedBox::setSelectedTabFontColor(Graphics::Color color){
    currentTabFontColor = color;
    if (activeTabFontColor){
        delete activeTabFontColor;
    }
    activeTabFontColor = new Effects::Gradient(50, tabFontColor, currentTabFontColor);
}

void TabbedBox::renderTabs(const Graphics::Bitmap & bmp, const Font & vFont){
    const int tabHeight = fontHeight + 5;
    // const Font & vFont = Font::getFont(font, fontWidth, fontHeight);
    
    int x = 0;
    Graphics::Bitmap::transBlender(0, 0, 0, colors.bodyAlpha);
    
    for (std::vector<Gui::Tab *>::iterator i = tabs.begin(); i != tabs.end(); ++i){
        Gui::Tab * tab = *i;
        const int textWidth = vFont.textLength(tab->name.c_str()) + 5;
        // for last tab
        int modifier = 0;
        // Check last tab so we can ensure proper sizing
        if ( i == (tabs.begin() + tabs.size() -1)){
            if ( ( (tabWidthMax * (tabs.size() - 1) ) + textWidth ) != (unsigned int)location.getWidth() ){
            modifier = location.getWidth() - x - (tab->active ? textWidth : tabWidthMax);
            }
        }
        
        if (tab->getContext().transforms.getRadius() > 0){
        } else {
            if (tab->active){
                if (!inTab){
                    //bmp.translucent().rectangle(x, 0, x+textWidth + modifier - 1, tabHeight, colors.border);
                    bmp.translucent().vLine(0,x,tabHeight,colors.border);
                    bmp.translucent().hLine(x,0,x+textWidth+modifier-1,colors.border);
                    bmp.translucent().vLine(0,x+textWidth+modifier-1,tabHeight,colors.border);
                    bmp.translucent().rectangleFill( x+1, 1, x+textWidth + modifier - 2, tabHeight, colors.body);

                    bmp.setClipRect(x, 0, x+textWidth + modifier, tabHeight-1);
                    vFont.printf(x + (((textWidth + modifier)/2)-(((textWidth + modifier) - 5)/2)), 0, currentTabFontColor, bmp, tab->name, 0 );
                } else {
                    //bmp.translucent().rectangle(x, 0, x+textWidth + modifier -1, tabHeight, colors.border);
                    bmp.translucent().vLine(0,x,tabHeight,colors.border);
                    bmp.translucent().hLine(x,0,x+textWidth+modifier-1,colors.border);
                    bmp.translucent().vLine(0,x+textWidth+modifier-1,tabHeight,colors.border);
                    bmp.translucent().rectangleFill( x+1, 1, x+textWidth-2 + modifier, tabHeight, colors.body );
                    
                    bmp.setClipRect(x, 0, x+textWidth + modifier, tabHeight-1);
                    vFont.printf(x + (((textWidth + modifier)/2)-(((textWidth + modifier) - 5)/2)), 0, activeTabFontColor->current(), bmp, tab->name, 0 );
                }

                x+=textWidth + modifier;
            } else {
                const int heightMod = tabHeight * .15;
                // FIXME make background tabs customizable for now just make it gray
                bmp.translucent().rectangle(x, 1 + heightMod, x+tabWidthMax + modifier -1, tabHeight, Graphics::makeColor(58, 58, 58));
                bmp.translucent().hLine(x,tabHeight,x+tabWidthMax+modifier-1,colors.border);
                // FIXME make background tabs customizable for now just make it gray
                bmp.translucent().rectangleFill( x+1, 2 + heightMod, x+tabWidthMax + modifier -2, tabHeight-2, Graphics::makeColor(105, 105, 105));
                
                bmp.setClipRect(x+2, 6 + heightMod, x+tabWidthMax + modifier -3, tabHeight-1);
                
#if 0
                // FIXME find a way to make the font smaller
                const int oldsizex = vFont.getSizeX();
                const int oldsizey = vFont.getSizeY();
                vFont.setSize(oldsizex/2, oldsizey/2);
                const int disabledTextWidth = vFont.textLength(tab->name.c_str()) + 5;
                vFont.printf(x + (((tabWidthMax + modifier)/2)-((disabledTextWidth + modifier)/2)), 0, tabFontColor, bmp, tab->name, 0 );
                vFont.setSize(oldsizex, oldsizey);
#endif
                vFont.printf(x + (((tabWidthMax + modifier)/2)-((textWidth + modifier)/2)), 0, tabFontColor, bmp, tab->name, 0 );
                x += tabWidthMax + modifier;
            }
            bmp.setClipRect(0, 0, bmp.getWidth(), bmp.getHeight());
        }
    }
}
