#include "animation.h"

#include <stdio.h>
#include <vector>
#include <math.h>
#include <sstream>
#include "util/token.h"
#include "util/trans-bitmap.h"
#include "util/bitmap.h"
#include "util/stretch-bitmap.h"
#include "util/font.h"
#include "globals.h"
#include "../debug.h"
#include "../funcs.h"
#include "../file-system.h"
#include "../load_exception.h"

using namespace std;
using namespace Gui;

// Temporary solution
static void renderSprite(const Graphics::Bitmap & bmp, const int x, const int y, const int alpha, const bool hflip, const bool vflip, const Graphics::Bitmap & work){
    if (alpha != 255){
        Graphics::Bitmap::transBlender( 0, 0, 0, alpha );
        if (hflip && !vflip){
            bmp.translucent().drawHFlip(x,y, work);
        } else if (!hflip && vflip){
            bmp.translucent().drawVFlip(x,y, work);
        } else if (hflip && vflip){
            bmp.translucent().drawHVFlip(x,y, work);
        } else if (!hflip && !vflip){
            bmp.translucent().draw(x,y, work);
        }
    } else {
        if (hflip && !vflip){
            bmp.drawHFlip(x,y, work);
        } else if (!hflip && vflip){
            bmp.drawVFlip(x,y, work);
        } else if (hflip && vflip){
            bmp.drawHVFlip(x,y, work);
        } else if (!hflip && !vflip){
            bmp.draw(x,y, work);
        }
    }
}

Frame::Frame(const Token *the_token, ImageMap &images, const string & baseDir):
bmp(0),
time(0),
horizontalFlip(false),
verticalFlip(false),
alpha(255){
    /*
    if (*the_token != "frame"){
        throw LoadException(__FILE__, __LINE__, "Not an frame");
    }
    */
    /* The usual setup of an animation frame is
    // use image -1 to not draw anything, it can be used to get a blinking effect
    (frame (image NUM) (alpha NUM) (offset x y) (hflip 0|1) (vflip 0|1) (time NUM))
    */
    TokenView view = the_token->view();
    while (view.hasMore()){
        try{
            const Token * token;
            view >> token;
            parseToken(token, baseDir, images);
        } catch ( const TokenException & ex ) {
            throw LoadException(__FILE__, __LINE__, ex, "Menu animation parse error");
        } catch ( const LoadException & ex ) {
            throw ex;
        }
    }
}
    
void Frame::parseToken(const Token * token, const string & baseDir, ImageMap & images){
    if (*token == "image"){
        // get the number
        string maybeNumber;
        token->view() >> maybeNumber;
        if (Util::matchRegex("[0-9]+", maybeNumber)){
            int num;
            token->view() >> num;
            if (images.find(num) == images.end()){
                ostringstream out;
                out << "No image for index " << num;
                throw LoadException(__FILE__, __LINE__, out.str());
            }
            // now assign the bitmap
            bmp = images[num];
        } else {
            bmp = Util::ReferenceCount<Graphics::Bitmap>(new Graphics::Bitmap(Storage::instance().find(Filesystem::RelativePath(baseDir + "/" + maybeNumber)).path()));
        }
    } else if (*token == "alpha"){
        // get alpha
        token->view() >> alpha;
    } else if (*token == "offset"){
        // Get the offset location it defaults to 0,0
        double x=0, y=0;
        try {
            token->view() >> x >> y;
        } catch (const TokenException & ex){
        }
        offset.set(x,y);
    } else if (*token == "hflip"){
        // horizontal flip
        token->view() >> horizontalFlip;
    } else if (*token == "vflip"){
        // horizontal flip
        token->view() >> verticalFlip;
    } else if (*token == "time"){
        // time to display
        token->view() >> time;
    } else {
        Global::debug( 3 ) << "Unhandled menu attribute: "<<endl;
        if (Global::getDebug() >= 3){
            token->print(" ");
        }
    }
}

Frame::Frame(Util::ReferenceCount<Graphics::Bitmap> bmp):
bmp(bmp),
time(0),
horizontalFlip(false),
verticalFlip(false),
alpha(255){
}

Frame::~Frame(){
}

