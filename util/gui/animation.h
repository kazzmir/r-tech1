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
    /* use an existing bitmap */
    Animation(Util::ReferenceCount<Graphics::Bitmap> image);
    virtual ~Animation();
    // Logic
    virtual void act();
    virtual void draw(const Graphics::Bitmap &);
    virtual void draw(int x, int y, int width, int height, const Graphics::Bitmap &);
    virtual void forwardFrame();
    virtual void backFrame();
    virtual void resetAll();

    inline void reset(){ if (allowReset){ currentFrame = 0; } }
    inline int getID() const { return id; }

    enum Depth {
        BackgroundBottom,
        BackgroundMiddle,
        BackgroundTop,
        ForegroundBottom,
        ForegroundMiddle,
        ForegroundTop,
    };
    inline const Depth & getDepth() const { return this->depth; }

private:
    int id;
    Depth depth;
    int ticks;
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
    
    void act();
    void render(const Gui::Animation::Depth &, const Graphics::Bitmap &);
    
    void add(Util::ReferenceCount<Gui::Animation > animation);
    
    void reset();
    
    virtual inline const bool empty() const{
        return this->animations.empty();
    }
private:
    std::map< Gui::Animation::Depth, std::vector< Util::ReferenceCount<Gui::Animation> > > animations;
};

}
#endif
