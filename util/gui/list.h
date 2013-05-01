#ifndef _gui_list_h
#define _gui_list_h

#include "widget.h"
#include "util/pointer.h"
#include "util/font.h"

namespace Gui{
class ListItem{
public:
    ListItem();
    virtual ~ListItem();
    
    virtual void act() = 0;
    virtual void draw(int x, int y, const Graphics::Bitmap &) = 0;
};

class ListInterface : public Gui::Widget{
public:
    ListInterface();
    virtual ~ListInterface();
    
    virtual void act(const Font &);
    virtual void draw(const Font &, const Graphics::Bitmap &);
    
    virtual void add(const Util::ReferenceCount<ListItem>) = 0;
    virtual void add(const std::vector< Util::ReferenceCount<ListItem> > &) = 0;
    virtual void remove(const Util::ReferenceCount<ListItem>) = 0;
    virtual void replace(const std::vector<ListItem> &) = 0;
    virtual void clear() = 0;
    virtual inline const std::vector< Util::ReferenceCount<ListItem> > & getList() const = 0;
};
}

#endif