void Frame::act(double xvel, double yvel){
    scrollOffset.moveBy(xvel, yvel);
    if (bmp != NULL){
        if (scrollOffset.getDistanceFromCenterX() >= bmp->getWidth()){
            scrollOffset.setX(0);
        } else if (scrollOffset.getDistanceFromCenterX() <= -(bmp->getWidth())){
            scrollOffset.setX(0);
        }
        if (scrollOffset.getDistanceFromCenterY() >= bmp->getHeight()){
            scrollOffset.setY(0);
        } else if (scrollOffset.getDistanceFromCenterY() <= -(bmp->getHeight())){
            scrollOffset.setY(0);
        }
    }
}
    
static bool closeFloat(double a, double b){
    const double epsilon = 0.001;
    return fabs(a-b) < epsilon;
}

void Frame::draw(const int xaxis, const int yaxis, const Graphics::Bitmap & work){
    if (!bmp){
        return;
    }

    if (!closeFloat(scrollOffset.getDistanceFromCenterX(), 0) || !closeFloat(scrollOffset.getDistanceFromCenterY(), 0)){

        // Lets do some scrolling
        // Graphics::Bitmap temp = Graphics::Bitmap::temporaryBitmap(bmp->getWidth(), bmp->getHeight());
        //AnimationPoint loc;
        AbsolutePoint loc;
        if (scrollOffset.getRelativeX() < 0){
            loc.setX(scrollOffset.getDistanceFromCenterX() + bmp->getWidth());
        } else if (scrollOffset.getRelativeX() > 0){
            loc.setX(scrollOffset.getDistanceFromCenterX() - bmp->getWidth());
        }
        if (scrollOffset.getRelativeY() < 0){
            loc.setY(scrollOffset.getDistanceFromCenterY() + bmp->getHeight());
        } else if (scrollOffset.getRelativeY() > 0){
            loc.setY(scrollOffset.getDistanceFromCenterY() - bmp->getHeight());
        }

        /*
        bmp->Blit((int) scrollOffset.getDistanceFromCenterX(), (int) scrollOffset.getDistanceFromCenterY(), temp);
        bmp->Blit((int) scrollOffset.getDistanceFromCenterX(), (int) loc.getY(), temp);
        bmp->Blit((int) loc.getX(), (int) scrollOffset.getDistanceFromCenterY(), temp);
        bmp->Blit((int) loc.getX(), (int) loc.getY(), temp);

        renderSprite(temp, (int)(xaxis+offset.getDistanceFromCenterX()), (int)(yaxis+offset.getDistanceFromCenterY()), alpha, horizontalFlip, verticalFlip, work);
        */

        double x = xaxis+offset.getDistanceFromCenterX();
        double y = yaxis+offset.getDistanceFromCenterY();

        renderSprite(*bmp,
                    (int)(x + scrollOffset.getDistanceFromCenterX()),
                    (int)(y + scrollOffset.getDistanceFromCenterY()),
                    alpha, horizontalFlip, verticalFlip, work);

        renderSprite(*bmp,
                    (int)(x + loc.getX()),
                    (int)(y + scrollOffset.getDistanceFromCenterY()),
                    alpha, horizontalFlip, verticalFlip, work);

        renderSprite(*bmp,
                    (int)(x + scrollOffset.getDistanceFromCenterX()),
                    (int)(y + loc.getY()),
                    alpha, horizontalFlip, verticalFlip, work);

        renderSprite(*bmp,
                    (int)(x + loc.getX()),
                    (int)(y + loc.getY()),
                    alpha, horizontalFlip, verticalFlip, work);


    } else {
        renderSprite(*bmp, (int)(xaxis+offset.getDistanceFromCenterX()), (int)(yaxis+offset.getDistanceFromCenterY()), alpha, horizontalFlip, verticalFlip, work);
    }
}

/* This is mainly called from select-list to draw the cells. I'm not sure
 * of the utility of stretching the bitmap to fit the cell dimensions. Most
 * likely the cell will contain an image and the image will be the same
 * size as the cell.
 */
