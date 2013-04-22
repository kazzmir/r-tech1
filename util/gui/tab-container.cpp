#include "tab-container.h"

#include "util/font.h"

namespace Gui{

TabItem::TabItem(){
}

TabItem::TabItem(const std::string & name):
name(name){
}

TabItem::~TabItem(){
}

DummyTab::DummyTab(const std::string & name):
TabItem(name){
}

DummyTab::~DummyTab(){
}

void DummyTab::act(){
}

void DummyTab::draw(const Font&, const Graphics::Bitmap & work){
    work.fill(Graphics::makeColor(220, 220, 220));
}

TabContainer::TabContainer(){
}

TabContainer::TabContainer(const TabContainer & copy){
}

TabContainer::~TabContainer(){
}

TabContainer & TabContainer::operator=(const TabContainer & copy){
    return *this;
}

void TabContainer::act(const Font & font){
}

void TabContainer::render(const Graphics::Bitmap &){
}

void TabContainer::draw(const Font & font, const Graphics::Bitmap & work){
    const int tabHeight = font.getHeight();
    // rounded?
#if 0    
    if (transforms.getRadius() > 0){
        Graphics::Bitmap area(work, 0, tabHeight+1);
        if (colors.bodyAlpha < 255){
            Graphics::Bitmap::transBlender(0,0,0,colors.bodyAlpha);
            area.translucent().roundRectFill(transforms.getRadius(), 0, -(tabHeight+1), location.getWidth(), location.getHeight(), colors.body);
            area.translucent().roundRect(transforms.getRadius(), 0, -(tabHeight+1), location.getWidth(), location.getHeight(), colors.border);
        } else {
            area.roundRectFill(transforms.getRadius(), 0, -(tabHeight+1), location.getWidth(), location.getHeight(), colors.body);
            area.roundRect(transforms.getRadius(), 0, -(tabHeight+1), location.getWidth(), location.getHeight(), colors.border);
        }
    } else {
        Graphics::Bitmap area(work, location.getX(), location.getY(), location.getWidth(), location.getHeight());
        if (colors.bodyAlpha < 255){
            Graphics::Bitmap::transBlender(0,0,0,colors.bodyAlpha);
            area.translucent().rectangleFill(0, tabHeight+1, location.getWidth()-1, location.getHeight()-1, colors.body );
            area.translucent().vLine(tabHeight,0,location.getHeight()-1,colors.border);
            area.translucent().hLine(0,location.getHeight()-1,location.getWidth()-1,colors.border);
            area.translucent().vLine(tabHeight,location.getWidth()-1,location.getHeight()-1,colors.border);
        } else {
            area.rectangleFill(0, tabHeight+1, location.getWidth()-1, location.getHeight()-1, colors.body );
            area.vLine(tabHeight,0,location.getHeight()-1,colors.border);
            area.hLine(0,location.getHeight()-1,location.getWidth()-1,colors.border);
            area.vLine(tabHeight,location.getWidth()-1,location.getHeight()-1,colors.border);
        }
    }
#endif
}

void TabContainer::add(Util::ReferenceCount<TabItem> tab){
    tabs.push_back(tab);
}

}
