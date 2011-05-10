#ifndef _paintown_gui_popup_box_h
#define _paintown_gui_popup_box_h

#include <string>
#include <vector>

#include "widget.h"
#include "box.h"

#include "../gradient.h"

namespace Gui{
    
class PopupBox : public Widget {
    public:
        PopupBox();
        PopupBox(const PopupBox &);
        virtual ~PopupBox();
        
        //! copy
        PopupBox &operator=(const PopupBox &);
        //! Logic
        virtual void act(const Font &);
        //! Render
        using Widget::render;
        virtual void render(const Graphics::Bitmap &);
        //! Open box
	virtual void open();
	//! Close box
	virtual void close();

        //! Is active?
	virtual inline bool isActive(){
	    return (this->fadeState != Closed);
	}

        virtual inline bool isOpen(){
            return this->fadeState == Open;
        }

        //!set fadespeed
        virtual inline void setFadeSpeed(int speed){
            this->fadeSpeed = speed;
        }
        //! Get current box coordinates
        virtual inline const Coordinate & getArea(){
            return this->board.location;
        }
    private:
	
	void doFade();
	
	enum FadeState{
	    Closed,
	    FadeIn,
	    Open,
	    FadeOut,
	};
        //! Current fade state
	FadeState fadeState;

        //! Fade speed
	int fadeSpeed;
	
	//! Board
	Box board;
};

}

#endif