void Frame::draw(const Graphics::Bitmap & work){
    const Graphics::Bitmap & temp = Graphics::Bitmap::temporaryBitmap(bmp->getWidth(), bmp->getHeight());
    temp.clearToMask();
    renderSprite(*bmp, 0, 0, alpha, horizontalFlip, verticalFlip, temp);
    temp.drawStretched(work);
    
    /* FIXME: This should work, but it doesn't.. */
    /*
    Graphics::StretchedBitmap show(bmp->getWidth(), bmp->getHeight(), work);
    show.start();
    renderSprite(*bmp, 0, 0, alpha, horizontalFlip, verticalFlip, show);
    show.finish();
    */
}

void Frame::reset(){
    scrollOffset = RelativePoint();
}

void Frame::setToEnd(const RelativePoint & end){
    scrollOffset = end;
}

static const char * FRAME_TEXT = "Offset: ( %f, %f)\nScroll Offset: ( %f, %f)\nTime: %d\nHorizontal Flip: %d\nVertical Flip: %d\nAlpha: %d\n\n";

const std::string Frame::getInfo(){
    char info[255];
    sprintf(info, FRAME_TEXT, offset.getRelativeX(), offset.getRelativeY(), scrollOffset.getRelativeX(), scrollOffset.getRelativeY(), time, horizontalFlip, verticalFlip, alpha);
    return std::string(info);
}

TextFrame::TextFrame(const Token *token, ImageMap & map, const string & baseDir):
Frame(token, map, baseDir){
    TokenView view = token->view();
    while (view.hasMore()){
        try{
            const Token * token;
            view >> token;
            parseToken(token, baseDir, map);
        } catch ( const TokenException & ex ) {
            throw LoadException(__FILE__, __LINE__, ex, "Menu animation parse error");
        } catch ( const LoadException & ex ) {
            throw ex;
        }
    }
}

TextFrame::~TextFrame(){
}
    
void TextFrame::draw(int xaxis, int yaxis, const Graphics::Bitmap & work){
    double x = xaxis + offset.getDistanceFromCenterX() + scrollOffset.getDistanceFromCenterX();
    double y = yaxis + offset.getDistanceFromCenterY() + scrollOffset.getDistanceFromCenterY();

    const Font & font = Font::getFont(Filesystem::RelativePath(this->font), 20, 20);
    font.printf((int) x, (int) y, Graphics::makeColor(255, 255, 255), work, "%s", 0, message.c_str());
}

void TextFrame::draw(const Graphics::Bitmap & work){
    /* Probably don't need this.. but implement it if you do! */
}

void TextFrame::parseToken(const Token * token, const string & baseDir, ImageMap & map){
    if (*token == "message"){
        token->view() >> message;
    } else if (*token == "font"){
        token->view() >> font;
    }
}

Sequence::Sequence(){
}

Sequence::~Sequence(){
}

SequenceFrame::SequenceFrame(const Util::ReferenceCount<Frame> & frame):
frame(frame),
ticks(0){
}

Util::ReferenceCount<Frame> SequenceFrame::getCurrentFrame() const {
    return frame;
}

int SequenceFrame::totalTicks() const {
    return frame->getTime();
}

bool SequenceFrame::forward(int tickCount, double velocityX, double velocityY){
    frame->act(tickCount * velocityX, tickCount * velocityY);
    ticks += tickCount;
    return ticks > frame->getTime();
}

bool SequenceFrame::reverse(int tickCount, double velocityX, double velocityY){
    frame->act(-tickCount * velocityX, -tickCount * velocityY);
    ticks -= tickCount;
    return ticks < 0;
}

void SequenceFrame::forwardFrame(){
    /* nop */
}

void SequenceFrame::backFrame(){
    /* nop */
}
    
void SequenceFrame::reset(){
    ticks = 0;
    frame->reset();
}

void SequenceFrame::resetTicks(){
    ticks = 0;
}

SequenceLoop::SequenceLoop(int loops):
currentFrame(0),
currentLoop(loops),
loopTimes(loops){
}
    
Util::ReferenceCount<Frame> SequenceLoop::getCurrentFrame() const {
    if (currentFrame < frames.size()){
        return frames[currentFrame]->getCurrentFrame();
    } else {
        /* Return the last frame */
        if (frames.size() > 0){
            return frames[frames.size() - 1]->getCurrentFrame();
        }
    }
    return Util::ReferenceCount<Frame>(NULL);
}
    
