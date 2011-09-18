#ifndef _paintown_gui_context_box_h
#define _paintown_gui_context_box_h

#include <string>
#include <vector>

#include "widget.h"
#include "popup-box.h"
#include "scroll-list.h"

#include "../gradient.h"
#include "../file-system.h"

#include "util/pointer.h"

namespace Gui{

Effects::Gradient standardGradient();

class ContextBox;
class ScrollListInterface;
class ContextItem: public ScrollItem {
public:
    ContextItem(const ContextBox & parent);
    virtual ~ContextItem();
    
    virtual const std::string getName() const = 0;
    virtual bool isAdjustable();
    virtual int getLeftColor();
    virtual int getRightColor();
    virtual void draw(int x, int y, const Graphics::Bitmap & where, const Font & font, int distance) const;
    virtual int size(const Font & font) const;
protected:
    const ContextBox & parent;
};

class ContextBox: public Widget {
    public:
        ContextBox();
        ContextBox(const ContextBox &);
        virtual ~ContextBox();
        
        //! copy
        ContextBox &operator=(const ContextBox &);
        //! Logic
        virtual void act(const Font &);
        //! Render
        using Widget::render;
        virtual void render(const Graphics::Bitmap &);
        virtual void render(const Graphics::Bitmap &, const Font & font);
        //! Next
        virtual bool next(const Font &);
        //! Previous
        virtual bool previous(const Font &);
	//! Adjust left
	virtual void adjustLeft();
	//! Adjust right
	virtual void adjustRight();
	//! open context box
	virtual void open();
	//! Close context box
	virtual void close();
        //! Set context list
        virtual void setList(const std::vector<Util::ReferenceCount<ContextItem> > & list);
        virtual void addItem(const Util::ReferenceCount<ContextItem> & item);
        
        /*! Scroll or Normal */
        enum ListType{
            Normal,
            Scroll,
        };
        virtual void setListType(const ListType &);
        virtual void setListWrap(bool wrap);
        
        virtual Graphics::Color getSelectedColor() const;

        /*
        //! Set current font
	virtual inline void setFont(const Filesystem::RelativePath & font, int width, int height){
	    this->font = font;
	    this->fontWidth = width;
	    this->fontHeight = height;
	}
        */
        //! Get current index
        virtual inline unsigned int getCurrentIndex(){
            return this->list->getCurrentIndex();
        }
        //! Is active?
	virtual inline bool isActive(){
	    return (this->fadeState != NotActive);
	}
        //!set fadespeed
        virtual inline void setFadeSpeed(int speed){
            this->fadeSpeed = speed;
            this->board.setFadeSpeed(speed);
        }
        //!set fade alpha
        virtual inline void setFadeAlpha(int alpha){
            this->fadeAlpha = alpha;
        }

        virtual inline int getFadeAlpha() const {
            return this->fadeAlpha;
        }
        
        //! use gradient?
        virtual inline void setUseGradient(bool useGradient){
            this->useGradient = useGradient;
        }
        
        //! Set to use only text on background
        virtual inline void setRenderOnlyText(bool render){
	    this->renderOnlyText = render;
	}

        virtual PopupBox & getBoard(){
            return board;
        }

    private:
	
	void doFade();
	
	void calculateText(const Font & font);
	
        void doDraw(int x, int y, int middle_x, int min_y, int max_y, const Font & font, int current, int selected, const Graphics::Bitmap & area, int direction);
	void drawText(const Graphics::Bitmap &, const Font & font);
	
	enum FadeState{
	    NotActive,
	    FadeIn,
	    Active,
	    FadeOut,
	};
        //! Current index
        // unsigned int current;
	
	//! Current fade state
	FadeState fadeState;

        //! Context list
        // std::vector<ContextItem *> context;
        //ScrollList list;
        Util::ReferenceCount<ScrollListInterface> list;
	
	//! Current font
        /*
        Filesystem::RelativePath font;
	int fontWidth;
	int fontHeight;
        */
	
	//! Fade speed
	int fadeSpeed;
	
	//! Fade Aplha
	int fadeAlpha;
	
	//! Board
	PopupBox board;
	
        //! The centered position 
        int cursorCenter;
        //! Current y coordinate to render text from
        int cursorLocation;
        //! scroll wait
        int scrollWait;
        
        //! Gradient for selected cursor
        Effects::Gradient selectedGradient;

        //! Use gradient
        bool useGradient;
	
	//! Render Text only
	bool renderOnlyText;
};

}

#endif
