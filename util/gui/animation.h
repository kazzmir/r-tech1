#ifndef _paintown_gui_animation_h
#define _paintown_gui_animation_h

#include <vector>
#include <map>

#include "../load_exception.h"
#include "coordinate.h"

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
typedef std::map< int, Graphics::Bitmap *> imageMap;

class Frame{
public:
    Frame(const Token *token, imageMap &images) throw (LoadException);
    Frame(Graphics::Bitmap *);
    virtual ~Frame();
    virtual void act(double xvel, double yvel);
    virtual void draw(int xaxis, int yaxis, const Graphics::Bitmap &);
    Graphics::Bitmap *bmp;
    RelativePoint offset;
    RelativePoint scrollOffset;
    int time;
    bool horizontalFlip;
    bool verticalFlip;
    int alpha;
};

class Animation{
public:
    Animation(const Token *token) throw (LoadException);
    /*! Load only a single bitmap (for bacwards compatibility of backgrounds in menu) */
    Animation(const std::string &) throw (LoadException);
    Animation(const AbsolutePath &) throw (LoadException);
    /* use an existing bitmap */
    Animation(Graphics::Bitmap * image);
    virtual ~Animation();
    // Logic
    virtual void act();
    virtual void draw(const Graphics::Bitmap &);
    virtual void forwardFrame();
    virtual void backFrame();

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
    std::vector<Frame *> frames;
    imageMap images;
};
}
#endif