Util::ReferenceCount<Sequence> SequenceLoop::getCurrentSequence() const {
    if (currentFrame < frames.size()){
        return frames[currentFrame];
    } else {
        /* Return the last frame */
        if (frames.size() > 0){
            return frames[frames.size() - 1];
        }
    }
    return Util::ReferenceCount<Sequence>(NULL);

}

void SequenceLoop::reset(){
    currentFrame = 0;
    currentLoop = loopTimes;
    for (vector<Util::ReferenceCount<Sequence> >::iterator it = frames.begin(); it != frames.end(); it++){
        Util::ReferenceCount<Sequence> & sequence = *it;
        sequence->reset();
    }
}

/* Does the same thing as reset except for calling resetTicks on children nodes */
void SequenceLoop::resetTicks(){
    currentFrame = 0;
    currentLoop = loopTimes;
    for (vector<Util::ReferenceCount<Sequence> >::iterator it = frames.begin(); it != frames.end(); it++){
        Util::ReferenceCount<Sequence> & sequence = *it;
        sequence->resetTicks();
    }
}

/* Similar to resetTicks but doesn't reset the current frame or loop */
void SequenceLoop::resetChildrenTicks(){
    for (vector<Util::ReferenceCount<Sequence> >::iterator it = frames.begin(); it != frames.end(); it++){
        Util::ReferenceCount<Sequence> & sequence = *it;
        sequence->resetTicks();
    }
}

void SequenceLoop::setToEnd(){
    /* FIXME */
}

void SequenceLoop::addSequence(const Util::ReferenceCount<Sequence> & sequence){
    frames.push_back(sequence);
}

void SequenceLoop::addSequence(const Util::ReferenceCount<SequenceFrame> & sequence){
    addSequence(sequence.convert<Sequence>());
}

void SequenceLoop::addSequence(const Util::ReferenceCount<SequenceLoop> & sequence){
    addSequence(sequence.convert<Sequence>());
}

void SequenceLoop::parse(const Token * token, ImageMap & images, const string & baseDir){
    TokenView view = token->view();
    /* first ignore the number of times to loop */
    int ignore;
    view >> ignore;

    while (view.hasMore()){
        const Token * next;
        view >> next;
        if (*next == "frame"){
            Util::ReferenceCount<Frame> frame(new Frame(next, images, baseDir));
            addSequence(Util::ReferenceCount<SequenceFrame>(new SequenceFrame(frame)));
        } else if (*next == "text"){
            Util::ReferenceCount<Frame> frame(new TextFrame(next, images, baseDir));
            addSequence(Util::ReferenceCount<SequenceFrame>(new SequenceFrame(frame)));
        } else if (*next == "loop"){
            int times;
            next->view() >> times;
            Util::ReferenceCount<SequenceLoop> loop(new SequenceLoop(times));
            loop->parse(next, images, baseDir);
            addSequence(loop);
        }
    }
}

int SequenceLoop::totalTicks() const {
    int total = 0;
    for (vector<Util::ReferenceCount<Sequence> >::const_iterator it = frames.begin(); it != frames.end(); it++){
        const Util::ReferenceCount<Sequence> & sequence = *it;
        total += sequence->totalTicks();
    }
    return total * (1 + loopTimes);
}

bool SequenceLoop::forward(int tickCount, double velocityX, double velocityY){
    Util::ReferenceCount<Sequence> current = getCurrentSequence();
    if (current != NULL){
        if (current->forward(tickCount, velocityX, velocityY)){
            forwardFrame();
        }
    }

    return currentFrame == frames.size();
}

bool SequenceLoop::reverse(int tickCount, double velocityX, double velocityY){
    Util::ReferenceCount<Sequence> current = getCurrentSequence();
    if (current != NULL){
        if (current->reverse(tickCount, velocityX, velocityY)){
            backFrame(); 
        }
    }

    return currentFrame == frames.size();
}

void SequenceLoop::forwardFrame(){
    currentFrame += 1;

    /* do the loop */
    if (currentFrame == frames.size() && currentLoop > 0){
        currentLoop -= 1;
        currentFrame = 0;
        resetChildrenTicks();
    }
}

void SequenceLoop::backFrame(){
    currentFrame -= 1;

    /* do the loop */
    if (currentFrame <= 0 && currentLoop > 0){
        currentLoop -= 1;
        resetChildrenTicks();
        if (frames.size() > 0){
            currentFrame = frames.size() - 1;
        } else {
            currentFrame = 0;
        }
    }
}

