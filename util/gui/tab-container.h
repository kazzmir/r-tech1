#ifndef _gui_tab_container_h
#define _gui_tab_container_h

#include <string>
#include <vector>

#include "widget.h"
#include "util/file-system.h"
#include "util/pointer.h"
#include "util/graphics/gradient.h"

class Token;

namespace Gui{

class TabItem{
public:
    TabItem();
    TabItem(const std::string &);
    virtual ~TabItem();
    virtual void act() = 0;
    virtual void draw(const Font&, const Graphics::Bitmap &) = 0;
    virtual inline void setName(const std::string & name) {
        this->name = name;
    } 
    virtual inline const std::string & getName() const {
        return this->name;
    } 
    virtual inline bool isActive() const {
        return this->active;
    }
    virtual inline void toggleActive() {
        this->active = !this->active;
    }
protected:
    bool active;
    std::string name;    
};

class DummyTab: public TabItem{
public:
    DummyTab(const std::string &);
    virtual ~DummyTab();
    virtual void act();
    virtual void draw(const Font&, const Graphics::Bitmap &);
};
    

class TabContainer: public Widget {
public:
    TabContainer();
    TabContainer(const TabContainer &);
    virtual ~TabContainer();
        
    // copy
    TabContainer & operator=(const TabContainer &);
        
    // Logic
    virtual void act(const Font &);
    
    // Render
    using Widget::render;
    virtual void render(const Graphics::Bitmap &);
    virtual void draw(const Font &, const Graphics::Bitmap &);
    
    // Add
    virtual void add(Util::ReferenceCount<TabItem> tab);
	    
    // Empty
    virtual inline bool empty() const {
        return this->tabs.empty();
    }
	
protected:

    virtual void drawTabs(const Font &, const Graphics::Bitmap &);

    std::vector< Util::ReferenceCount<TabItem> > tabs;
    
    unsigned int current;
    
    /*! Gradient for active selection */
    Effects::Gradient  * activeColor;
};

}

#endif
