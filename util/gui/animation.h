#ifndef _paintown_gui_animation_h
#define _paintown_gui_animation_h

#include <vector>
#include <map>
#include <string>

#include "coordinate.h"
#include "util/pointer.h"
#include "util/gradient.h"

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
typedef std::map<int, Util::ReferenceCount<Graphics::Bitmap> > ImageMap;

/* TODO: move most of this class definition to ImageFrame */
class Frame{
public:
    Frame(const Token *token, ImageMap &images, const std::string & baseDir);
    Frame(Util::ReferenceCount<Graphics::Bitmap> bmp);
    virtual ~Frame();
    virtual void act(double xvel, double yvel);
    virtual void draw(int xaxis, int yaxis, const Graphics::Bitmap &);
    virtual void draw(const Graphics::Bitmap &);
    virtual void reset();
    virtual void setToEnd(const RelativePoint &);
    virtual const std::string getInfo();

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
    virtual void parseToken(const Token * token, const std::string & baseDir, ImageMap & map);

protected:
    Util::ReferenceCount<Graphics::Bitmap> bmp;
    RelativePoint offset;
    RelativePoint scrollOffset;
    int time;
    bool horizontalFlip;
    bool verticalFlip;
    int alpha;
};

class TextFrame: public Frame {
public:
    /* 'map' and 'baseDir' are vestigal parameters only needed for Frame. get rid
     * of them at some point.
     */
    TextFrame(const Token *token, ImageMap & map, const std::string & baseDir);
    virtual ~TextFrame();
    
    virtual void act(double xvel, double yvel);
    virtual void draw(int xaxis, int yaxis, const Graphics::Bitmap &);
    virtual void draw(const Graphics::Bitmap &);

protected:
    virtual void parseToken(const Token * token, const std::string & baseDir, ImageMap & map);

    /* FIXME: default this to Globals::DEFAULT_FONT */
    std::string font;
    std::string message;
    int fontWidth, fontHeight;
    Effects::Gradient gradient;
};

/* Iterates over a series of items */
class Sequence{
public:
    Sequence();

    virtual Util::ReferenceCount<Frame> getCurrentFrame() const = 0;
    virtual int totalTicks() const = 0;
    virtual void setToEnd() = 0;

    /* Move the sequence along by the number of ticks and at the specified speed
     * Returns true if the sequence can't move any farther.
     */ 
    virtual bool forward(int tickCount, double velocityX, double velocityY) = 0;
    virtual bool reverse(int tickCount, double velocityX, double velocityY) = 0;
    
    virtual void draw(int xaxis, int yaxis, const Graphics::Bitmap &) = 0;

    virtual void reset() = 0;
    virtual void resetTicks() = 0;

    /* Forcifully move to the next/previous frame */
    virtual void forwardFrame() = 0;
    virtual void backFrame() = 0;

    virtual ~Sequence();
};

class SequenceFrame: public Sequence {
public:
    SequenceFrame(const Util::ReferenceCount<Frame> & frame);
    virtual Util::ReferenceCount<Frame> getCurrentFrame() const;

    virtual int totalTicks() const;
    virtual void reset();
    virtual void resetTicks();
    virtual void setToEnd();
    
    virtual void draw(int xaxis, int yaxis, const Graphics::Bitmap &);

    /* Move the sequence along by the number of ticks and at the specified speed */
    virtual bool forward(int tickCount, double velocityX, double velocityY);
    virtual bool reverse(int tickCount, double velocityX, double velocityY);

    /* Forcifully move to the next/previous frame */
    virtual void forwardFrame();
    virtual void backFrame();

protected:
    Util::ReferenceCount<Frame> frame;
    int ticks;
};

/* Shows sequences in a loop */
class SequenceLoop: public Sequence {
public:
    SequenceLoop(int loops);
    
    virtual Util::ReferenceCount<Frame> getCurrentFrame() const;

    virtual void reset();
    virtual void resetTicks();
    virtual void setToEnd();
    virtual void addSequence(const Util::ReferenceCount<Sequence> & sequence);
    virtual void parse(const Token * token, ImageMap & map, const std::string & baseDir);
    
    virtual void draw(int xaxis, int yaxis, const Graphics::Bitmap &);
    
    virtual int totalTicks() const;

    /* Move the sequence along by the number of ticks and at the specified speed */
    virtual bool forward(int tickCount, double velocityX, double velocityY);
    virtual bool reverse(int tickCount, double velocityX, double velocityY);