static int CURRENT_ID = 0;

static int getNextId(){
    return CURRENT_ID++;
}

Animation::Animation(const Token *the_token):
id(getNextId()),
depth(BackgroundBottom),
allowReset(true),
sequence(0){
    /* Whats this for? */
    images[-1] = 0;

    std::string basedir = ".";
    if ( *the_token != "anim" && *the_token != "animation" ){
        throw LoadException(__FILE__, __LINE__, "Not an animation");
    }
    /* The usual setup of an animation is
	The images must be listed prior to listing any frames, basedir can be used to set the directory where the images are located
	loop will begin at the subsequent frame listed after loop
	axis is the location in which the drawing must be placed
	location *old* - used to render in background or foreground (0 == background [default]| 1 == foreground)
    depth - used to render in background or foreground space (depth background bottom|middle|top) | (depth foreground bottom|midle|top)
	reset - used to allow resetting of animation (0 == no | 1 == yes [default])
	velocity - used to get a wrapping scroll effect while animating
    window - area in which the item will be contained
	(anim (id NUM) 
	      (location NUM)
              (scale NUM) ;; optional
              (depth background|foreground NUM)
	      (basedir LOCATION)
	      (image NUM FILE [SCALE]) 
	      (velocity x y)
	      (axis x y) 
	      (frame "Read comments above in constructor") 
	      (loop 2)
	      (reset NUM)
	      (window x1 y1 x2 y2))
    */
    double scale = 1.0;
    bool scaleSet = false;
    const Token & tok = *the_token;
    TokenView view = tok.view();
    while (view.hasMore()){
        try{
            const Token * token;
            view >> token;
            if (*token == "id"){
                // get the id
                token->view() >> id;
            } else if (*token == "location"){
                // translate location to depth
                /*int location = 0;
                token->view() >> location;
                if (location == 0){
                    depth = BackgroundBottom;
                } else if (location == 1){
                    depth = ForegroundBottom;
                }*/
                Global::debug(0) << "The option (" << token->getLineage() << ") in the file '" << token->getFileName() << "' is no longer valid and will be ignored. Consider using 'depth' instead." << std::endl;
            } else if (*token == "scale"){
                token->view() >> scale;
                scaleSet = true;
            } else if (*token == "depth"){
                // get the depth
                std::string name, level;
                token->view() >> name >> level;
                if (name == "background"){
                    if (level == "bottom"){
                        depth = BackgroundBottom;
                    } else if (level == "middle"){
                        depth = BackgroundMiddle;
                    } else if (level == "top"){
                        depth = BackgroundTop;
                    }
                } else if (name == "foreground"){
                    if (level == "bottom"){
                        depth = ForegroundBottom;
                    } else if (level == "middle"){
                        depth = ForegroundMiddle;
                    } else if (level == "top"){
                        depth = ForegroundTop;
                    }
                }
            } else if (*token == "basedir"){
                // set the base directory for loading images
                token->view() >> basedir;
            } else if (*token == "image"){
                // add bitmaps by number to the map
                int number;
                std::string temp;
                double localScale = 1;
                bool localScaleSet = false;
                TokenView view = token->view();
                view >> number >> temp;
                try{
                    view >> localScale;
                    localScaleSet = true;
                } catch (const TokenException & fail){
                }
                Util::ReferenceCount<Graphics::Bitmap> bmp(new Graphics::Bitmap(Storage::instance().find(Filesystem::RelativePath(basedir + "/" + temp)).path()));
                if (!bmp->getError()){
                    if (scaleSet || localScaleSet){
                        if (localScaleSet){
                            *bmp = bmp->scaleBy(localScale, localScale);
                        } else {
                            *bmp = bmp->scaleBy(scale, scale);
                        }
                    }
                    images[number] = bmp;
                }
            } else if (*token == "axis"){
                // Get the axis location it defaults to 0,0
                double x=0, y=0;
                try {
                    token->view() >> x >> y;
                } catch (const TokenException & ex){
                }
                axis.set(x,y);
            } else if (*token == "window"){
                // Windowed area where to clip if necessary otherwise it defaults to max
                double x1=0,x2=0, y1=0,y2=0;
                try {
                    token->view() >> x1 >> y1 >> x2 >> y2;
                } catch (const TokenException & ex){
                }
                window.set(x1,y1,x2,y2);
            } else if (*token == "velocity"){
                // This allows the animation to get a wrapping scroll action going on
                double x=0, y=0;
                try {
                    token->view() >> x >> y;
                } catch (const TokenException & ex){
                }
                velocity.set(x,y);
            } else if (*token == "frame"){
                Util::ReferenceCount<Frame> frame(new Frame(token, images, basedir));
                sequence.addSequence(Util::ReferenceCount<SequenceFrame>(new SequenceFrame(frame)));
            } else if (*token == "text"){
                Util::ReferenceCount<Frame> frame(new TextFrame(token, images, basedir));
                sequence.addSequence(Util::ReferenceCount<SequenceFrame>(new SequenceFrame(frame)));
            } else if (*token == "loop"){
                // start loop here
                int times;
                token->view() >> times;
                Util::ReferenceCount<SequenceLoop> loop(new SequenceLoop(times));
                loop->parse(token, images, basedir);
                sequence.addSequence(loop);

                /*
		if (l >= (int)frames.size()){
		    ostringstream out;
		    out << "Loop location is larger than the number of frames. Loop: " << loop << " Frames: " << frames.size();
		    throw LoadException(__FILE__, __LINE__, out.str());
		}
                */
            } else if (*token == "reset"){
                // Allow reset of animation
                token->view() >> allowReset;
            } else {
                Global::debug( 3 ) << "Unhandled menu attribute: "<<endl;
                if (Global::getDebug() >= 3){
                    token->print(" ");
                }
            }
        } catch ( const TokenException & ex ) {
            throw LoadException(__FILE__, __LINE__, ex, "Menu animation parse error");
        } catch ( const LoadException & ex ) {
            throw ex;
        }
    }
}

