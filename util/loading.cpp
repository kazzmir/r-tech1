#include "bitmap.h"
#include "trans-bitmap.h"
#include <math.h>
#include <iostream>

/* FIXME: get rid of this dependancy */
#include "paintown-engine/level/utils.h"

#include "messages.h"
#include "loading.h"
#include "file-system.h"
#include "font.h"
#include "funcs.h"
#include "gradient.h"
#include "parameter.h"
#include "thread.h"
#include "globals.h"
#include <vector>
#include "thread.h"
#include "message-queue.h"
#include "init.h"
#include "events.h"

using namespace std;

namespace Loader{

volatile bool done_loading = true;

typedef struct pair{
	int x, y;
} ppair;

class Info{
public:
    Info(){
        Global::registerInfo(&messages);
    }

    bool transferMessages(Messages & box){
        bool did = false;
        while (messages.hasAny()){
            const string & str = messages.get();
            box.addMessage(str);
            did = true;
        }
        return did;
    }

    ~Info(){
        Global::unregisterInfo(&messages);
    }

private:
    MessageQueue messages;
};

void * loadingScreenSimple1(void * arg);

static void setupBackground(const Graphics::Bitmap & background, int load_x, int load_y, int load_width, int load_height, int infobox_x, int infobox_y, int infoWidth, int infoHeight, const Graphics::Bitmap & infoBackground){
    Font::getDefaultFont().printf( 400, 480 - Font::getDefaultFont().getHeight() * 5 / 2 - Font::getDefaultFont().getHeight(), Graphics::makeColor( 192, 192, 192 ), background, "Paintown version %s", 0, Global::getVersionString().c_str());
    Font::getDefaultFont().printf( 400, 480 - Font::getDefaultFont().getHeight() * 5 / 2, Graphics::makeColor( 192, 192, 192 ), background, "Made by Jon Rafkind", 0 );
    background.BlitToScreen();
    background.Blit(infobox_x, infobox_y, infoWidth, infoHeight, 0, 0, infoBackground);
}

/* converts a bitmap with some text on it into a sequence of points */
static vector<ppair> generateFontPixels(const Font & myFont, const string & message, int width, int height){
    Graphics::Bitmap letters(width, height);
    letters.fill(Graphics::MaskColor());
    myFont.printf(0, 0, Graphics::makeColor(255, 255, 255), letters, message.c_str(), 0); 

    vector<ppair> pairs;
    /* store every pixel we need to draw */
    letters.lock();
    for (int x = 0; x < letters.getWidth(); x++){
        for (int y = 0; y < letters.getHeight(); y++){
            Graphics::Color pixel = letters.getPixel(x, y);
            if (pixel != Graphics::MaskColor()){
                ppair p;
                p.x = x;
                p.y = y;
                pairs.push_back(p);
            }
        }
    }
    letters.unlock();

    // Graphics::resetDisplay();

    return pairs;
}

/* shows time elapsed */
class TimeCounter{
public:
    TimeCounter():
    work(200, 40){
        start = Global::second_counter;
        last = 0;
    }

    void draw(int x, int y){
        const Font & font = Font::getDefaultFont(24, 24);
        if (Global::second_counter != last){
            work.clear();
            last = Global::second_counter;
            font.printf(0, 0, Graphics::makeColor(192, 192, 192), work, "Waiting.. %d", 0, last - start);
            work.BlitAreaToScreen(x, y);
        }
    }

