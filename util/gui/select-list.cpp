#include "select-list.h"

#include "util/bitmap.h"
#include "util/font.h"
#include "util/debug.h"

#include <math.h>

using namespace Gui;

SelectItem::SelectItem(){
}
SelectItem::~SelectItem(){
}

SelectListInterface::SelectListInterface():
accessEmpty(true),
allowWrap(true){
}

SelectListInterface::~SelectListInterface(){
}

SimpleSelect::SimpleSelect():
layout(Horizontal),
viewable(3),
currentTop(0),
scrollOffset(0),
cellWidth(100),
cellHeight(100),
cellSpacingX(0),
cellSpacingY(0),
cellMarginX(0),
cellMarginY(0){
}
SimpleSelect::~SimpleSelect(){
}
void SimpleSelect::act(){
    /* Check if viewable is larger than the current items */
    if (viewable >= items.size()){
        viewable = items.size()-1;
    }
}
void SimpleSelect::render(const Graphics::Bitmap & work, const Font & font) const{
    int x = cellMarginX;
    int y = cellMarginY;
    const int stop = currentTop + viewable;
    int count = currentTop;
    for (std::vector<Util::ReferenceCount<SelectItem> >::const_iterator i = items.begin() + currentTop; i != items.end() && count != stop; ++i, ++count){
        const Util::ReferenceCount<SelectItem> item = *i;
        item->draw(x, y, cellWidth, cellHeight, work, font);
        x+=cellSpacingX + (layout == Horizontal ? cellWidth + cellMarginX : 0);
        y+=cellSpacingY + (layout == Vertical ? cellHeight + cellMarginY : 0);
    }
}
void SimpleSelect::addItem(const Util::ReferenceCount<SelectItem> & item){
    items.push_back(item);
}
void SimpleSelect::addItems(const std::vector<Util::ReferenceCount<SelectItem> > & itemList){
    items.insert(items.begin(), itemList.begin(), itemList.end());
}
const std::vector<Util::ReferenceCount<SelectItem> > & SimpleSelect::getItems() const{
    return items;
}
void SimpleSelect::clearItems(){
    items.clear();
}
void SimpleSelect::setCellDimensions(int width, int height){
    cellWidth = width;
    cellHeight = height;
}
void SimpleSelect::setCellSpacing(int x, int y){
    cellSpacingX = x;
    cellSpacingY = y;
}
void SimpleSelect::setCellMargins(int x, int y){
    cellMarginX = x;
    cellMarginY = y;
}
void SimpleSelect::setCursors(int total){
    cursors.resize(total);
}
int SimpleSelect::totalCursors() const{
    return cursors.size();
}
void SimpleSelect::setCurrentIndex(int cursor, unsigned int location){
    if (checkCursor(cursor) && location >= items.size()){
        return;
    }
    cursors[cursor] = location;
}
unsigned int SimpleSelect::getCurrentIndex(int cursor) const{
    if (checkCursor(cursor)){
        return 0;
    }
    return cursors[cursor];
}
/* NOTE This doesn't account for other cursors and viewable areas */
bool SimpleSelect::up(int cursor){
    if (checkCursor(cursor)){
        return false;
    }
    if (cursors[cursor] > 0){
        cursors[cursor]--;
        calculateLeft(cursor);
        return true;
    } else if (allowWrap){
        cursors[cursor] = items.size()-1;
        currentTop = cursors[cursor] - viewable+1;
        return true;
    }
    return false;
}
bool SimpleSelect::down(int cursor){
    if (checkCursor(cursor)){
        return false;
    }
    if (cursors[cursor] < items.size()-1){
        cursors[cursor]++;
        calculateRight(cursor);
        return true;
    } else if (allowWrap){
        cursors[cursor] = currentTop = 0;
        return true;
    }
    return false;
}
bool SimpleSelect::left(int cursor){
    if (checkCursor(cursor)){
        return false;
    }
    if (cursors[cursor] > 0){
        cursors[cursor]--;
        calculateLeft(cursor);
        return true;
    } else if (allowWrap){
        cursors[cursor] = items.size()-1;
        currentTop = cursors[cursor] - viewable + 1;
        return true;
    }
    return false;
}

bool SimpleSelect::right(int cursor){
    if (checkCursor(cursor)){
        return false;
    }
    if (cursors[cursor] < items.size()-1){
        cursors[cursor]++;
        calculateRight(cursor);
        return true;
    } else if (allowWrap){
        cursors[cursor] = currentTop = 0;
        return true;
    }
    return false;
}

bool SimpleSelect::checkCursor(int cursor) const {
    return ((unsigned int)cursor >= cursors.size());
}

void SimpleSelect::calculateLeft(int cursor){
    if (currentTop == 0){
        //currentTop = cursors[cursor];
    } else if (cursors[cursor] < currentTop + scrollOffset){
        currentTop = cursors[cursor] - scrollOffset;
    }
}

void SimpleSelect::calculateRight(int cursor){
    const unsigned int view = viewable-1;
    if ((currentTop + view) == items.size()-1){
        //currentTop = right;
    } else if (cursors[cursor] >= (currentTop + view - scrollOffset)){
        currentTop = cursors[cursor] - view + scrollOffset;
    }
}