Animation::Animation(const std::string & background):
id(getNextId()),
depth(BackgroundBottom),
allowReset(true),
sequence(0){
    // add bitmap
    Util::ReferenceCount<Graphics::Bitmap> bmp(new Graphics::Bitmap(Storage::instance().find(Filesystem::RelativePath(background)).path()));
    if (bmp->getError()){
        throw LoadException(__FILE__,__LINE__, "Problem loading file: " + background);
    } else {
        images[0] = bmp;
    }
    Util::ReferenceCount<Frame> frame(new Frame(bmp));
    sequence.addSequence(Util::ReferenceCount<SequenceFrame>(new SequenceFrame(frame)));
}

Animation::Animation(const Filesystem::AbsolutePath & path):
id(getNextId()),
depth(BackgroundBottom),
allowReset(true),
sequence(0){
    // add bitmap
    Util::ReferenceCount<Graphics::Bitmap> bmp(new Graphics::Bitmap(path.path()));
    if (bmp->getError()){
        throw LoadException(__FILE__,__LINE__, "Problem loading file: " + path.path());
    } else {
        images[0] = bmp;
    }
    Util::ReferenceCount<Frame> frame(new Frame(bmp));
    sequence.addSequence(Util::ReferenceCount<SequenceFrame>(new SequenceFrame(frame)));
}

Animation::Animation(Util::ReferenceCount<Graphics::Bitmap> image):
id(getNextId()),
depth(BackgroundBottom),
allowReset(true),
sequence(0){
    images[0] = image;
    Util::ReferenceCount<Frame> frame(new Frame(image));
    sequence.addSequence(Util::ReferenceCount<SequenceFrame>(new SequenceFrame(frame)));
}

Animation::~Animation(){
}

int Animation::totalTicks() const {
    return sequence.totalTicks();
}

void Animation::forward(int tickCount){
    sequence.forward(tickCount, velocity.getRelativeX(), velocity.getRelativeY());
}

void Animation::reverse(int tickCount){
    sequence.reverse(tickCount, velocity.getRelativeX(), velocity.getRelativeY());
}

void Animation::act(){
    forward();
}

