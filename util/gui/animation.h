#ifndef _paintown_gui_animation_h
#define _paintown_gui_animation_h

#include <vector>
#include <map>
#include <string>

#include "coordinate.h"
#include "util/pointer.h"

class Filesystem;
namespace Path{
    class AbsolutePath;
}
typedef Path::AbsolutePath AbsolutePath;

namespace Graphics{
    class Bitmap;
}

class Token;

namespace Gui{

// To hold images by number easier to access and reuse
typedef std::map< int, Util::ReferenceCount<Graphics::Bitmap> > imageMap;

class Frame{
public:
    Frame(const Token *token, imageMap &images);
    Frame(Util::ReferenceCount<Graphics::Bitmap> bmp);
    virtual ~Frame();
    virtual void act(double xvel, double yvel);
    virtual void draw(int xaxis, int yaxis, const Graphics::Bitmap &);
    virtual void draw(const Graphics::Bitmap &);
    virtual void reset();
    virtual void setToEnd(const RelativePoint &);
    virtual inline const Util::ReferenceCount<Graphics::Bitmap> & getBitmap() const {
        return this->bmp;
    }
    virtual inline const RelativePoint getOffset() const {
        return this->offset;
    }
    virtual inline const RelativePoint getScrollOffset() const {
        return this->offset;
    }
    virtual inline int getTime() const {
        return this->time;
    }
    virtual inline int getAlpha() const {
        return this->alpha;
    }
protected:
    Util::ReferenceCount<Graphics::Bitmap> bmp;
    RelativePoint offset;
    RelativePoint scrollOffset;
    int time;
    bool horizontalFlip;
    bool verticalFlip;
    int alpha;
};

class Animation{
public:
    Animation(const Token *token);
    /*! Load only a single bitmap (for bacwards compatibility of backgrounds in menu) */
    Animation(const std::string &);
    Animation(const AbsolutePath &);
    /*! use an existing bitmap */
    Animation(Util::ReferenceCount<Graphics::Bitmap> image);
    virtual ~Animation();
    /*! Reverse with ticks */
    virtual void reverse(int tickCount = 1);
    /*! Forward with ticks */
    virtual void forward(int tickCount = 1);
    /*! Logic ticking per iteration and moving to subsequent frames */
    virtual void act();
    /*! Draw */
    virtual void draw(const Graphics::Bitmap &);
    virtual void draw(int x, int y, int width, int height, const Graphics::Bitmap &);
    /*! Forward to next frame with no regard to ticks */
    virtual void forwardFrame();
    /*! Back a frame with no regard to ticks */
    virtual void backFrame();
    /*! Reset everything to the beginning of the start of the animation */
    virtual void resetAll();
    /*! Set everything to the end of the animation */
    virtual void setToEnd();
    /*! Reset only frame ticks and other things are ignored */
    virtual inline void reset(){ 
        if (allowReset){ 
            currentFrame = 0; 
        } 
    }
    /*! Return ID */
    virtual inline int getID() const { 
        return id;
    }
    /*! Depth of animation
        TODO make depth unlimited and just use integers for weight of depth
    */
    enum Depth {
        BackgroundBottom,
        BackgroundMiddle,
        BackgroundTop,
        ForegroundBottom,
        ForegroundMiddle,
        ForegroundTop,
    };
    /*! Get depth */
    inline const Depth & getDepth() const { 
        return this->depth;
    }

private:
    //! Set end ticks
    void calculateEndTicks();
    
    int id;
    Depth depth;
    int ticks;
    int endTicks;
    unsigned int currentFrame;
    unsigned int loop;
    bool allowReset;
    RelativePoint axis;
    // This allows the frames to scroll in place
    RelativePoint velocity;
    Coordinate window;
    std::vector<Util::ReferenceCount<Frame> > frames;
    imageMap images;
};

/*! Generalized to for re-use in other contexts (menu, cutscene, characterselect, etc) */
class AnimationManager{
public:
    AnimationManager();
    AnimationManager(const AnimationManager &);
    virtual ~AnimationManager();
    
    const AnimationManager & operator=(const AnimationManager &);
    
    void forward(int tickCount=1);
    void reverse(int tickCount=1);
    void act();
    void render(const Gui::Animation::Depth &, const Graphics::Bitmap &);
    
    void add(Util::ReferenceCount<Gui::Animation > animation);
    
    void reset();
    void setToEnd();
    
    virtual inline const bool empty() const{
        return this->animations.empty();
    }
private:
    std::map< Gui::Animation::Depth, std::vector< Util::ReferenceCount<Gui::Animation> > > animations;
};

}
#endif