    /* Forcifully move to the next/previous frame */
    virtual void forwardFrame();
    virtual void backFrame();

protected:
    void resetChildrenTicks();
    virtual Util::ReferenceCount<Sequence> getCurrentSequence() const;

    /* The current frame to display */
    unsigned int currentFrame;

    /* The number of times left to loop */
    unsigned int currentLoop;

    /* The total number of times to loop */
    const unsigned int loopTimes;

    std::vector<Util::ReferenceCount<Sequence> > frames;
};

/* Shows all sequences simaltaneously */
class SequenceAll: public Sequence {
public:
    SequenceAll(const Token * token, ImageMap & images, const std::string & baseDir);
    
    virtual Util::ReferenceCount<Frame> getCurrentFrame() const;

    virtual void reset();
    virtual void resetTicks();
    virtual void setToEnd();
    virtual void addSequence(const Util::ReferenceCount<Sequence> & sequence);
    
    virtual void draw(int xaxis, int yaxis, const Graphics::Bitmap &);
    
    virtual int totalTicks() const;

    /* Move the sequence along by the number of ticks and at the specified speed */
    virtual bool forward(int tickCount, double velocityX, double velocityY);
    virtual bool reverse(int tickCount, double velocityX, double velocityY);

    /* Forcifully move to the next/previous frame */
    virtual void forwardFrame();
    virtual void backFrame();

protected:
    std::vector<Util::ReferenceCount<Sequence> > sequences;
    typedef std::vector<Util::ReferenceCount<Sequence> >::iterator SequenceIterator;
    typedef std::vector<Util::ReferenceCount<Sequence> >::const_iterator SequenceConstIterator;
};

/* Displays a random child node for its duration */
class SequenceRandom: public Sequence {
public:
    SequenceRandom(const Token * token, ImageMap & images, const std::string & baseDir);
    
    virtual Util::ReferenceCount<Frame> getCurrentFrame() const;

    virtual void reset();
    virtual void resetTicks();
    virtual void setToEnd();
    virtual void addSequence(const Util::ReferenceCount<Sequence> & sequence);
    
    virtual void draw(int xaxis, int yaxis, const Graphics::Bitmap &);
    
    virtual int totalTicks() const;

    /* Move the sequence along by the number of ticks and at the specified speed */
    virtual bool forward(int tickCount, double velocityX, double velocityY);
    virtual bool reverse(int tickCount, double velocityX, double velocityY);

    /* Forcifully move to the next/previous frame */
    virtual void forwardFrame();
    virtual void backFrame();

protected:
    unsigned int current;
    std::vector<Util::ReferenceCount<Sequence> > sequences;
    typedef std::vector<Util::ReferenceCount<Sequence> >::iterator SequenceIterator;
    typedef std::vector<Util::ReferenceCount<Sequence> >::const_iterator SequenceConstIterator;
};

class Animation{
public:
    Animation(const Token *token);
    /*! Load only a single bitmap (for backwards compatibility of backgrounds in menu) */
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
    /*! Get printable information regarding current frame */
    virtual const std::string getInfo();

    /*! Reset only frame ticks and other things are ignored */
    virtual void reset();

    /* Total number of ticks used by this animation. If any frames have a time
     * of -1 then the total time will also be -1, meaning infinity.
     */
    virtual int totalTicks() const;

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
    int id;
    Depth depth;
    unsigned int currentSequence;
    bool allowReset;
    RelativePoint axis;
    // This allows the frames to scroll in place
    RelativePoint velocity;
    Coordinate window;
    // std::vector<Util::ReferenceCount<Frame> > frames;
    SequenceLoop sequence;
    ImageMap images;
};

/*! Generalized to for re-use in other contexts (menu, cutscene, characterselect, etc) */
class AnimationManager{
public:
    AnimationManager();
    AnimationManager(const AnimationManager &);
    virtual ~AnimationManager();
    
    const AnimationManager & operator=(const AnimationManager &);
    
    void forward(int tickCount = 1);
    void reverse(int tickCount = 1);
    void act();
    void render(const Gui::Animation::Depth &, const Graphics::Bitmap &);
    
    void add(Util::ReferenceCount<Gui::Animation > animation);
    
    void reset();
    void setToEnd();

    int totalTicks() const;
    
    const std::string getInfo(int id, bool all = false);
    
    const std::vector<int> getIdList();
    
    virtual inline const bool empty() const{
        return this->animations.empty();
    }

protected:
    int countTicks(const std::vector<Util::ReferenceCount<Gui::Animation> > & toCount) const;

private:
    std::map< Gui::Animation::Depth, std::vector< Util::ReferenceCount<Gui::Animation> > > animations;
};

}
#endif