void Animation::draw(const Graphics::Bitmap & work){
    /* FIXME: should use sub-bitmaps here */
    /*const int x = window.getX();
    const int y = window.getY();
    const int width = window.getWidth();
    const int height = window.getHeight();
    Global::debug(0) << "Distance x2: " << width << " Distance y2: " << height << std::endl;
    Graphics::Bitmap clipped(work, x, y, height, width);
    frames[currentFrame]->draw(0, 0,clipped);*/
     // Set clip from the axis default is 0,0,bitmap width, bitmap height
    work.setClipRect(-window.getPosition().getDistanceFromCenterX(),
                     -window.getPosition().getDistanceFromCenterY(),
                     work.getWidth() - window.getPosition2().getDistanceFromCenterX(),
                     work.getHeight() - window.getPosition2().getDistanceFromCenterY());

    const Util::ReferenceCount<Frame> & frame = sequence.getCurrentFrame();
    if (frame != NULL){
        frame->draw(axis.getDistanceFromCenterX(), axis.getDistanceFromCenterY(), work);
    }

    work.setClipRect(0, 0, work.getWidth(), work.getHeight());
}

void Animation::draw(int x, int y, int width, int height, const Graphics::Bitmap & work){
    const Util::ReferenceCount<Frame> & frame = sequence.getCurrentFrame();
    if (frame != NULL){
        Graphics::Bitmap clipped(work, x, y, width, height);
        frame->draw(clipped);
    }
}

void Animation::forwardFrame(){
    sequence.forwardFrame();
}

void Animation::backFrame(){
    sequence.backFrame();
}
    
void Animation::reset(){
    sequence.reset();
}

void Animation::resetAll(){
    sequence.reset();
}

void Animation::setToEnd(){
    /* FIXME */
    sequence.setToEnd();
    // currentFrame = frames.size()-1;
    // currentLoop = 0;
    // Set offsets 
    /*
    for (std::vector<Util::ReferenceCount<Frame> >::iterator i = frames.begin(); i != frames.end(); ++i){
        Util::ReferenceCount<Frame> frame = *i;
        frame->setToEnd(RelativePoint(ticks * velocity.getRelativeX(), ticks * velocity.getRelativeY()));
    }
    */
}

const std::string Animation::getInfo(){
    /*
    static const char * ANIMATION_TEXT = "Animation ID: %d\nTicks: %d\nFrame Index: %d\nLoop From: %d\nAxis: ( %f, %f)\nVelocity: ( %f, %f)\n";
    char info[255];
    sprintf(info, ANIMATION_TEXT, id, ticks, currentFrame, loop, axis.getRelativeX(), axis.getRelativeY(), velocity.getRelativeX(), velocity.getRelativeY());
    return std::string(info) + frames[currentFrame]->getInfo();
    */
    /* FIXME */
    ostringstream out;
    out << "Animation ID: " << id;
    return out.str();
}

AnimationManager::AnimationManager(){
    // Set the current id to 0 for each context
    CURRENT_ID = 0;
}

AnimationManager::AnimationManager(const AnimationManager & copy):
animations(copy.animations){
    // Set the current id to 0 for each context
    CURRENT_ID = 0;
}

AnimationManager::~AnimationManager(){
}

const AnimationManager & AnimationManager::operator=(const AnimationManager & copy){
    animations = copy.animations;
    return *this;
}


void AnimationManager::forward(int tickCount){
    for (std::map<Gui::Animation::Depth, std::vector<Util::ReferenceCount<Gui::Animation> > >::iterator i = animations.begin(); i != animations.end(); ++i){
        std::vector<Util::ReferenceCount<Gui::Animation> > & animations = i->second;
        for (std::vector<Util::ReferenceCount<Gui::Animation> >::iterator j = animations.begin(); j != animations.end(); ++j){
            Util::ReferenceCount<Gui::Animation> animation = *j;
            if (animation != NULL){
                animation->forward(tickCount);
            }
        }
    }
}

void AnimationManager::reverse(int tickCount){
    for (std::map<Gui::Animation::Depth, std::vector<Util::ReferenceCount<Gui::Animation> > >::iterator i = animations.begin(); i != animations.end(); ++i){
        std::vector<Util::ReferenceCount<Gui::Animation> > & animations = i->second;
        for (std::vector<Util::ReferenceCount<Gui::Animation> >::iterator j = animations.begin(); j != animations.end(); ++j){
            Util::ReferenceCount<Gui::Animation> animation = *j;
            if (animation != NULL){
                animation->reverse(tickCount);
            }
        }
    }
}

