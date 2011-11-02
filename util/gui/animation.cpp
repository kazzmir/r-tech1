#include "animation.h"

#include <vector>
#include <math.h>
#include <sstream>
#include "util/token.h"
#include "util/trans-bitmap.h"
#include "util/bitmap.h"
#include "globals.h"
#include "../debug.h"
#include "../funcs.h"
#include "../file-system.h"

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

Frame::Frame(const Token *the_token, imageMap &images) throw (LoadException):
bmp(0),
time(0),
horizontalFlip(false),
verticalFlip(false),
alpha(255){
    if ( *the_token != "frame" ){
        throw LoadException(__FILE__, __LINE__, "Not an frame");
    }
    const Token & tok = *the_token;
    /* The usual setup of an animation frame is
    // use image -1 to not draw anything, it can be used to get a blinking effect
    (frame (image NUM) (alpha NUM) (offset x y) (hflip 0|1) (vflip 0|1) (time NUM))
    */
    TokenView view = tok.view();
    while (view.hasMore()){
        try{
            const Token * token;
            view >> token;
            if (*token == "image"){
                // get the number
                int num;
                token->view() >> num;
                // now assign the bitmap
                bmp = images[num];
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
        } catch ( const TokenException & ex ) {
            throw LoadException(__FILE__, __LINE__, ex, "Menu animation parse error");
        } catch ( const LoadException & ex ) {
            throw ex;
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
    scrollOffset.moveBy(xvel,yvel);
    if (scrollOffset.getDistanceFromCenterX() >=bmp->getWidth()){
	    scrollOffset.setX(0);
    } else if (scrollOffset.getDistanceFromCenterX() <= -(bmp->getWidth())){
	    scrollOffset.setX(0);
    }
    if (scrollOffset.getDistanceFromCenterY() >=bmp->getHeight()){
	    scrollOffset.setY(0);
    } else if (scrollOffset.getDistanceFromCenterY() <= -(bmp->getHeight())){
	    scrollOffset.setY(0);
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

Animation::Animation(const Token *the_token) throw (LoadException):
id(0),
depth(BackgroundBottom),
ticks(0),
currentFrame(0),
loop(0),
allowReset(true){
    images[-1] = 0;
    std::string basedir = "";
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
          (depth background|foreground NUM)
	      (basedir LOCATION)
	      (image NUM FILE) 
	      (velocity x y)
	      (axis x y) 
	      (frame "Read comments above in constructor") 
	      (loop)
	      (reset NUM)
	      (window x1 y1 x2 y2))
    */
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
                Global::debug(0) << "The option 'location' is no longer valid and will be ignored. Consider using 'depth'" << std::endl;
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
                token->view() >> number >> temp;
                Util::ReferenceCount<Graphics::Bitmap> bmp(new Graphics::Bitmap(Storage::instance().find(Filesystem::RelativePath(basedir + temp)).path()));
                if (!bmp->getError()){
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
                // new frame
                Util::ReferenceCount<Frame> frame(new Frame(token, images));
                frames.push_back(frame);
            } else if (*token == "loop"){
                // start loop here
                int l;
                token->view() >> l;
		if (l >= (int)frames.size()){
		    ostringstream out;
		    out << "Loop location is larger than the number of frames. Loop: " << loop << " Frames: " << frames.size();
		    throw LoadException(__FILE__, __LINE__, out.str());
		}
		loop = l;
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

Animation::Animation(const std::string & background) throw (LoadException):
id(0),
depth(BackgroundBottom),
ticks(0),
currentFrame(0),
loop(0),
allowReset(true){
    // add bitmap
    Util::ReferenceCount<Graphics::Bitmap> bmp(new Graphics::Bitmap(Storage::instance().find(Filesystem::RelativePath(background)).path()));
    if (bmp->getError()){
        throw LoadException(__FILE__,__LINE__, "Problem loading file: " + background);
    } else {
        images[0] = bmp;
    }
    Util::ReferenceCount<Frame> frame(new Frame(bmp));
    frames.push_back(frame);
}

Animation::Animation(const Filesystem::AbsolutePath & path) throw (LoadException):
id(0),
depth(BackgroundBottom),
ticks(0),
currentFrame(0),
loop(0),
allowReset(true){
    // add bitmap
    Util::ReferenceCount<Graphics::Bitmap> bmp(new Graphics::Bitmap(path.path()));
    if (bmp->getError()){
        throw LoadException(__FILE__,__LINE__, "Problem loading file: " + path.path());
    } else {
        images[0] = bmp;
    }
    Util::ReferenceCount<Frame> frame(new Frame(bmp));
    frames.push_back(frame);
}

Animation::Animation(Util::ReferenceCount<Graphics::Bitmap> image):
id(0),
depth(BackgroundBottom),
ticks(0),
currentFrame(0),
loop(0),
allowReset(true){
    images[0] = image;
    Util::ReferenceCount<Frame> frame(new Frame(image));
    frames.push_back(frame);
}

Animation::~Animation(){
}
void Animation::act(){
    // Used for scrolling
    for (std::vector<Util::ReferenceCount<Frame> >::iterator i = frames.begin(); i != frames.end(); ++i){
        Util::ReferenceCount<Frame> frame = *i;
	    frame->act(velocity.getRelativeX(), velocity.getRelativeY());
    }
    if( frames[currentFrame]->time != -1 ){
	    ticks++;
	    if(ticks >= frames[currentFrame]->time){
		    ticks = 0;
		    forwardFrame();
	    }
    }
}
void Animation::draw(const Graphics::Bitmap & work){
    /* FIXME: should use sub-bitmaps here */
     // Set clip from the axis default is 0,0,bitmap width, bitmap height
    work.setClipRect(-(window.getPosition().getDistanceFromCenterX()),-(window.getPosition().getDistanceFromCenterY()),work.getWidth() - window.getPosition2().getDistanceFromCenterX(),work.getHeight() - window.getPosition2().getDistanceFromCenterY());
    frames[currentFrame]->draw(axis.getDistanceFromCenterX(), axis.getDistanceFromCenterY(),work);
    work.setClipRect(0,0,work.getWidth(),work.getHeight());
}

void Animation::forwardFrame(){
    if (currentFrame < frames.size() -1){
        currentFrame++;
    } else {
        currentFrame = loop;
    }
}
void Animation::backFrame(){
    if (currentFrame > loop){
	    currentFrame--;
    } else {
	    currentFrame = frames.size() - 1;
    }
}

AnimationManager::AnimationManager(){
}
AnimationManager::AnimationManager(const AnimationManager & copy):
animations(copy.animations){
}

AnimationManager::~AnimationManager(){
}

const AnimationManager & AnimationManager::operator=(const AnimationManager & copy){
    animations = copy.animations;
    return *this;
}

void AnimationManager::act(){
    for (std::map<Gui::Animation::Depth, std::vector<Util::ReferenceCount<Gui::Animation> > >::iterator i = animations.begin(); i != animations.end(); ++i){
        std::vector<Util::ReferenceCount<Gui::Animation> > & animations = i->second;
        for (std::vector<Util::ReferenceCount<Gui::Animation> >::iterator j = animations.begin(); j != animations.end(); ++j){
            Util::ReferenceCount<Gui::Animation> animation = *j;
            if (animation != NULL){
                animation->act();
            }
        }
    }
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
