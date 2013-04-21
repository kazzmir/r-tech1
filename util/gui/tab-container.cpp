#include "tab-container.h"

namespace Gui{

TabItem::TabItem(){
}

TabItem::~TabItem(){
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

void TabContainer::act(const Font &){
}

void TabContainer::render(const Graphics::Bitmap &){
}

void TabContainer::draw(const Font &, const Graphics::Bitmap &){
}

}