void AnimationManager::act(){
    forward();
}

void AnimationManager::render(const Gui::Animation::Depth & depth, const Graphics::Bitmap & work){
    for (std::vector<Util::ReferenceCount<Gui::Animation> >::iterator i = animations[depth].begin(); i != animations[depth].end(); ++i){
        Util::ReferenceCount<Gui::Animation> animation = *i;
        if (animation != NULL){
            animation->draw(work);
        }   
    }
}

void AnimationManager::add(Util::ReferenceCount<Gui::Animation > animation){
    animations[animation->getDepth()].push_back(animation);
}

void AnimationManager::reset(){
    for (std::map<Gui::Animation::Depth, std::vector<Util::ReferenceCount<Gui::Animation> > >::iterator i = animations.begin(); i != animations.end(); ++i){
        std::vector<Util::ReferenceCount<Gui::Animation> > & animations = i->second;
        for (std::vector<Util::ReferenceCount<Gui::Animation> >::iterator j = animations.begin(); j != animations.end(); ++j){
            Util::ReferenceCount<Gui::Animation> animation = *j;
            if (animation != NULL){
                animation->resetAll();
            }
        }
    }
}
    
int AnimationManager::totalTicks() const {
    int count = 0;
    for (map<Gui::Animation::Depth, vector<Util::ReferenceCount<Gui::Animation> > >::const_iterator it = animations.begin(); it != animations.end(); it++){
        Gui::Animation::Depth depth = it->first;
        const vector<Util::ReferenceCount<Gui::Animation> > & stuff = it->second;
        int next = countTicks(stuff);
        /* If any animation takes infinite time then the whole thing takes infinite time */
        if (next == -1){
            return -1;
        }
        if (next > count){
            count = next;
        }
    }
    return count;
}

int AnimationManager::countTicks(const vector<Util::ReferenceCount<Animation> > & toCount) const {
    int total = 0;
    for (vector<Util::ReferenceCount<Animation> >::const_iterator it = toCount.begin(); it != toCount.end(); it++){
        const Util::ReferenceCount<Animation> & animation = *it;
        int ticks = animation->totalTicks();
        if (ticks == -1){
            return ticks;
        }
        total += ticks;
    }
    return total;
}

void AnimationManager::setToEnd(){
    for (std::map<Gui::Animation::Depth, std::vector<Util::ReferenceCount<Gui::Animation> > >::iterator i = animations.begin(); i != animations.end(); ++i){
        std::vector<Util::ReferenceCount<Gui::Animation> > & animations = i->second;
        for (std::vector<Util::ReferenceCount<Gui::Animation> >::iterator j = animations.begin(); j != animations.end(); ++j){
            Util::ReferenceCount<Gui::Animation> animation = *j;
            if (animation != NULL){
                animation->setToEnd();
            }
        }
    }
}

const std::string AnimationManager::getInfo(int id, bool all){
    std::string info("");
    for (std::map<Gui::Animation::Depth, std::vector<Util::ReferenceCount<Gui::Animation> > >::iterator i = animations.begin(); i != animations.end(); ++i){
        std::vector<Util::ReferenceCount<Gui::Animation> > & animations = i->second;
        for (std::vector<Util::ReferenceCount<Gui::Animation> >::iterator j = animations.begin(); j != animations.end(); ++j){
            Util::ReferenceCount<Gui::Animation> animation = *j;
            if (animation != NULL){
                if (id == animation->getID() || all){
                    info += animation->getInfo();
                }
            }
        }
    }
    return info;
}

const std::vector<int> AnimationManager::getIdList(){
    std::vector<int> list;
    for (std::map<Gui::Animation::Depth, std::vector<Util::ReferenceCount<Gui::Animation> > >::iterator i = animations.begin(); i != animations.end(); ++i){
        std::vector<Util::ReferenceCount<Gui::Animation> > & animations = i->second;
        for (std::vector<Util::ReferenceCount<Gui::Animation> >::iterator j = animations.begin(); j != animations.end(); ++j){
            Util::ReferenceCount<Gui::Animation> animation = *j;
            if (animation != NULL){
                list.push_back(animation->getID());
            }
        }
    }
    return list;
}