    Graphics::Bitmap work;
    unsigned int start;
    unsigned int last;
};

static void loadingScreen1(LoadingContext & context, const Level::LevelInfo & levelInfo){
    int load_x = 80;
    int load_y = 220;
    const int infobox_width = 300;
    const int infobox_height = 150;
    Graphics::resetDisplay();
    const Font & myFont = Font::getFont(Global::DEFAULT_FONT, 24, 24);

    if (levelInfo.getPositionX() != -1){
        load_x = levelInfo.getPositionX();
    }
    
    if (levelInfo.getPositionY() != -1){
        load_y = levelInfo.getPositionY();
    }

    // const char * the_string = (arg != NULL) ? (const char *) arg : "Loading...";
    int load_width = myFont.textLength(levelInfo.loadingMessage().c_str());
    int load_height = myFont.getHeight(levelInfo.loadingMessage().c_str());

    Global::debug(2) << "loading screen" << endl;

    Messages infobox(infobox_width, infobox_height);

    const int MAX_COLOR = 200;

    /* blend from dark grey to light red */
    Effects::Gradient gradient(MAX_COLOR, Graphics::makeColor(16, 16, 16), Graphics::makeColor(192, 8, 8));

    TimeCounter counter;

    struct State{
        bool drawInfo;
    };

    class Logic: public Util::Logic {
    public:
        Logic(LoadingContext & context, State & state, Effects::Gradient & gradient, Messages & infoBox):
        context(context),
        state(state),
        gradient(gradient),
        infobox(infoBox){
        }

        LoadingContext & context;
        State & state;
        Effects::Gradient & gradient;
        Info info;
        Messages & infobox;

        void run(){
            gradient.backward();
            state.drawInfo = info.transferMessages(infobox) || state.drawInfo;
        }

        double ticks(double system){
            return system;
        }

        bool done(){
            return context.done();
        }
    };

    class Draw: public Util::Draw {
    public:
        Draw(const Level::LevelInfo & levelInfo, State & state, Messages & infobox, Effects::Gradient & gradient, int load_width, int load_height, int infobox_width, int infobox_height, int load_x, int load_y):
            gradient(gradient),
            state(state),
            infobox(infobox),
            infoWork(*Util::Parameter<Graphics::Bitmap*>::current(), load_x, load_y + load_height * 2, infobox_width, infobox_height),
            infoBackground(infobox_width, infobox_height),
            infobox_x(load_x),
            infobox_y(load_y + load_height * 2),
            load_x(load_x),
            load_y(load_y),
            load_width(load_width),
            load_height(load_height){

            const Font & myFont = Font::getFont(Global::DEFAULT_FONT, 24, 24);
            pairs = generateFontPixels(myFont, levelInfo.loadingMessage(), load_width, load_height);

            if (levelInfo.getBackground() != 0){
                setupBackground(*levelInfo.getBackground(), load_x, load_y, load_width, load_height, infobox_x, infobox_y, infoBackground.getWidth(), infoBackground.getHeight(), infoBackground);
            } else {
                setupBackground(Graphics::Bitmap(levelInfo.loadingBackground().path()), load_x, load_y, load_width, load_height, infobox_x, infobox_y, infoBackground.getWidth(), infoBackground.getHeight(), infoBackground);
            }
        }

        Effects::Gradient & gradient;
        State & state;
        Messages & infobox;
        Graphics::Bitmap infoWork;
        Graphics::Bitmap infoBackground;
        vector<ppair> pairs;
        const int infobox_x;
        const int infobox_y;
        const int load_x;
        const int load_y;
        const int load_width;
        const int load_height;

        void draw(const Graphics::Bitmap & screen){
            Graphics::Bitmap work(screen, load_x, load_y, load_width, load_height);
            work.lock();
            for (vector< ppair >::iterator it = pairs.begin(); it != pairs.end(); it++){
                Graphics::Color color = gradient.current(it->x);
                work.putPixel(it->x, it->y, color);
            }
            work.unlock();

            if (state.drawInfo){
                infoBackground.Blit(infoWork);

                const Font & infoFont = Font::getFont(Global::DEFAULT_FONT, 24, 24);
                /* cheesy hack to change the font size. the font
                 * should store the size and change it on its own
                 */
                Font::getFont(Global::DEFAULT_FONT, 13, 13);
                infobox.draw(0, 0, infoWork, infoFont);
                Font::getFont(Global::DEFAULT_FONT, 24, 24);
                infoWork.BlitAreaToScreen(infobox_x, infobox_y);
                // infoWork.BlitToScreen();
                state.drawInfo = false;
            }
            /* work already contains the correct background */
            // work.Blit( load_x, load_y, *Bitmap::Screen );
            // work.BlitToScreen();
            work.BlitAreaToScreen(load_x, load_y);
        }
    };

    State state;
    state.drawInfo = true;
    Logic logic(context, state, gradient, infobox);
    Draw draw(levelInfo, state, infobox, gradient, load_width, load_height, infobox_width, infobox_height, load_x, load_y);

    Util::standardLoop(logic, draw);
    Graphics::resetDisplay();

#if 0
    while (! context.done()){

        /* true if a logic loop has passed */
        bool draw = firstDraw;

        /* will be true if any new info messages appeared */
        bool drawInfo = firstDraw;
        firstDraw = false;
        if ( Global::speed_counter > 0 ){
            double think = Global::speed_counter;	
            Global::speed_counter = 0;
            draw = true;

            while ( think > 0 ){
                gradient.backward();
                think -= 1;
            }

            /* if no new messages appeared this will be false */
            drawInfo = info.transferMessages(infobox);
        } else {
            Util::rest( 1 );
        }

        if (draw){
            for ( vector< ppair >::iterator it = pairs.begin(); it != pairs.end(); it++ ){
                int color = gradient.current(it->x);
                work.putPixel(it->x, it->y, color);
            }

            // counter.draw(200, 100);

            /* we might not have to draw the whole info box again if no new
             * messages appeared.
             */
            if (drawInfo){
                infoBackground.Blit(infoWork);

                /* cheesy hack to change the font size. the font
                 * should store the size and change it on its own
                 */
                Font::getFont(Global::DEFAULT_FONT, 13, 13);
                infobox.draw(0, 0, infoWork, infoFont);
                Font::getFont(Global::DEFAULT_FONT, 24, 24);
                infoWork.BlitAreaToScreen(infobox_x, infobox_y);
            }
            /* work already contains the correct background */
            // work.Blit( load_x, load_y, *Bitmap::Screen );
            work.BlitAreaToScreen(load_x, load_y);
        }
    }
#endif
}

static void loadingScreenSimpleX1(LoadingContext & context, const Level::LevelInfo & levelInfo){
    class Logic: public Util::Logic {
    public:
        Logic(LoadingContext & context, int & angle, int speed):
        context(context),
        speed(speed),
        angle(angle){
        }

        LoadingContext & context;
        /* speed of rotation */
        const int speed;
        int & angle;

        double ticks(double system){
            return system / 2;
        }

        bool done(){
            return context.done();
        }

        void run(){
            angle += speed * 2;
        }
    };

    class Draw: public Util::Draw {
    public:
        Draw(int & angle, const int speed):
        original(40, 40),
        angle(angle),
        speed(speed){
            original.BlitFromScreen(0, 0);

            color1 = Graphics::makeColor(0, 0, 0);
            color2 = Graphics::makeColor(0x00, 0x99, 0xff);
            color3 = Graphics::makeColor(0xff, 0x22, 0x33);
            color4 = Graphics::makeColor(0x44, 0x77, 0x33);
            colors[0] = color1;
            colors[1] = color2;
            colors[2] = color3;
            colors[3] = color4;
            Graphics::Bitmap::transBlender(0, 0, 0, 64);
        }

        Graphics::Bitmap original;
        int & angle;
        const int speed;

        Graphics::Color color1;
        Graphics::Color color2;
        Graphics::Color color3;
        Graphics::Color color4;
        /* the length of this array is the number of circles to show */
        Graphics::Color colors[4];

        ~Draw(){
        }

        void draw(const Graphics::Bitmap & screen){
            Graphics::Bitmap work(screen, 0, 0, 40, 40);
            int max = sizeof(colors) / sizeof(int);
            double middleX = work.getWidth() / 2;
            double middleY = work.getHeight() / 2;
            original.Blit(work);
            for (int i = 0; i < max; i++){
                double x = cos(Util::radians(angle + 360 / max * i)) * 15;
                double y = sin(Util::radians(angle + 360 / max * i)) * 15;
                /* ghost circle */
                work.translucent().circleFill(middleX + x, middleY + y, 2, colors[i]);
                x = cos(Util::radians(angle + speed + 360 / max * i)) * 15;
                y = sin(Util::radians(angle + speed + 360 / max * i)) * 15;
                /* real circle */
                work.circleFill(middleX + x, middleY + y, 2, colors[i]);
            }
            work.BlitAreaToScreen(0, 0);
        }
    };

    int angle = 0;
    int speed = 7;
    Logic logic(context, angle, speed);
    Draw draw(angle, speed);
    Util::standardLoop(logic, draw);

#if 0
    while (! context.done()){
        bool draw = false;

        if (Global::speed_counter > 0){
            double think = Global::speed_counter;	
            Global::speed_counter = 0;
            draw = true;

            while (think > 0){
                angle += speed;
                think -= 1;
            }
        } else {
            Util::rest(1);
        }

        if (draw){
            int max = sizeof(colors) / sizeof(int);
            double middleX = work.getWidth() / 2;
            double middleY = work.getHeight() / 2;
            original.Blit(work);
            for (int i = 0; i < max; i++){
                double x = cos(Util::radians(angle + 360 / max * i)) * 15;
                double y = sin(Util::radians(angle + 360 / max * i)) * 15;
                /* ghost circle */
                work.translucent().circleFill(middleX + x, middleY + y, 2, colors[i]);
                x = cos(Util::radians(angle + speed + 360 / max * i)) * 15;
                y = sin(Util::radians(angle + speed + 360 / max * i)) * 15;
                /* real circle */
                work.circleFill(middleX + x, middleY + y, 2, colors[i]);
            }
            work.BlitAreaToScreen(0, 0);
        }
    }
#endif
}

LoadingContext::LoadingContext():
finished(false){
    Util::Thread::initializeLock(&lock);
}

LoadingContext::~LoadingContext(){
}

void LoadingContext::doLoad(){
    this->load();
    Util::Thread::acquireLock(&lock);
    finished = true;
    Util::Thread::releaseLock(&lock);
}

bool LoadingContext::done(){
    bool ok = false;
    Util::Thread::acquireLock(&lock);
    ok = this->finished;
    Util::Thread::releaseLock(&lock);
    return ok;
}
    
void * LoadingContext::load_it(void * arg){
    LoadingContext * context = (LoadingContext*) arg;
    context->doLoad();
    return NULL;
}

static void showLoadMessage(){
    int screenX = 80;
    int screenY = 50;
    Graphics::Bitmap work(110, 50);
    work.BlitFromScreen(screenX, screenY);
    Graphics::Bitmap top(110, 50);
    top.fill(Graphics::makeColor(0, 0, 0));
    Font::getDefaultFont(25, 25).printf(10, 5, Graphics::makeColor(192, 192, 192), top, "Loading", 0);
    Graphics::Bitmap::transBlender(0, 0, 0, 200);
    top.translucent().draw(0, 0, work);
    work.BlitAreaToScreen(screenX, screenY);
}

void loadScreen(LoadingContext & context, const Level::LevelInfo & info, Kind kind){
    Util::Thread::Id loadingThread;
    bool created = Util::Thread::createThread(&loadingThread, NULL, (Util::Thread::ThreadFunction) LoadingContext::load_it, &context);
    if (!created){
        Global::debug(0) << "Could not create loading thread. Loading will occur in the main thread" << endl;
        showLoadMessage();
        LoadingContext::load_it(&context);
        // throw LoadException(__FILE__, __LINE__, "Could not create loader thread");
    } else {
        switch (kind){
            case Default: loadingScreen1(context, info); break;
            case SimpleCircle: loadingScreenSimpleX1(context, info); break;
            default: loadingScreen1(context, info); break;
        }

        Util::Thread::joinThread(loadingThread);
    }
}

}