GridSelect::GridSelect():
layout(Static),
gridX(0),
gridY(0),
cellWidth(100),
cellHeight(100),
cellSpacingX(0),
cellSpacingY(0),
cellMarginX(0),
cellMarginY(0){
}
GridSelect::~GridSelect(){
}
void GridSelect::act(){
}
void GridSelect::render(const Graphics::Bitmap & where, const Font & font) const{
    std::vector<Util::ReferenceCount<SelectItem> >::const_iterator item_iterator = items.begin();
    switch (layout){
        case Static:{
            int y = cellMarginY;
            for (int row = 0; row < gridY; ++row){
                int x = cellMarginX;
                for (int column = 0; column < gridX; ++column){
                    if (item_iterator != items.end()){
                        Util::ReferenceCount<SelectItem> item = *item_iterator;
                        item->draw(x, y, cellWidth, cellHeight, where, font);
                        item_iterator++;
                    }
                    x+=cellSpacingX + cellWidth + cellMarginX;
                }
                y+=cellSpacingY + cellHeight + cellMarginY;
            }
            break;
        }
        case InfiniteHorizontal:
            break;
        case InfiniteVertical:
            break;
        default:
            break;
    }
}
void GridSelect::addItem(const Util::ReferenceCount<SelectItem> & item){
    items.push_back(item);
}
void GridSelect::addItems(const std::vector<Util::ReferenceCount<SelectItem> > & itemList){
    items.insert(items.begin(), itemList.begin(), itemList.end());
}
const std::vector<Util::ReferenceCount<SelectItem> > & GridSelect::getItems() const{
    return items;
}
void GridSelect::clearItems(){
    items.clear();
}
void GridSelect::setCellDimensions(int width, int height){
    cellWidth = width;
    cellHeight = height;
}
void GridSelect::setCellSpacing(int x, int y){
    cellSpacingX = x;
    cellSpacingY = y;
}
void GridSelect::setCellMargins(int x, int y){
    cellMarginX = x;
    cellMarginY = y;
}
void GridSelect::setCursors(int total){
    cursors.resize(total);
}
int GridSelect::totalCursors() const{
    return cursors.size();
}
void GridSelect::setCurrentIndex(int cursor, unsigned int location){
    if (checkCursor(cursor) && location >= items.size()){
        return;
    }
    cursors[cursor] = location;
}
unsigned int GridSelect::getCurrentIndex(int cursor) const{
    if (checkCursor(cursor)){
        return 0;
    }
    return cursors[cursor];
}
static bool inRange(int check, int start, int end){
    return (check >= start && check <= end);
}
static bool endPoint(int check, int start, int end, int increment){
    for (int i = start; i <= end; i+=increment){
        if (check == i){
            return true;
        }
    }
    return false;
}
bool GridSelect::up(int cursor){
    if (checkCursor(cursor)){
        return false;
    }
    switch (layout){
        case Static:{
            if (inRange(cursors[cursor], 0, gridX-1)){
                if (allowWrap){
                    unsigned int location = (gridX * (gridY-1)) + cursors[cursor];
                    if (location >= items.size()){
                        location = items.size()-1;
                    }
                    cursors[cursor] = location;
                }
            } else {
                cursors[cursor] -= gridX;
            }
            break;
        }
        case InfiniteHorizontal:
            break;
        case InfiniteVertical:
            break;
        default:
            break;
    }
    return false;
}
bool GridSelect::down(int cursor){
    if (checkCursor(cursor)){
        return false;
    }
    switch (layout){
        case Static:{
            if (inRange(cursors[cursor], gridX * (gridY-1), gridX * gridY)){
                if (allowWrap){
                    unsigned int location = cursors[cursor] - (gridX * (gridY-1));
                    cursors[cursor] = location;
                }
            } else {
                cursors[cursor] += gridX;
            }
            break;
        }
        case InfiniteHorizontal:
            break;
        case InfiniteVertical:
            break;
        default:
            break;
    }
    return false;
}
bool GridSelect::left(int cursor){
    if (checkCursor(cursor)){
        return false;
    }
    switch (layout){
        case Static:{
            if (endPoint(cursors[cursor], 0, gridX * gridY, gridX)){
                if (allowWrap){
                    cursors[cursor] += gridX-1;
                }
            } else {
                cursors[cursor]--;
            }
            break;
        }
        case InfiniteHorizontal:
            break;
        case InfiniteVertical:
            break;
        default:
            break;
    }
    return false;
}
bool GridSelect::right(int cursor){
    if (checkCursor(cursor)){
        return false;
    }
    switch (layout){
        case Static:{
            if (endPoint(cursors[cursor], gridX-1, gridX * gridY, gridX)){
                if (allowWrap){
                    int location = cursors[cursor] - gridX+1;
                    if (location < 0){
                        location = 0;
                    }
                    cursors[cursor] = location;
                }
            } else {
                cursors[cursor]++;
            }
            break;
        }
        case InfiniteHorizontal:
            break;
        case InfiniteVertical:
            break;
        default:
            break;
    }
    return false;
}

bool GridSelect::checkCursor(int cursor) const {
    return ((unsigned int)cursor >= cursors.size());
}