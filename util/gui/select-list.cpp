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
drawEmpty(false),
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
cellMarginY(0),
startOffsetX(0),
startOffsetY(0){
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
    int x = startOffsetX + cellMarginX;
    int y = startOffsetY + cellMarginY;
    const int stop = currentTop + viewable;
    int count = currentTop;
    for (std::vector<Util::ReferenceCount<SelectItem> >::const_iterator i = items.begin() + currentTop; i != items.end() && count != stop; ++i, ++count){
        const Util::ReferenceCount<SelectItem> item = *i;
        if (item->isEmpty()){
            if (drawEmpty){
                item->draw(x, y, cellWidth, cellHeight, work, font);
            }
        } else {
            item->draw(x, y, cellWidth, cellHeight, work, font);
        }
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

const Util::ReferenceCount<SelectItem> SimpleSelect::getItem(unsigned int index) const{
    if (index >= items.size()){
        return Util::ReferenceCount<SelectItem>();
    }
    return items[index];
}

const Util::ReferenceCount<SelectItem> SimpleSelect::getItemByCursor(int cursor) const{
    if (getCurrentIndex(cursor) >= items.size()){
        return Util::ReferenceCount<SelectItem>();
    }
    return items[getCurrentIndex(cursor)];
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

void SimpleSelect::setStartingOffset(int x, int y){
    startOffsetX = x;
    startOffsetY = y;
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

bool SimpleSelect::hasMoreLow() const{
    return (currentTop > 0);
}

bool SimpleSelect::hasMoreHigh() const{
    return ((currentTop + viewable) < items.size());
}

int SimpleSelect::getWidth(){
    int x = startOffsetX + cellMarginX;
    const int stop = currentTop + viewable;
    int count = currentTop;
    for (std::vector<Util::ReferenceCount<SelectItem> >::const_iterator i = items.begin() + currentTop; i != items.end() && count != stop; ++i, ++count){
        x+=cellSpacingX + (layout == Horizontal ? cellWidth + cellMarginX : 0);
    }
    x+=cellSpacingX + (layout == Horizontal ? cellWidth + cellMarginX : 0);
    return x;
}

int SimpleSelect::getHeight(){
    int y = startOffsetY + cellMarginY;
    const int stop = currentTop + viewable;
    int count = currentTop;
    for (std::vector<Util::ReferenceCount<SelectItem> >::const_iterator i = items.begin() + currentTop; i != items.end() && count != stop; ++i, ++count){
        y+=cellSpacingY + (layout == Vertical ? cellHeight + cellMarginY : 0);
    }
    y+=cellSpacingY + (layout == Vertical ? cellHeight + cellMarginY : 0);
    return y;
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
cellMarginY(0),
startOffsetX(0),
startOffsetY(0),
offset(0){
}

GridSelect::~GridSelect(){
}

void GridSelect::act(){
}

void GridSelect::render(const Graphics::Bitmap & where, const Font & font) const{
    std::vector<Util::ReferenceCount<SelectItem> >::const_iterator item_iterator = items.begin();
    switch (layout){
        case Static:{
            int x = startOffsetX + (cellSpacingX * gridY < 0 ? abs(cellSpacingX * gridY) : 0);
            int y = startOffsetY + (cellSpacingY * gridX < 0 ? abs(cellSpacingY * gridX) : 0);
            for (int row = 0; row < gridY; ++row){
                int x_spacing_mod = x;
                int y_spacing_mod = y;
                for (int column = 0; column < gridX; ++column){
                    if (item_iterator != items.end()){
                        Util::ReferenceCount<SelectItem> item = *item_iterator;
                        if (item->isEmpty()){
                            if (drawEmpty){
                                item->draw(x_spacing_mod, y_spacing_mod, cellWidth, cellHeight, where, font);
                            }
                        } else {
                            item->draw(x_spacing_mod, y_spacing_mod, cellWidth, cellHeight, where, font);
                        }
                        item_iterator++;
                    }
                    x_spacing_mod+= cellSpacingX + cellWidth + cellMarginX;
                    y_spacing_mod+= cellSpacingY;
                }
                x+= cellSpacingX;
                y+= cellHeight + cellMarginY;
            }
            break;
        }
        case InfiniteHorizontal:{
            int x = startOffsetX + (cellSpacingX * gridY < 0 ? abs(cellSpacingX * gridY) : 0);
            int y = startOffsetY + (cellSpacingY * gridX < 0 ? abs(cellSpacingY * gridX) : 0);
            // Start off on offset
            item_iterator += offset * gridY;
            for (int column = 0; column < gridX; ++column){
                int x_spacing_mod = x;
                int y_spacing_mod = y;
                for (int row = 0; row < gridY; ++row){
                    if (item_iterator != items.end()){
                        Util::ReferenceCount<SelectItem> item = *item_iterator;
                        if (item->isEmpty()){
                            if (drawEmpty){
                                item->draw(x_spacing_mod, y_spacing_mod, cellWidth, cellHeight, where, font);
                            }
                        } else {
                            item->draw(x_spacing_mod, y_spacing_mod, cellWidth, cellHeight, where, font);
                        }
                        item_iterator++;
                    }
                    x_spacing_mod+= cellSpacingX;
                    y_spacing_mod+= cellSpacingY + cellHeight + cellMarginY;
                }
                x+= cellWidth + cellMarginX;
                y+= cellSpacingY;
            }
            break;
        }
        case InfiniteVertical:{
            int x = startOffsetX + (cellSpacingX * gridY < 0 ? abs(cellSpacingX * gridY) : 0);
            int y = startOffsetY + (cellSpacingY * gridX < 0 ? abs(cellSpacingY * gridX) : 0);
            // Start off on offset
            item_iterator += offset * gridX;
            for (int row = 0; row < gridY; ++row){
                int x_spacing_mod = x;
                int y_spacing_mod = y;
                for (int column = 0; column < gridX; ++column){
                    if (item_iterator != items.end()){
                        Util::ReferenceCount<SelectItem> item = *item_iterator;
                        if (item->isEmpty()){
                            if (drawEmpty){
                                item->draw(x_spacing_mod, y_spacing_mod, cellWidth, cellHeight, where, font);
                            }
                        } else {
                            item->draw(x_spacing_mod, y_spacing_mod, cellWidth, cellHeight, where, font);
                        }
                        item_iterator++;
                    }
                    x_spacing_mod+=cellSpacingX + cellWidth + cellMarginX;
                    y_spacing_mod+=cellSpacingY;
                }
                x+= cellSpacingX;
                y+= cellHeight + cellMarginY;
            }
            break;
        }
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

const Util::ReferenceCount<SelectItem> GridSelect::getItem(unsigned int index) const{
    if (index >= items.size()){
        return Util::ReferenceCount<SelectItem>();
    }
    return items[index];
}

const Util::ReferenceCount<SelectItem> GridSelect::getItemByCursor(int cursor) const{
    if (getCurrentIndex(cursor) >= items.size()){
        return Util::ReferenceCount<SelectItem>();
    }
    return items[getCurrentIndex(cursor)];
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

void GridSelect::setStartingOffset(int x, int y){
    startOffsetX = x;
    startOffsetY = y;
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

static int computeOffset(int location, int width, int height){
    int large = 0;
    int small = 0;
    if (width == height){
        large = small = width;
    } else if (width > height){
        small = width;
        large = height;
    } else if (width < height){
        small = height;
        large = width;
    }
    int offset = (location/large - small) + 1;
    if (offset < 0){
        return 0;
    }
    return offset;
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
        case InfiniteHorizontal:{
            int location = cursors[cursor];
            location--;
            if (location < 0){
                if (allowWrap){
                    location = items.size()-1;
                    offset = computeOffset(location, gridX, gridY);
                } else {
                    location = 0;
                }
            } else {
                if ((unsigned int)location < offset * gridY){
                    offset--;
                }
            }
            cursors[cursor] = location;
            break;
        }
        case InfiniteVertical:{
            int location = cursors[cursor];
            location-=gridX;
            if (location < 0){
                if (allowWrap){
                    location = items.size()-1;
                    offset = computeOffset(location, gridX, gridY);
                } else {
                    location = cursors[cursor];
                }
            } else {
                if ((unsigned int)location < offset * gridX){
                    offset--;
                }
            }
            cursors[cursor] = location;
            break;
        }
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
                unsigned int location = cursors[cursor] + gridX;
                if (location >= items.size()){
                    location = items.size()-1;
                }
                cursors[cursor] = location;
            }
            break;
        }
        case InfiniteHorizontal:{
            int location = cursors[cursor];
            location++;
            if ((unsigned int)location >= items.size()){
                if (allowWrap){
                    location = offset = 0;
                }
            } else {
                if ((unsigned int)location > ((offset+gridX) * gridY)-1){
                    offset++;
                }
            }
            cursors[cursor] = location;
            break;
        }
        case InfiniteVertical:{
            int location = cursors[cursor];
            location+=gridX;
            if ((unsigned int)location >= items.size()){
                if (cursors[cursor] < items.size()-1){
                    location = items.size()-1;
                    offset = computeOffset(location, gridX, gridY);
                } else if (allowWrap){
                    location = offset = 0;
                }
            } else {
                if ((unsigned int)location > ((offset+gridY) * gridX)-1){
                    offset++;
                }
            }
            cursors[cursor] = location;
            break;
        }
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
                    unsigned int location = cursors[cursor] += gridX-1;
                    if (location >= items.size()){
                        location = items.size()-1;
                    }
                    cursors[cursor] = location;
                }
            } else {
                cursors[cursor]--;
            }
            break;
        }
        case InfiniteHorizontal:{
            int location = cursors[cursor];
            location-=gridY;
            if (location < 0){
                if (allowWrap){
                    location = items.size()-1;
                    offset = computeOffset(location, gridX, gridY);
                } else {
                    location = cursors[cursor];
                }
            } else {
                if ((unsigned int)location < offset * gridY){
                    offset--;
                }
            }
            cursors[cursor] = location;
            break;
        }
        case InfiniteVertical:{
            int location = cursors[cursor];
            location--;
            if (location < 0){
                if (allowWrap){
                    location = items.size()-1;
                    offset = computeOffset(location, gridX, gridY);
                } else {
                    location = 0;
                }
            } else {
                if ((unsigned int)location < offset * gridX){
                    offset--;
                }
            }
            cursors[cursor] = location;
            break;
        }
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
                unsigned int location = cursors[cursor]+1;
                if (location >= items.size()){
                    if (allowWrap){
                        location = (gridX * gridY) - gridX;
                    } else {
                        location = items.size()-1;
                    }
                }
                cursors[cursor] = location;
            }
            break;
        }
        case InfiniteHorizontal:{
            int location = cursors[cursor];
            location+=gridY;
            if ((unsigned int)location >= items.size()){
                if (cursors[cursor] < items.size()-1){
                    location = items.size()-1;
                    offset = computeOffset(location, gridX, gridY);
                } else if (allowWrap){
                    location = offset = 0;
                }
            } else {
                if ((unsigned int)location > ((offset+gridX) * gridY)-1){
                    offset++;
                }
            }
            cursors[cursor] = location;
            break;
        }
        case InfiniteVertical:{
            int location = cursors[cursor];
            location++;
            if ((unsigned int)location >= items.size()){
                if (allowWrap){
                    location = offset = 0;
                }
            } else {
                if ((unsigned int)location > ((offset+gridY) * gridX)-1){
                    offset++;
                }
            }
            cursors[cursor] = location;
            break;
        }
        default:
            break;
    }
    return false;
}


bool GridSelect::hasMoreLow() const{
    return (offset > 0);
}

bool GridSelect::hasMoreHigh() const{
    if (!cursors.empty()){
        switch (layout){
            case InfiniteHorizontal:{
                const unsigned int location = (offset * gridY) + (gridX * gridY);
                if (location < items.size() && location > ((offset+gridX) * gridY)-1){
                    return true;
                }
                break;
            }
            case InfiniteVertical:{
                const unsigned int location = (offset * gridX) + (gridX * gridY);
                if (location < items.size() && location > ((offset+gridY) * gridX)-1){
                    return true;
                }
                break;
            }
            case Static:
            default:
                break;
        }
    }
    return false;
}

int GridSelect::getWidth(){
    return (cellWidth+cellMarginX+cellSpacingX) * gridX;
}

int GridSelect::getHeight(){
    return (cellHeight+cellMarginY+cellSpacingY) * gridY;
}

bool GridSelect::checkCursor(int cursor) const {
    return ((unsigned int)cursor >= cursors.size());
}