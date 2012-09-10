#include "util/bitmap.h"
#include "util/stretch-bitmap.h"
#include "util/trans-bitmap.h"

#include "options.h"
#include "util/token.h"
#include "util/input/input-source.h"
#include "util/parameter.h"
#include "util/tokenreader.h"
#include "menu.h"
#include "configuration.h"
#include "menu-exception.h"
#include "util/init.h"
#include "util/events.h"
#include "optionfactory.h"

#include "util/music.h"

#include "util/input/keyboard.h"
#include "util/funcs.h"
#include "util/file-system.h"
#include "factory/font_factory.h"
#include "globals.h"
#include "util/exceptions/shutdown_exception.h"
#include "util/exceptions/exception.h"
#include "util/font.h"
#include "util/gui/box.h"
#include "util/thread.h"

#include "util/loading.h"
#include "util/fire.h"
#include "util/input/input-map.h"
#include "util/input/input-manager.h"

#include <sstream>
#include <algorithm>
#include <time.h>
#include <math.h>

using namespace std;
using namespace Gui;

/* true if the arguments passed in match todays date.
 * pass 0 for any argument that you don't care about (it will match any date)
 */
static bool todaysDate(int month, int day, int year){
    int currentMonth;
    int currentDay;
    int currentYear;
    time_t result = time(NULL);
    struct tm * local = localtime(&result);
    return (month == 0 || month == (local->tm_mon + 1)) &&
           (day == 0 || day == local->tm_mday) &&
           (year == 0 || year == local->tm_year + 1900);
}

static bool jonBirthday(){
    return todaysDate(3, 25, 0);
}

static bool miguelBirthday(){
    return todaysDate(8, 11, 0);
}

OptionCredits::Block::Block(const std::string & title):
title(title),
titleColorOverride(false),
titleColor(Graphics::makeColor(0,255,255)),
colorOverride(false),
color(Graphics::makeColor(255,255,255)),
spacing(0){
}

OptionCredits::Block::Block(const Token * token):
titleColorOverride(false),
titleColor(Graphics::makeColor(0,255,255)),
colorOverride(false),
color(Graphics::makeColor(255,255,255)),
topWidth(0),
topHeight(0),
bottomWidth(0),
bottomHeight(0),
spacing(0){
    if ( *token != "credit-block" ){
        throw LoadException(__FILE__, __LINE__, "Not a credit block");
    }
    
    TokenView view = token->view();
    
    while (view.hasMore()){
        std::string match;
        try{
            const Token * tok;
            view >> tok;
            if ( *tok == "title" ) {
                tok->view() >> title;
            } else if (*tok == "credit"){
                std::string credit;
                tok->view() >> credit;
                credits.push_back(credit);
            } else if ( *tok == "titlecolor" ) {
                try{
                    int r,b,g;
                    tok->view() >> r >> g >> b;
                    titleColor = Graphics::makeColor( r, g, b );
                    titleColorOverride = true;
                } catch (const TokenException & ex){
                }
            } else if ( *tok == "color" ) {
                try{
                    int r,b,g;
                    tok->view() >> r >> g >> b;
                    color = Graphics::makeColor( r, g, b );
                    colorOverride = true;
                } catch (const TokenException & ex){
                }
            } else if ( *tok == "animation" ) {
                TokenView animView = tok->view();
                while (animView.hasMore()){
                    const Token * animTok;
                    animView >> animTok;
                    if (*animTok == "top"){
                        tok->match("_/width", topWidth);
                        tok->match("_/height", topHeight);
                        topAnimation = Util::ReferenceCount<Gui::Animation>(new Animation(tok));
                    } else if (*animTok == "bottom"){
                        tok->match("_/width", bottomWidth);
                        tok->match("_/height", bottomHeight);
                        bottomAnimation = Util::ReferenceCount<Gui::Animation>(new Animation(tok));
                    }
                }
            } else if (*tok == "spacing"){
                tok->view() >> spacing;
            } else {
                Global::debug( 3 ) <<"Unhandled Credit Block attribute: "<<endl;
                if (Global::getDebug() >= 3){
                    tok->print(" ");
                }
            }
        } catch ( const TokenException & ex ) {
            throw LoadException(__FILE__, __LINE__, ex, "Credit Block parse error");
        } catch ( const LoadException & ex ) {
            throw ex;
        }
    }
}

OptionCredits::Block::Block(const OptionCredits::Block & copy):
title(copy.title),
credits(copy.credits),
titleColorOverride(copy.titleColorOverride),
titleColor(copy.titleColor),
colorOverride(copy.colorOverride),
color(copy.color),
topAnimation(copy.topAnimation),
topWidth(copy.topWidth),
topHeight(copy.topHeight),
bottomAnimation(copy.bottomAnimation),
bottomWidth(copy.bottomWidth),
bottomHeight(copy.bottomHeight),
spacing(copy.spacing){
}

OptionCredits::Block::~Block(){
}

const OptionCredits::Block & OptionCredits::Block::operator=(const OptionCredits::Block & copy){
    title = copy.title;
    credits = copy.credits;
    titleColor = copy.titleColor;
    titleColorOverride = copy.titleColorOverride;
    color = copy.color;
    colorOverride = copy.colorOverride;
    topAnimation = copy.topAnimation;
    topWidth =copy.topWidth;
    topHeight = copy.topHeight;
    bottomAnimation = copy.bottomAnimation;
    bottomWidth = copy.bottomWidth;
    bottomHeight = copy.bottomHeight;
    spacing = copy.spacing;
    return *this;
}

void OptionCredits::Block::addCredit(const std::string & credit){
    credits.push_back(credit);
}

void OptionCredits::Block::act(){
    // Top animation
    if (topAnimation != NULL){
        topAnimation->act();
    }
    // Bottom animation
    if (bottomAnimation != NULL){
        bottomAnimation->act();
    }
}

int OptionCredits::Block::print(int x, int y, Graphics::Color defaultTitleColor, Graphics::Color defaultColor, const Font & font, const Graphics::Bitmap & work, const Justification & justification) const {
    int currentY = y;
    
    // Top animation
    if (topAnimation != NULL){
        int xmod = 0;
        switch (justification){
            default:
            case Left:
                xmod = 0;
                break;
            case Center:
                xmod = topWidth/2;
                break;
            case Right:
                xmod = topWidth;
                break;
        }
        topAnimation->draw(x - xmod, y, topWidth, topHeight, work);
        currentY+=topHeight;
    }
    
    if (!title.empty()){
        int xmod = 0;
        switch (justification){
            default:
            case Left:
                xmod = 0;
                break;
            case Center:
                xmod = font.textLength(title.c_str())/2;
                break;
            case Right:
                xmod = font.textLength(title.c_str());
                break;
        }
        font.printf(x - xmod, currentY, (titleColorOverride ? titleColor : defaultTitleColor), work, title, 0);
        currentY += font.getHeight();
    }
    
    for (std::vector<std::string>::const_iterator i = credits.begin(); i != credits.end(); ++i){
        const std::string & credit = *i;
        int xmod = 0;
        switch (justification){
            default:
            case Left:
                xmod = 0;
                break;
            case Center:
                xmod = font.textLength(credit.c_str())/2;
                break;
            case Right:
                xmod = font.textLength(credit.c_str());
                break;
        }
        font.printf(x - xmod, currentY, (colorOverride ? color : defaultColor), work, credit, 0);
        currentY += font.getHeight();
    }
    
    // Bottom animation
    if (bottomAnimation != NULL){
        int xmod = 0;
        switch (justification){
            default:
            case Left:
                xmod = 0;
                break;
            case Center:
                xmod = bottomWidth/2;
                break;
            case Right:
                xmod = bottomWidth;
                break;
        }
        bottomAnimation->draw(x - xmod, y, bottomWidth, bottomHeight, work);
        currentY+=bottomHeight;
    }
    
    currentY += font.getHeight() + spacing;
    
    return currentY;
}

const int OptionCredits::Block::size(const Font & font) const{
    // Counts title and space in between
    int total = 0;
    if (topAnimation != NULL){
        total += topHeight;
    }
    if (!title.empty()){
        total+= font.getHeight();
    }
    total += credits.size() * font.getHeight();
    if (bottomAnimation != NULL){
        total += bottomHeight;
    }
    total += font.getHeight();
    
    total += spacing;
    return total;
}

OptionCredits::Sequence::Sequence(Token * token):
type(Primary),
x(0),
y(0),
startx(0),
endx(0),
starty(0),
endy(0),
speed(0),
alpha(0),
alphaMultiplier(0),
justification(Block::Center),
current(0),
done(false){
    if ( *token != "sequence" ){
        throw LoadException(__FILE__, __LINE__, "Not a credit sequence");
    }
    
    TokenView view = token->view();
    
    while (view.hasMore()){
        std::string match;
        try{
            const Token * tok;
            view >> tok;
            if (*tok == "type"){
                std::string sequenceType;
                tok->view() >> sequenceType;
                if (sequenceType == "roll"){
                    type = Roll;
                } else if (sequenceType == "primary"){
                    type = Primary;
                }
            } else if (*tok == "start-x"){
                tok->view() >> startx;
            } else if (*tok == "end-x"){
                tok->view() >> endx;
            } else if (*tok == "start-y"){
                tok->view() >> starty;
            } else if (*tok == "end-y"){
                tok->view() >> endy;
            } else if (*tok == "speed"){
                tok->view() >> speed;
            } else if (*tok == "alpha-multiplier"){
                tok->view() >> alphaMultiplier;
            } else if ( *tok == "justification" ) {
                std::string justify;
                tok->view() >> justify;
                if (justify == "left"){
                    justification = Block::Left;
                } else if (justify == "center"){
                    justification = Block::Center;
                } else if (justify == "right"){
                    justification = Block::Right;
                }
            } else if (*tok == "block"){
                credits.push_back(Block(tok));
            } else {
                Global::debug( 3 ) <<"Unhandled Credit Sequence attribute: "<<endl;
                if (Global::getDebug() >= 3){
                    tok->print(" ");
                }
            }
        } catch ( const TokenException & ex ) {
            throw LoadException(__FILE__, __LINE__, ex, "Credit Sequence parse error");
        } catch ( const LoadException & ex ) {
            throw ex;
        }
    }
    
    // Initial
    reset();
}

OptionCredits::Sequence::Sequence(const Sequence & copy):
type(copy.type),
x(0),
y(0),
startx(copy.startx),
endx(copy.endx),
starty(copy.starty),
endy(copy.endy),
speed(copy.speed),
alpha(0),
alphaMultiplier(copy.alphaMultiplier),
justification(copy.justification),
credits(copy.credits),
current(0),
done(false){
}

OptionCredits::Sequence::~Sequence(){
}

const OptionCredits::Sequence & OptionCredits::Sequence::operator=(const OptionCredits::Sequence & copy){
    type = copy.type;
    x = copy.x;
    y = copy.y;
    startx = copy.startx;
    endx = copy.endx;
    starty = copy.starty;
    endy = copy.endy;
    speed = copy.speed;
    alpha = copy.alpha;
    alphaMultiplier = copy.alphaMultiplier;
    justification = copy.justification;
    credits = copy.credits;
    current = 0;
    done = false;
    return *this;
}

void OptionCredits::Sequence::act(){
    if (!done && !credits.empty()){
        if (type == Roll){
        } else if (type == Primary){
            credits[current].act();
            x += speed;
            if (startx > endx){
                alpha = 255 - fabs((double)(((startx+endx)/2) - x)) * alphaMultiplier;
                if (x < endx){
                    next();
                }
            } else if (startx < endx){
                alpha = 255 - fabs((double)(((startx+endx)/2) - x)) * alphaMultiplier;
                if (x > endx){
                    next();
                }
            }
        }
    }
}

void OptionCredits::Sequence::draw(Graphics::Color title, Graphics::Color color, const Graphics::Bitmap & work){
    if (!done && !credits.empty()){
        if (type == Roll){
            int rollY = (int) y;
            for (std::vector<OptionCredits::Block>::const_iterator i = credits.begin(); i != credits.end(); ++i){
                const OptionCredits::Block & block = *i;
                rollY = block.print(x, y, title, color, Menu::menuFontParameter.current()->get(), work, justification);
            }
        } else if (type == Primary){
            Graphics::Bitmap::transBlender(0, 0, 0, alpha);
            credits[current].print(x, y, title, color, Menu::menuFontParameter.current()->get(), work.translucent(), justification);
        }
    }
}

void OptionCredits::Sequence::reset(){
    done = false;
    current = 0;
    if (!credits.empty()){
        if (type == Roll){
            x = startx;
            y = starty;
        } else if (type == Primary){
            x = startx;
            y = starty - (credits[current].size(Menu::menuFontParameter.current()->get())/2);
        }
    }
}

void OptionCredits::Sequence::next(){
    if (type == Primary){
        if (current < credits.size()){
            current++;
            x = startx;
            y = starty - (credits[current].size(Menu::menuFontParameter.current()->get())/2);
            if (current == credits.size()){
                done = true;
            }
        }
    }
}

OptionCredits::OptionCredits(const Gui::ContextBox & parent, const Token * token):
MenuOption(parent, token),
creditsContext(new Menu::Context()),
primaryStart(360),
primaryEnd(280),
primarySpeed(-.2),
primaryAlphaSpeed(2),
rollSpeed(.8),
rollOffset(0),
rollJustification(Block::Center),
music(""),
color(Graphics::makeColor(255,255,255)),
title(Graphics::makeColor(0,255,255)),
clearColor(Graphics::makeColor(0,0,0)){
    /* Always */
    if (jonBirthday()){
        Block birthday("Happy birthday, Jon!");
        creditsPrimary.push_back(birthday);
    }

    if (miguelBirthday()){
        Block birthday("Happy birthday, Miguel!");
        creditsPrimary.push_back(birthday);
    }
    const std::string paintownToken = "(credit-block"
        "(animation (top) (width 200) (height 65) (image 0 \"sprites/logo.png\") (frame (image 0) (time -1))))";
        //"(title \"Paintown\"))";
    TokenReader reader;
    Block paintown(reader.readTokenFromString(paintownToken));
    paintown.addCredit(string("Version ") + Global::getVersionString());
    creditsPrimary.push_back(paintown);
    
    Block programming("Programming");
    programming.addCredit("Jon Rafkind");
    programming.addCredit("Miguel Gavidia");
    creditsPrimary.push_back(programming);
    
    Block levels("Level design");
    levels.addCredit("Jon Rafkind");
    levels.addCredit("Miguel Gavidia");
    creditsPrimary.push_back(levels);
    
    Block contact("Contact");
    contact.addCredit("Website: http://paintown.org");
    contact.addCredit("Email: jon@rafkind.com");
    creditsPrimary.push_back(contact);
    
    if ( *token != "credits" ){
        throw LoadException(__FILE__, __LINE__, "Not a credit menu");
    }

    readName(token);
    
    TokenView view = token->view();
    
    // NOTE Use this to handle legacy additional blocks for the time being
    Block legacyAdditional("");
    bool additionalTitle = true;
    
    while (view.hasMore()){
        try{
            const Token * tok;
            view >> tok;
            if ( *tok == "music" ) {
                /* Set music for credits */
                tok->view() >> music;
            } else if ( *tok == "background" ) {
                /* Create an image and push it back on to vector */
                std::string temp;
                tok->view() >> temp;
                creditsContext->addBackground(temp);
            } else if ( *tok == "anim" || *tok == "animation" ){
                creditsContext->addBackground(tok);
            } else if ( *tok == "additional" ) {
                std::string str;
                TokenView additionalView = tok->view();
                while (additionalView.hasMore()){
                    additionalView >> str;
                    if (additionalTitle){
                        legacyAdditional = Block(str);
                        additionalTitle = false;
                    } else {
                        legacyAdditional.addCredit(str);
                    }
                }
            } else if ( *tok == "titlecolor" ) {
                int r,b,g;
                tok->view() >> r >> g >> b;
                title = Graphics::makeColor( r, g, b );
            } else if ( *tok == "color" ) {
                    int r,b,g;
                    tok->view() >> r >> g >> b;
                    color = Graphics::makeColor( r, g, b );
            } else if ( *tok == "clear-color" ) {
                    int r,b,g;
                    tok->view() >> r >> g >> b;
                    clearColor = Graphics::makeColor( r, g, b );
            } else if (*tok == "credit-block"){
                creditsRoll.push_back(Block(tok));
            } else if (*tok == "primary-credit-block"){
                creditsPrimary.push_back(Block(tok));
            } else if ( *tok == "primary-start" ) {
                tok->view() >> primaryStart;
            } else if ( *tok == "primary-end" ) {
                tok->view() >> primaryEnd;
            } else if ( *tok == "primary-speed" ) {
                tok->view() >> primarySpeed;
            } else if ( *tok == "primary-alpha" ) {
                tok->view() >> primaryAlphaSpeed;
            } else if ( *tok == "roll-speed" ) {
                tok->view() >> rollSpeed;
            } else if ( *tok == "roll-offset" ) {
                tok->view() >> rollOffset;
            } else if ( *tok == "roll-justification" ) {
                std::string justify;
                tok->view() >> justify;
                if (justify == "left"){
                    rollJustification = Block::Left;
                } else if (justify == "center"){
                    rollJustification = Block::Center;
                } else if (justify == "right"){
                    rollJustification = Block::Right;
                }
            } else {
                Global::debug( 3 ) <<"Unhandled menu attribute: "<<endl;
                if (Global::getDebug() >= 3){
                    tok->print(" ");
                }
            }
        } catch ( const TokenException & ex ) {
            throw LoadException(__FILE__, __LINE__, ex, "Menu parse error");
        } catch ( const LoadException & ex ) {
            throw ex;
        }
    }
    
    if (!legacyAdditional.empty()){
        creditsRoll.push_back(legacyAdditional);
    }
	
    input.set(Keyboard::Key_ESC, 0, true, Exit);
    input.set(Joystick::Button2, 0, true, Exit);
}

OptionCredits::~OptionCredits(){
}

void OptionCredits::logic(){
}

class LogicDraw : public Util::Logic, public Util::Draw{
public:
    LogicDraw(OptionCredits & self, const Font & font, InputMap<OptionCredits::CreditKey> & input, Menu::Context & context):
    self(self),
    font(font),
    input(input),
    quit(false),
    context(context),
    currentX(self.primaryStart),
    currentY(220 - (!self.creditsPrimary.empty() ? (self.creditsPrimary[0].size(font)/2) : 0)),
    currentBlock(0),
    blockAlpha(0),
    startPrimary(true),
    startRoll(false),
    /* FIXME hardcoded resolution */
    rollY(480),
    maxCredits(0){
        for (std::vector<OptionCredits::Block>::const_iterator i = self.creditsRoll.begin(); i != self.creditsRoll.end(); ++i){
            const OptionCredits::Block & block = *i;
            maxCredits+=block.size(font);
        }
    }

    OptionCredits & self;
    const Font & font;
    InputMap<OptionCredits::CreditKey> & input;
    bool quit;
    Menu::Context & context;
    double currentX;
    double currentY;
    unsigned int currentBlock;
    int blockAlpha;
    bool startPrimary;
    bool startRoll;
    double rollY;
    int maxCredits;

    void run(){
        vector<InputMap<OptionCredits::CreditKey>::InputEvent> out = InputManager::getEvents(input, InputSource());
        for (vector<InputMap<OptionCredits::CreditKey>::InputEvent>::iterator it = out.begin(); it != out.end(); it++){
            const InputMap<OptionCredits::CreditKey>::InputEvent & event = *it;
            if (event.enabled){
                if (event.out == OptionCredits::Exit){
                    quit = true;
                    context.finish();
                }
            }
        }

        if (startRoll){
            rollY -= self.rollSpeed;
            if (rollY < -(maxCredits * 1.1)){
                /* FIXME: hard coded resolution */
                rollY = 480;
                startRoll = false;
            }
        } else {
            handlePrimary();
        }
        
        context.act();
    }

    bool done(){
        return quit;
    }

    double ticks(double system){
        return system * Global::ticksPerSecond(90);
    }
    
    void draw(const Graphics::Bitmap & buffer){
        /* FIXME: hard coded resolution */
        Graphics::StretchedBitmap work(640, 480, buffer, Graphics::qualityFilterName(Configuration::getQualityFilter()));
        work.fill(self.clearColor);
        work.start();
        //background.Blit(work);
        context.render(NULL, work);
        
        if (startRoll){
            int y = (int) rollY;

            for (std::vector<OptionCredits::Block>::const_iterator i = self.creditsRoll.begin(); i != self.creditsRoll.end(); ++i){
                const OptionCredits::Block & block = *i;
                y = block.print(self.rollOffset, y, self.title, self.color, font, work, self.rollJustification);
            }
        } else {
            Graphics::Bitmap::transBlender(0, 0, 0, blockAlpha);
            self.creditsPrimary[currentBlock].print(currentX, currentY, self.title, self.color, font, work.translucent(), OptionCredits::Block::Center);
        }
        
        work.finish();
        
        buffer.BlitToScreen();
    }
    
    void handlePrimary(){
        if (!self.creditsPrimary.empty()){
            if (!startPrimary){
                nextPrimary();
                // FIXME use spaces not static positions
                blockAlpha = 0;
                currentX = self.primaryStart;
                currentY = 220 - (self.creditsPrimary[currentBlock].size(font)/2);
                startPrimary = true;
            } else {
                // Act
                self.creditsPrimary[currentBlock].act();
                currentX += self.primarySpeed;
                if (self.primaryStart > self.primaryEnd){
                    if (currentX >= (self.primaryStart+self.primaryEnd)/2){
                        if (blockAlpha < 255){
                            blockAlpha +=self.primaryAlphaSpeed;
                        } else {
                            blockAlpha = 255;
                        }
                    } else if (currentX < (self.primaryStart+self.primaryEnd)/2){
                        if (blockAlpha > 0){
                            blockAlpha -=self.primaryAlphaSpeed;
                        } else {
                            blockAlpha = 0;
                        }
                    }
                    if (currentX < self.primaryEnd){
                        startPrimary = false;
                    }
                } else if (self.primaryStart < self.primaryEnd){
                    if (currentX <= (self.primaryStart+self.primaryEnd)/2){
                        if (blockAlpha < 255){
                            blockAlpha +=self.primaryAlphaSpeed;
                        } else {
                            blockAlpha = 255;
                        }
                    } else if (currentX > (self.primaryStart+self.primaryEnd)/2){
                        if (blockAlpha > 0){
                            blockAlpha -=self.primaryAlphaSpeed;
                        } else {
                            blockAlpha = 0;
                        }
                    }
                    if (currentX > self.primaryEnd){
                        startPrimary = false;
                    }
                }
            }
        } else {
            startPrimary = false;
            startRoll = true;
        }
    }
    
    void nextPrimary(){
        if (currentBlock < self.creditsPrimary.size()){
            currentBlock++;
            if (currentBlock == self.creditsPrimary.size()){
                startRoll = true;
                startPrimary = false;
                currentBlock = 0;
            }
        }
    }
};

void OptionCredits::run(const Menu::Context & context){
    Menu::Context localContext(context, *creditsContext);
    localContext.initialize();

    if (!music.empty()){
        if (Music::loadSong(Storage::instance().find(Filesystem::RelativePath(music)).path())){
            Music::pause();
            Music::play();
        }
    }

    const Font & vFont = Menu::menuFontParameter.current()->get();
    
    LogicDraw loop(*this, vFont, input, localContext);
    Util::standardLoop(loop, loop);

    InputManager::waitForRelease(input, InputSource(), Exit);
    throw Menu::Reload(__FILE__, __LINE__);
}

OptionDummy::OptionDummy(const Gui::ContextBox & parent, const Token *token):
MenuOption(parent, token){
    if ( *token != "dummy" ){
        throw LoadException(__FILE__, __LINE__, "Not dummy option");
    }

    readName(token);

    if (getText().empty()){
        this->setText("Dummy");
    }
    
    setRunnable(false);
}

OptionDummy::OptionDummy(const Gui::ContextBox & parent, const std::string &name):
MenuOption(parent, 0){
    if (name.empty()){
	throw LoadException(__FILE__, __LINE__, "No name given to dummy");
    }
    this->setText(name);
    
    
    setRunnable(false);
}

OptionDummy::~OptionDummy(){
}

void OptionDummy::logic(){
}

void OptionDummy::run(const Menu::Context & context){
}

OptionFullscreen::OptionFullscreen(const Gui::ContextBox & parent, const Token *token):
MenuOption(parent, token),
lblue(255),
lgreen(255),
rblue(255),
rgreen(255){
	setRunnable(false);
	
	if ( *token != "fullscreen" )
		throw LoadException(__FILE__, __LINE__, "Not fullscreen option");

        readName(token);
}

OptionFullscreen::~OptionFullscreen()
{
	// Nothing
}
    
std::string OptionFullscreen::getText() const {
    ostringstream out;
    out << MenuOption::getText() << ": " << (Configuration::getFullscreen() ? "Yes" : "No");
    return out.str();
}

void OptionFullscreen::logic(){;
}

static void changeScreenMode(){
    Configuration::setFullscreen(!Configuration::getFullscreen());
    int gfx = (Configuration::getFullscreen() ? Global::FULLSCREEN : Global::WINDOWED);
    Graphics::setGraphicsMode(gfx, Global::getScreenWidth(), Global::getScreenHeight());
}

void OptionFullscreen::run(const Menu::Context & context){
    changeScreenMode();
}

bool OptionFullscreen::leftKey(){
    changeScreenMode();
    return true;
}
bool OptionFullscreen::rightKey(){
    changeScreenMode();
    return true;
}

OptionFps::OptionFps(const Gui::ContextBox & parent, const Token * token):
MenuOption(parent, token){
    readName(token);
    setRunnable(false);
}

void OptionFps::logic(){
}

void OptionFps::run(const Menu::Context & context){
}

std::string OptionFps::getText() const {
    ostringstream out;
    out << "Frames per second: " << Global::TICS_PER_SECOND;
    return out.str();
}

bool OptionFps::leftKey(){
    Global::setTicksPerSecond(Global::TICS_PER_SECOND - 1);
    Configuration::setFps(Global::TICS_PER_SECOND);
    return true;
}

bool OptionFps::rightKey(){
    Global::setTicksPerSecond(Global::TICS_PER_SECOND + 1);
    Configuration::setFps(Global::TICS_PER_SECOND);
    return true;
}

OptionFps::~OptionFps(){
}

OptionQualityFilter::OptionQualityFilter(const Gui::ContextBox & parent, const Token * token):
MenuOption(parent, token){
    readName(token);
    setRunnable(false);
}
              
std::string OptionQualityFilter::getText() const {
    ostringstream out;
    out << MenuOption::getText() << ": " << Configuration::getQualityFilter();
    return out.str();
}

void OptionQualityFilter::logic(){
}
    
bool OptionQualityFilter::leftKey(){
    string quality = Configuration::getQualityFilter();
    if (quality == "none"){
        quality = "hqx";
    } else if (quality == "hqx"){
        quality = "xbr";
    } else if (quality == "xbr"){
        quality = "none";
    }
    Configuration::setQualityFilter(quality);
    return true;
}

bool OptionQualityFilter::rightKey(){
    string quality = Configuration::getQualityFilter();
    if (quality == "none"){
        quality = "xbr";
    } else if (quality == "hqx"){
        quality = "none";
    } else if (quality == "xbr"){
        quality = "hqx";
    }
    Configuration::setQualityFilter(quality);
    return true;

}

void OptionQualityFilter::run(const Menu::Context & context){
}

OptionQualityFilter::~OptionQualityFilter(){
}

OptionInvincible::OptionInvincible(const Gui::ContextBox & parent, const Token *token):
MenuOption(parent, token),
lblue(255),
lgreen(255),
rblue(255),
rgreen(255){
	setRunnable(false);
	
	if ( *token != "invincible" )
		throw LoadException(__FILE__, __LINE__, "Not invincible option");

        readName(token);
}

OptionInvincible::~OptionInvincible()
{
	// Nothing
}
                
std::string OptionInvincible::getText() const {
    ostringstream out;
    out << MenuOption::getText() << ": " << (Configuration::getInvincible() ? "Yes" : "No");
    return out.str();
}

void OptionInvincible::logic(){
}

void OptionInvincible::run(const Menu::Context & context){
}

bool OptionInvincible::leftKey(){
	Configuration::setInvincible(!Configuration::getInvincible());
	return true;
}
bool OptionInvincible::rightKey(){
	Configuration::setInvincible(!Configuration::getInvincible());
	return true;
}

static OptionJoystick::JoystickType convertToKey(const std::string &k){
    std::string temp = k;
    for(unsigned int i=0;i<temp.length();i++){
        temp[i] = tolower(temp[i]);
    }

    if (temp == "up") return OptionJoystick::Up;
    if (temp == "down") return OptionJoystick::Down;
    if (temp == "left") return OptionJoystick::Left;
    if (temp == "right") return OptionJoystick::Right;
    if (temp == "jump") return OptionJoystick::Jump;
    if (temp == "attack1") return OptionJoystick::Attack1;
    if (temp == "attack2") return OptionJoystick::Attack2;
    if (temp == "attack3") return OptionJoystick::Attack3;
    if (temp == "attack4") return OptionJoystick::Attack4;
    if (temp == "attack5") return OptionJoystick::Attack5;
    if (temp == "attack6") return OptionJoystick::Attack6;

    return OptionJoystick::Invalidkey;
}

static Configuration::JoystickInput getKey(int player, OptionJoystick::JoystickType k){
    switch(k){
        case OptionJoystick::Up:
            return Configuration::getJoystickUp(player);
        case OptionJoystick::Down:
            return Configuration::getJoystickDown(player);
        case OptionJoystick::Left:
            return Configuration::getJoystickLeft(player);
        case OptionJoystick::Right:
            return Configuration::getJoystickRight(player);
        case OptionJoystick::Jump:
            return Configuration::getJoystickJump(player);
        case OptionJoystick::Attack1:
            return Configuration::getJoystickAttack1(player);
        case OptionJoystick::Attack2:
            return Configuration::getJoystickAttack2(player);
        case OptionJoystick::Attack3:
            return Configuration::getJoystickAttack3(player);
        case OptionJoystick::Attack4:
            return Configuration::getJoystickAttack4(player);
        case OptionJoystick::Attack5:
            return Configuration::getJoystickAttack5(player);
        case OptionJoystick::Attack6:
            return Configuration::getJoystickAttack6(player);
        default:
            break;
    }

    return Configuration::getJoystickUp(player);
}

static void setKey(int player, OptionJoystick::JoystickType k, Configuration::JoystickInput key){
    switch(k){
        case OptionJoystick::Up:
            Configuration::setJoystickUp(player, key);
            break;
        case OptionJoystick::Down:
            Configuration::setJoystickDown(player, key);
            break;
        case OptionJoystick::Left:
            Configuration::setJoystickLeft(player, key);
            break;
        case OptionJoystick::Right:
            Configuration::setJoystickRight(player, key);
            break;
        case OptionJoystick::Jump:
            Configuration::setJoystickJump(player, key);
            break;
        case OptionJoystick::Attack1:
            Configuration::setJoystickAttack1(player, key);
            break;
        case OptionJoystick::Attack2:
            Configuration::setJoystickAttack2(player, key);
            break;
        case OptionJoystick::Attack3:
            Configuration::setJoystickAttack3(player, key);
            break;
        case OptionJoystick::Attack4:
            Configuration::setJoystickAttack4(player, key);
            break;
        case OptionJoystick::Attack5:
            Configuration::setJoystickAttack5(player, key);
            break;
        case OptionJoystick::Attack6:
            Configuration::setJoystickAttack6(player, key);
            break;
        default:
            break;
    }
}

static Configuration::JoystickInput readJoystick(){
    vector<Joystick::Key> keys;
    keys.push_back(Joystick::Up);
    keys.push_back(Joystick::Down);
    keys.push_back(Joystick::Left);
    keys.push_back(Joystick::Right);
    keys.push_back(Joystick::Button1);
    keys.push_back(Joystick::Button2);
    keys.push_back(Joystick::Button3);
    keys.push_back(Joystick::Button4);
    keys.push_back(Joystick::Button5);
    keys.push_back(Joystick::Button6);

    InputMap<Joystick::Key> input;
    for (vector<Joystick::Key>::iterator it = keys.begin(); it != keys.end(); it++){
        input.set(*it, 0, true, *it);
    }
    input.set(Keyboard::Key_ESC, 0, true, Joystick::Invalid);

    while (true){
        InputManager::poll();
        vector<InputMap<Joystick::Key>::InputEvent> out = InputManager::getEvents(input, InputSource());
        for (vector<InputMap<Joystick::Key>::InputEvent>::iterator it = out.begin(); it != out.end(); it++){
            const InputMap<Joystick::Key>::InputEvent & event = *it;
            if (event.enabled){
                Global::debug(1) << "Press: " << event.out << std::endl;
                if (event.out == Joystick::Invalid){
                    InputManager::waitForRelease(input, InputSource(), Joystick::Invalid);
                    throw Exception::Return(__FILE__, __LINE__);
                }
                return event.out;
            }
        }
        Util::rest(1);
    }

    /* control probably shouldn't get here.. */
    return Joystick::Up;
}

OptionJoystick::OptionJoystick(const Gui::ContextBox & parent, const Token *token):
MenuOption(parent, token),
name(""),
player(-1),
type(Invalidkey),
keyCode(0){
    if (*token != "joystick"){
        throw LoadException(__FILE__, __LINE__, "Not joystick option");
    }

    TokenView view = token->view();
    while (view.hasMore()){
        try{
            const Token * tok;
            view >> tok;
            if ( *tok == "name" ){
                tok->view() >> name;
            } else if ( *tok == "player" ) {
                tok->view() >> player;
            } else if ( *tok == "type" ) {
                std::string temp;
                tok->view() >> temp;
                type = convertToKey(temp);
            } else {
                Global::debug( 3 ) <<"Unhandled menu attribute: "<<endl;
                tok->print(" ");
            }
        } catch ( const TokenException & ex ) {
            throw LoadException(__FILE__, __LINE__, ex, "Menu parse error");
        } catch ( const LoadException & ex ) {
            // delete current;
            throw ex;
        }
    }

    if (name.empty()){
        throw LoadException(__FILE__, __LINE__, "No name set, this option should have a name!");
    }

    if (type == Invalidkey){
        throw LoadException(__FILE__, __LINE__, "Invalid joystick button, should be up, down, left, right, up, down, jump, attack1-6!");
    }

    if (player == -1){
        throw LoadException(__FILE__, __LINE__, "Player not specified in joystick configuration");
    }

    ostringstream out;
    out << name << ": " << Joystick::keyToName(getKey(player, type));
    setText(out.str());
}

OptionJoystick::~OptionJoystick(){
    // Nothing
}

void OptionJoystick::logic(){
    /*
    char temp[255];
    sprintf( temp, "%s: %s", name.c_str(), Joystick::keyToName(getKey(player,type)));
    setText(std::string(temp));
    */
}

void OptionJoystick::run(const Menu::Context & context){
    /*
    //int x, y, width, height;
    const Font & vFont = Menu::menuFontParameter.current()->get();
    const char * message = "Press a joystick button!";
    const int width = vFont.textLength(message) + 10;
    const int height = vFont.getHeight() + 10;
    // const int x = (getParent()->getWork()->getWidth()/2) - (width/2);
    // const int y = (getParent()->getWork()->getHeight()/2) - (height/2);
    const int x = Menu::Menu::Width / 2 - width/2;
    const int y = Menu::Menu::Height / 2 - height/2;
    Box dialog;
    dialog.location.setPosition(Gui::AbsolutePoint(0,0));
    dialog.location.setDimensions(vFont.textLength(message) + 10, vFont.getHeight() + 10);
    dialog.transforms.setRadius(0);
    dialog.colors.body = Graphics::makeColor(0,0,0);
    dialog.colors.bodyAlpha = 200;
    dialog.colors.border = Graphics::makeColor(255,255,255);
    dialog.colors.borderAlpha = 255;
    Graphics::Bitmap temp = Graphics::Bitmap::temporaryBitmap(width,height);
    dialog.render(temp, vFont);
    vFont.printf( 5, 5, Graphics::makeColor(255,255,255), temp, message, -1);
    temp.BlitToScreen(x,y);
    
    setKey(player, type, readJoystick());

    ostringstream out;
    out << name << ": " << Joystick::keyToName(getKey(player, type));
    setText(out.str());
    */

    Graphics::Bitmap temp(Menu::Menu::Width, Menu::Menu::Height);
    // Menu::Context tempContext = context;
    Menu::Context tempContext(context);
    tempContext.initialize();
    Menu::InfoBox keyDialog;
    // keyDialog.setFont(tempContext.getFont());
    //keyDialog.location.set(-1,-1,1,1);
    const int width = temp.getWidth();
    const int height = temp.getHeight();
    const Font & font = Menu::menuFontParameter.current()->get();
    const int radius = 15;
    keyDialog.setText("Press a joystick button!");
    keyDialog.initialize(font);
    keyDialog.location.setDimensions(font.textLength("Press a joystick button!") + radius, font.getHeight() + radius);
    keyDialog.location.setCenterPosition(Gui::RelativePoint(0, 0));
    // keyDialog.location.setPosition(Gui::AbsolutePoint((width/2)-(keyDialog.location.getWidth()/2), (height/2)-(keyDialog.location.getHeight()/2)));
    // keyDialog.location.setPosition2(Gui::AbsolutePoint((
    keyDialog.transforms.setRadius(radius);
    keyDialog.colors.body = Graphics::makeColor(0,0,0);
    keyDialog.colors.bodyAlpha = 180;
    keyDialog.colors.border = Graphics::makeColor(255,255,255);
    keyDialog.colors.borderAlpha = 255;
    keyDialog.open();
    InputManager::waitForClear();
    while (!InputManager::anyInput() && keyDialog.isActive()){
        InputManager::poll();
	keyDialog.act(font);
        /*
	if (keyDialog.isActive()){
            InputManager::poll();
	}
        */
	tempContext.act();
	tempContext.render(0, temp);
	keyDialog.render(temp, font);
	temp.BlitToScreen();
    }
    tempContext.finish();
    setKey(player, type, readJoystick());
    InputManager::waitForClear();
    
    ostringstream out;
    out << name << ": " << Joystick::keyToName(getKey(player, type));
    setText(out.str());

    /*
    Keyboard key;
    keyCode = readKey(key);
    setKey(player,type, keyCode);
    */
}

static OptionKey::keyType convertToKeyboardKey(const std::string &k){
    std::string temp = k;
    for(unsigned int i=0;i<temp.length();i++){
        temp[i] = tolower(temp[i]);
    }
    if (temp == "up") return OptionKey::up;
    if (temp == "down") return OptionKey::down;
    if (temp == "left") return OptionKey::left;
    if (temp == "right") return OptionKey::right;
    if (temp == "jump") return OptionKey::jump;
    if (temp == "attack1") return OptionKey::attack1;
    if (temp == "attack2") return OptionKey::attack2;
    if (temp == "attack3") return OptionKey::attack3;
    if (temp == "attack4") return OptionKey::attack4;
    if (temp == "attack5") return OptionKey::attack5;
    if (temp == "attack6") return OptionKey::attack6;

    return OptionKey::invalidkey;
}

static int getKey(int player, OptionKey::keyType k){
    switch(k){
        case OptionKey::up:
            return Configuration::getUp(player);
            break;
        case OptionKey::down:
            return Configuration::getDown(player);
            break;
        case OptionKey::left:
            return Configuration::getLeft(player);
            break;
        case OptionKey::right:
            return Configuration::getRight(player);
            break;
        case OptionKey::jump:
            return Configuration::getJump(player);
            break;
        case OptionKey::attack1:
            return Configuration::getAttack1(player);
            break;
        case OptionKey::attack2:
            return Configuration::getAttack2(player);
            break;
        case OptionKey::attack3:
            return Configuration::getAttack3(player);
            break;
        case OptionKey::attack4:
            return Configuration::getAttack4(player);
        case OptionKey::attack5:
            return Configuration::getAttack5(player);
        case OptionKey::attack6:
            return Configuration::getAttack6(player);
        default:
            break;
    }

    return 0;
}

static void setKey(int player, OptionKey::keyType k, int key){
    switch(k){
        case OptionKey::up:
            Configuration::setUp(player, key);
            break;
        case OptionKey::down:
            Configuration::setDown(player, key);
            break;
        case OptionKey::left:
            Configuration::setLeft(player, key);
            break;
        case OptionKey::right:
            Configuration::setRight(player, key);
            break;
        case OptionKey::jump:
            Configuration::setJump(player, key);
            break;
        case OptionKey::attack1:
            Configuration::setAttack1(player, key);
            break;
        case OptionKey::attack2:
            Configuration::setAttack2(player, key);
            break;
        case OptionKey::attack3:
            Configuration::setAttack3(player, key);
            break;
        case OptionKey::attack4:
            Configuration::setAttack4(player, key);
            break;
        case OptionKey::attack5:
            Configuration::setAttack5(player, key);
            break;
        case OptionKey::attack6:
            Configuration::setAttack6(player, key);
            break;
        default:
            break;
    }
}

/*
static int readKey( Keyboard & key ){
    int k = key.readKey();
    key.wait();
    return k;
}
*/

OptionKey::OptionKey(const Gui::ContextBox & parent, const Token *token):
MenuOption(parent, token),
name(""),
player(-1),
type(invalidkey),
keyCode(0){
    if ( *token != "key" )
        throw LoadException(__FILE__, __LINE__, "Not key option");

    TokenView view = token->view();
    while (view.hasMore()) {
        try {
            const Token * tok;
            view >> tok;
            if ( *tok == "name" ) {
                tok->view() >> name;
            } else if ( *tok == "player" ) {
                tok->view() >> player;
            } else if ( *tok == "type" ) {
                std::string temp;
                tok->view() >> temp;
                type = convertToKeyboardKey(temp);
            } else {
                Global::debug( 3 ) <<"Unhandled menu attribute: "<<endl;
                tok->print(" ");
            }
        } catch ( const TokenException & ex ){
            throw LoadException(__FILE__, __LINE__, ex, "Menu parse error");
        } catch ( const LoadException & ex ) {
            // delete current;
            throw ex;
        }
    }

    if(name.empty())throw LoadException(__FILE__, __LINE__, "No name set, this option should have a name!");
    if(type == invalidkey)throw LoadException(__FILE__, __LINE__, "Invalid key, should be up, down, left, right, up, down, jump, attack1-6!");
    if(player == -1)throw LoadException(__FILE__, __LINE__, "Player not specified in key configuration");

    char temp[255];
    sprintf( temp, "%s: %s", name.c_str(), Keyboard::keyToName(getKey(player,type)));
    setText(std::string(temp));
}

OptionKey::~OptionKey(){
	// Nothing
}

void OptionKey::logic(){
    char temp[255];
    sprintf( temp, "%s: %s", name.c_str(), Keyboard::keyToName(getKey(player,type)));
    setText(std::string(temp));
}

void OptionKey::run(const Menu::Context & context){
    // Do dialog
    //Box::messageDialog(Menu::Menu::Width, Menu::Menu::Height, "Press a Key!",2);

    /*
    Keyboard key;
    key.wait();
    */
    Graphics::Bitmap temp(Menu::Menu::Width, Menu::Menu::Height);
    // Menu::Context tempContext = context;
    Menu::Context tempContext(context);
    tempContext.initialize();
    Menu::InfoBox keyDialog;
    // keyDialog.setFont(tempContext.getFont());
    //keyDialog.location.set(-1,-1,1,1);
    const int width = temp.getWidth();
    const int height = temp.getHeight();
    const Font & font = Menu::menuFontParameter.current()->get();
    const int radius = 15;
    keyDialog.setText("Press a Key!");
    keyDialog.initialize(font);
    keyDialog.location.setDimensions(font.textLength("Press a Key!") + radius, font.getHeight() + radius);
    keyDialog.location.setCenterPosition(Gui::RelativePoint(0, 0));
    // keyDialog.location.setPosition(Gui::AbsolutePoint((width/2)-(keyDialog.location.getWidth()/2), (height/2)-(keyDialog.location.getHeight()/2)));
    // keyDialog.location.setPosition2(Gui::AbsolutePoint((
    keyDialog.transforms.setRadius(radius);
    keyDialog.colors.body = Graphics::makeColor(0,0,0);
    keyDialog.colors.bodyAlpha = 180;
    keyDialog.colors.border = Graphics::makeColor(255,255,255);
    keyDialog.colors.borderAlpha = 255;
    keyDialog.open();
    InputManager::waitForClear();
    while (!InputManager::anyInput() && keyDialog.isActive()){
        InputManager::poll();
	keyDialog.act(font);
        /*
	if (keyDialog.isActive()){
            InputManager::poll();
	}
        */
	tempContext.act();
	tempContext.render(0, temp);
	keyDialog.render(temp, font);
	temp.BlitToScreen();
    }
    tempContext.finish();
    keyCode = InputManager::readKey();
    setKey(player,type, keyCode);
    InputManager::waitForClear();
}

OptionLevel::OptionLevel(const Gui::ContextBox & parent, const Token *token, int * set, int value):
MenuOption(parent, token),
set(set),
value(value){
  // Nothing
}

OptionLevel::~OptionLevel(){
}

void OptionLevel::logic(){
}

void OptionLevel::run(const Menu::Context & context){
    *set = value;
    throw Menu::MenuException(__FILE__, __LINE__);
}

OptionLives::OptionLives(const Gui::ContextBox & parent, const Token * token):
MenuOption(parent, token),
lblue(255),
lgreen(255),
rblue(255),
rgreen(255){
	setRunnable(false);
	
	if ( *token != "lives" ){
		throw LoadException(__FILE__, __LINE__,  "Not lives option" );
	}

        readName(token);
}

OptionLives::~OptionLives(){
}
    
std::string OptionLives::getText() const {
    ostringstream out;
    out << MenuOption::getText() << ": " << Configuration::getLives();
    return out.str();
}

void OptionLives::logic(){

}

void OptionLives::run(const Menu::Context & context){
}

bool OptionLives::leftKey(){
	Configuration::setLives(Configuration::getLives() - 1);
	if ( Configuration::getLives() < 1 ){
		Configuration::setLives(1);
	}
	
	return false;
}

bool OptionLives::rightKey(){
	Configuration::setLives( Configuration::getLives() + 1 );
	
	return false;
}

OptionMenu::OptionMenu(const Gui::ContextBox & parent, const Token *token, const Menu::OptionFactory & factory):
MenuOption(parent, token),
menu(0){
    if (*token != "menu"){
        throw LoadException(__FILE__, __LINE__, "Not a menu");
    }

    if (token->numTokens() == 1){
        std::string temp;
        token->view() >> temp;
        menu = new Menu::Menu(Storage::instance().find(Filesystem::RelativePath(temp)), factory);
    } else {
        menu = new Menu::Menu(token, factory);
    }

    this->setText(menu->getName());
    this->setInfoText(menu->getInfo());
    
    // Lets check if this menu is going bye bye
    //if ( menu->checkRemoval() ) setForRemoval(true);
}

OptionMenu::~OptionMenu(){
    // Delete our menu
    if (menu){
        delete menu;
    }
}

void OptionMenu::logic(){
    // Nothing
}

void OptionMenu::run(const Menu::Context & context){
    // Do our new menu
    try{
        menu->run(context);
    } catch (const Exception::Return ignore){
        throw Menu::Reload(__FILE__, __LINE__);
    }
}

OptionNpcBuddies::OptionNpcBuddies(const Gui::ContextBox & parent, const Token * token):
MenuOption(parent, token),
lblue(255),
lgreen(255),
rblue(255),
rgreen(255){
    setRunnable(false);

    if ( *token != "npc" ){
        throw LoadException(__FILE__, __LINE__,  "Not npc option" );
    }

    readName(token);
}

OptionNpcBuddies::~OptionNpcBuddies(){
	// Nothing
}
        
std::string OptionNpcBuddies::getText() const {
    ostringstream out;
    out << MenuOption::getText() << ": " << Configuration::getNpcBuddies();
    return out.str();
}

void OptionNpcBuddies::logic(){

}

void OptionNpcBuddies::run(const Menu::Context & context){
}

bool OptionNpcBuddies::leftKey(){
	Configuration::setNpcBuddies(Configuration::getNpcBuddies() - 1);
	if ( Configuration::getNpcBuddies() < 1 ){
		Configuration::setNpcBuddies(1);
	}
	
	return false;
}

bool OptionNpcBuddies::rightKey(){
	Configuration::setNpcBuddies( Configuration::getNpcBuddies() + 1 );
	rblue = rgreen = 0;
	return false;
}

OptionPlayMode::OptionPlayMode(const Gui::ContextBox & parent, const Token *token):
MenuOption(parent, token),
lblue(255),
lgreen(255),
rblue(255),
rgreen(255){
    setRunnable(false);

    if ( *token != "play-mode" ){
        throw LoadException(__FILE__, __LINE__, "Not a play-mode");
    }

    readName(token);
}

OptionPlayMode::~OptionPlayMode(){
    // Nothing
}
    
std::string OptionPlayMode::getText() const {
    ostringstream out;
    out << MenuOption::getText() << ": ";

    /* TODO: language translations of these */
    if (Configuration::getPlayMode() == Configuration::FreeForAll){
        out << "Free for all";
    } else if (Configuration::getPlayMode() == Configuration::Cooperative){
        out << "Cooperative";
    }

    return out.str();
}

void OptionPlayMode::logic(){
    
}

void OptionPlayMode::run(const Menu::Context & context){
}
    
void OptionPlayMode::changeMode(){
    if (Configuration::getPlayMode() == Configuration::FreeForAll){
        Configuration::setPlayMode(Configuration::Cooperative);
    } else if (Configuration::getPlayMode() == Configuration::Cooperative){
        Configuration::setPlayMode(Configuration::FreeForAll);
    }
}

bool OptionPlayMode::leftKey(){
    changeMode();
    lblue = lgreen = 0;
    return true;
}

bool OptionPlayMode::rightKey(){
    changeMode();
    rblue = rgreen = 0;
    return true;
}

OptionReturn::OptionReturn(const Gui::ContextBox & parent, const Token * token):
MenuOption(parent, token){
    if (*token != "return"){
        throw LoadException(__FILE__, __LINE__, "Not a return option");
    }
    readName(token);
}

void OptionReturn::logic(){
}

/* maybe this option is misnamed, but its supposed to quit the current game
 * and go back to the main menu
 */
void OptionReturn::run(const Menu::Context & context){
    throw Exception::Quit(__FILE__, __LINE__);
}

OptionReturn::~OptionReturn(){
}

OptionContinue::OptionContinue(const Gui::ContextBox & parent, const Token * token):
MenuOption(parent, token){
    if (*token != "continue"){
        throw LoadException(__FILE__, __LINE__, "Not a continue option");
    }
    readName(token);
}

void OptionContinue::logic(){
}

void OptionContinue::run(const Menu::Context & context){
    throw Exception::Return(__FILE__, __LINE__);
}

OptionContinue::~OptionContinue(){
}

OptionQuit::OptionQuit(const Gui::ContextBox & parent, const Token *token):
MenuOption(parent, token){
    if ( *token != "quit" ){
        throw LoadException(__FILE__, __LINE__, "Not quit option");
    }

    readName(token);
}

OptionQuit::OptionQuit(const Gui::ContextBox & parent, const std::string &name):
MenuOption(parent, 0){
    if (name.empty()){
	throw LoadException(__FILE__, __LINE__, "No name given to quit");
    }
    this->setText(name);
}

OptionQuit::~OptionQuit(){
}

void OptionQuit::logic(){
}

void OptionQuit::run(const Menu::Context & context){
    throw ShutdownException();
}


#if defined(WINDOWS) && defined(doesnt_work_yet)
#include <windows.h>
#include <stdio.h>

/* contributed by Roy Underthump from allegro.cc */
static vector<ScreenSize> getScreenResolutions(){
    HWND hwnd;
    HDC  hdc;

    // int iPixelFormat;
    int descerr;
    int retval;

    DEVMODE d;

    PIXELFORMATDESCRIPTOR pfd;

    hwnd = GetDesktopWindow();
    hdc  = GetDC(hwnd);

    vector<ScreenSize> modes;

    for (int i = 0;; i++){
        retval = EnumDisplaySettings(0,i,&d);
        if (!retval){
            break;
        }

        descerr = DescribePixelFormat(hdc, i+1, sizeof(pfd), &pfd);
        if(!descerr){
            continue;
        }

        /*
           printf("\n#%d bpp %d width %d height %d colorbits %d fps %d",i,d.dmBitsPerPel,
           d.dmPelsWidth, d.dmPelsHeight,pfd.cColorBits,d.dmDisplayFrequency);

           if(pfd.dwFlags & PFD_SUPPORT_OPENGL)printf(" OGL OK");
         */
        modes.push_back(ScreenSize(d.dmPelsWidth, d.dmPelsHeight));
    }

    if (modes.empty()){
        modes.push_back(ScreenSize(640,480));
    }

    return modes;
}
#else
static vector<ScreenSize> getScreenResolutions(){
	vector<ScreenSize> modes;
	modes.push_back(ScreenSize(320, 240));
	modes.push_back(ScreenSize(640, 480));
	modes.push_back(ScreenSize(800, 600));
	modes.push_back(ScreenSize(960, 720));
	modes.push_back(ScreenSize(1024, 768));
	modes.push_back(ScreenSize(1280, 960));
	modes.push_back(ScreenSize(1600, 1200));
	return modes;
}
#endif

static bool doSort(const ScreenSize & a, const ScreenSize & b){
    return (a.w * a.h) < (b.w * b.h);
}

static vector<ScreenSize> sortResolutions(const vector<ScreenSize> & modes){
    vector<ScreenSize> copy(modes);
    std::sort(copy.begin(), copy.end(), doSort);
    return copy;
}

OptionScreenSize::OptionScreenSize(const Gui::ContextBox & parent, const Token *token):
MenuOption(parent, token),
name(""),
lblue(255),
lgreen(255),
rblue(255),
rgreen(255){
    setRunnable(false);

    Global::debug(1) << "Get screen resolution" << endl;
    modes = sortResolutions(getScreenResolutions());

    if (Global::getDebug() >= 1){
        for (vector<ScreenSize>::iterator it = modes.begin(); it != modes.end(); it++){
            Global::debug(1) << "Screen size: " << it->w << " x " << it->h << endl;
        }
    }

    if ( *token != "screen-size" ){
        throw LoadException(__FILE__, __LINE__, "Not a screen-size");
    }

    readName(token);
}

OptionScreenSize::~OptionScreenSize(){
    // Nothing
}

void OptionScreenSize::logic(){
    ostringstream temp;
    temp << "Screen size: " << Configuration::getScreenWidth() << " x " << Configuration::getScreenHeight();
    setText(temp.str());

}

void OptionScreenSize::run(const Menu::Context & context){
}

void OptionScreenSize::setMode(int width, int height){
    if (width != Configuration::getScreenWidth() ||
        height != Configuration::getScreenHeight()){

        Global::debug(1) << "Changing mode to " << width << " x " << height << endl;
        int gfx = Configuration::getFullscreen() ? Global::FULLSCREEN : Global::WINDOWED;
        int ok = Graphics::setGraphicsMode(gfx, width, height);
        if (ok == 0){
            Global::debug(1) << "Success" << endl;
            Configuration::setScreenWidth(width);
            Configuration::setScreenHeight(height);
        } else {
            Global::debug(1) << "Fail" << endl;
            int ok = Graphics::setGraphicsMode(gfx, Configuration::getScreenWidth(), Configuration::getScreenHeight());
            Global::debug(1) << "Set mode back " << ok << endl;
        }
    }
}

/*
static int modes[][2] = {{640,480}, {800,600}, {1024,768}, {1280,1024}, {1600,1200}};
// static int max_modes = sizeof(modes) / sizeof(int[]);
static int max_modes = 5;
*/

int OptionScreenSize::findMode(int width, int height){
    for (int mode = 0; mode < (int) modes.size(); mode++){
        if (modes[mode].w == width && modes[mode].h == height){
            return mode;
        }
    }
    return -1;
}

bool OptionScreenSize::leftKey(){
    int mode = findMode(Configuration::getScreenWidth(), Configuration::getScreenHeight());
    if (mode >= 1 && mode < (int)modes.size()){
        mode -= 1;
    } else {
        mode = 0;
    }

    setMode(modes[mode].w, modes[mode].h);

    lblue = lgreen = 0;
    return true;
}

bool OptionScreenSize::rightKey(){
    int mode = findMode(Configuration::getScreenWidth(), Configuration::getScreenHeight());
    if (mode >= 0 && mode < (int)modes.size() - 1){
        mode += 1;
    } else {
        mode = 0;
    }

    setMode(modes[mode].w, modes[mode].h);

    rblue = rgreen = 0;
    return true;
}

static string joinPaths(const vector<Filesystem::AbsolutePath> & strings, const string & middle){
    ostringstream out;

    for (vector<Filesystem::AbsolutePath>::const_iterator it = strings.begin(); it != strings.end(); it++){
        out << (*it).path() << middle;
    }

    return out.str();
}

static bool sortInfo(const Util::ReferenceCount<Menu::FontInfo> & info1, 
                     const Util::ReferenceCount<Menu::FontInfo> & info2){
    string name1 = Util::lowerCaseAll(info1->getName());
    string name2 = Util::lowerCaseAll(info2->getName());
    return name1 < name2;
}

static bool isWindows(){
#ifdef WINDOWS
    return true;
#else
    return false;
#endif
}

static bool isOSX(){
#ifdef MACOSX
    return true;
#else
    return false;
#endif
}

template <class X>
static vector<X> operator+(const vector<X> & v1, const vector<X> & v2){
    vector<X> out;
    for (typename vector<X>::const_iterator it = v1.begin(); it != v1.end(); it++){
        out.push_back(*it);
    }
    for (typename vector<X>::const_iterator it = v2.begin(); it != v2.end(); it++){
        out.push_back(*it);
    }
    return out;
}

static vector<Filesystem::AbsolutePath> findSystemFonts(){
    if (isWindows()){
        const char * windows = getenv("windir");
        if (windows != NULL){
            return Storage::instance().getFilesRecursive(Filesystem::AbsolutePath(string(windows) + "/fonts"), "*.ttf");
        }
        return vector<Filesystem::AbsolutePath>();
    } else if (isOSX()){
        return Storage::instance().getFilesRecursive(Filesystem::AbsolutePath("/Library/Fonts"), "*.ttf");
    } else {
        /* assume unix/linux conventions */
        return Storage::instance().getFilesRecursive(Filesystem::AbsolutePath("/usr/share/fonts/truetype"), "*.ttf") + 
               Storage::instance().getFilesRecursive(Filesystem::AbsolutePath("/usr/local/share/fonts/truetype"), "*.ttf");

    }
}

static vector<Util::ReferenceCount<Menu::FontInfo> > findFonts(){
    vector<Util::ReferenceCount<Menu::FontInfo> > fonts;
    try{
        Filesystem::AbsolutePath fontsDirectory = Storage::instance().find(Filesystem::RelativePath("fonts"));
        Global::debug(1, "fonts") << "Font directory " << fontsDirectory.path() << endl;
        vector<Filesystem::AbsolutePath> ttfFonts = Storage::instance().getFiles(fontsDirectory, "*.ttf");
        Global::debug(1, "fonts") << "Found ttf fonts " << joinPaths(ttfFonts, ", ") << endl;
        vector<Filesystem::AbsolutePath> otfFonts = Storage::instance().getFiles(fontsDirectory, "*.otf");
        Global::debug(1, "fonts") << "Found otf fonts " << joinPaths(otfFonts, ", ") << endl;

        for (vector<Filesystem::AbsolutePath>::iterator it = ttfFonts.begin(); it != ttfFonts.end(); it++){
            fonts.push_back(Util::ReferenceCount<Menu::FontInfo>(new Menu::RelativeFontInfo(Storage::instance().cleanse(*it), Configuration::getMenuFontWidth(), Configuration::getMenuFontHeight())));
        }

        for (vector<Filesystem::AbsolutePath>::iterator it = otfFonts.begin(); it != otfFonts.end(); it++){
            fonts.push_back(Util::ReferenceCount<Menu::FontInfo>(new Menu::RelativeFontInfo(Storage::instance().cleanse(*it), Configuration::getMenuFontWidth(), Configuration::getMenuFontHeight())));
        }
        
        /* linux specific fonts */
        vector<Filesystem::AbsolutePath> systemFonts = findSystemFonts();
        for (vector<Filesystem::AbsolutePath>::iterator it = systemFonts.begin(); it != systemFonts.end(); it++){
            Global::debug(1) << "Adding system font `" << (*it).path() << "'" << endl;
            fonts.push_back(Util::ReferenceCount<Menu::FontInfo>(new Menu::AbsoluteFontInfo(*it, Configuration::getMenuFontWidth(), Configuration::getMenuFontHeight())));
        }

        sort(fonts.begin(), fonts.end(), sortInfo);
        
        // DEFAULT (blank)
        // fonts.insert(fonts.begin(), new Menu::DefaultFontInfo());
        fonts.insert(fonts.begin(), Util::ReferenceCount<Menu::FontInfo>(NULL));
    } catch (const Filesystem::NotFound & e){
        throw LoadException(__FILE__, __LINE__, e, "Could not load font");
    }

    return fonts;
}

OptionSelectFont::OptionSelectFont(const Gui::ContextBox & parent, const Token *token):
MenuOption(parent, token),
typeAdjust(fontName),
lblue(255),
lgreen(255),
rblue(255),
rgreen(255){
    setRunnable(false);

    if ( *token != "font-select" ){
        throw LoadException(__FILE__, __LINE__, "Not a font selector");
    }

    TokenView view = token->view();
    while (view.hasMore()){
        try{
            const Token * tok;
            view >> tok;
            if ( *tok == "adjust" ){
                std::string temp;
                tok->view() >> temp;
                if ( temp == "name" ) typeAdjust = fontName;
                else if ( temp == "width" ) typeAdjust = fontWidth;
                else if ( temp == "height" ) typeAdjust = fontHeight;
                else throw LoadException(__FILE__, __LINE__, "Incorrect value \"" + temp + "\" in font-select");
            } else {
                Global::debug(3) << "Unhandled menu attribute: " << endl;
                if (Global::getDebug() >= 3){
                    tok->print(" ");
                }
            }
        } catch ( const TokenException & ex ) {
            throw LoadException(__FILE__, __LINE__, ex, "Menu parse error");
        } catch ( const LoadException & ex ) {
            // delete current;
            throw ex;
        }
    }
}

void OptionSelectFont::open(){
    // Find and set fonts now
    if (typeAdjust == fontName){
        fonts = findFonts();
    }
}
    
void OptionSelectFont::close(){
    if (typeAdjust == fontName){
        /* the user probably loaded a bunch of different fonts that will
         * never be used again, so clear the font cache
         * TODO: dont clear the currently selected font
         */
        FontFactory::clear();
    }
}

OptionSelectFont::~OptionSelectFont(){
    // Nothing
}

void OptionSelectFont::logic(){
    /* FIXME Get current font and display info */
    switch (typeAdjust){
        case fontName:{
	    std::string name;
            if (Configuration::hasMenuFont()){
                name = Configuration::getMenuFont()->getName();
            } else {
                name = "Default";
            }
            setText("Current Font: " + name);
            break;
	}
        case fontWidth:{
            ostringstream temp;
            temp << "Font Width: " << Configuration::getMenuFontWidth();
            setText(temp.str());
            break;
        }
        case fontHeight:{
            ostringstream temp;
            temp << "Font Height: " << Configuration::getMenuFontHeight();
            setText(temp.str());
            break;
        }
        default: break;
    }
    if (lblue < 255){
        lblue += 5;
    }

    if (rblue < 255){
        rblue += 5;
    }

    if (lgreen < 255){
        lgreen += 5;
    }

    if (rgreen < 255){
        rgreen += 5;
    }
}

void OptionSelectFont::run(const Menu::Context & context){
    // throw Menu::MenuException(__FILE__, __LINE__);
    /* throw something to quit back to the previous menu */
}

bool OptionSelectFont::leftKey(){
    switch (typeAdjust){
        case fontName:
            nextIndex(false);
            break;
        case fontWidth:
            Configuration::setMenuFontWidth(Configuration::getMenuFontWidth() - 1);
            break;
        case fontHeight:
            Configuration::setMenuFontHeight(Configuration::getMenuFontHeight() - 1);
            break;
        default:
            break;
    }
    lblue = lgreen = 0;
    return true;
}

bool OptionSelectFont::rightKey(){
    switch (typeAdjust){
        case fontName:
            nextIndex(true);
            break;
        case fontWidth:
            Configuration::setMenuFontWidth(Configuration::getMenuFontWidth() + 1);
            break;
        case fontHeight:
            Configuration::setMenuFontHeight(Configuration::getMenuFontHeight() + 1);
            break;
        default:
            break;
    }
    rblue = rgreen = 0;
    return true;
}

static bool saneFont(const Util::ReferenceCount<Menu::FontInfo> & info){
    class Context: public Loader::LoadingContext {
    public:
        Context(const Util::ReferenceCount<Menu::FontInfo> & info):
            info(info),
            isok(false){
            }

        bool ok(){
            try{
                const Font & font = info->get();
                return font.textLength("A") != 0 &&
                    font.getHeight() != 0;
            } catch (const Exception::Base & ignore){
                return true;
            }
        }

        virtual void load(){
            isok = ok();
        }

        const Util::ReferenceCount<Menu::FontInfo> & info;
        bool isok;
    };

    if (info == NULL){
        return true;
    }

    Context context(info);
    /* an empty Info object, we don't really care about it */
    Loader::Info level;
    Loader::loadScreen(context, level, Loader::SimpleCircle);
    return context.isok;
}

void OptionSelectFont::nextIndex(bool forward){
    if (fonts.size() == 0){
        return;
    }
    
    int index = 0;
    for (unsigned int i = 0 ; i < fonts.size() ; ++i){
        if ((Configuration::getMenuFont() == NULL && fonts[i] == NULL) ||
            ((Configuration::getMenuFont() != NULL && fonts[i] != NULL) &&
             (*Configuration::getMenuFont() == *fonts[i]))){
            index = i;
        }
    }

    if (forward){
	index++;
	if (index >= (int) fonts.size()){
	    index = 0;
	}
    } else {
	index--;
	if (index < 0){
	    index = (int)fonts.size()-1;
	}
    }

    while (!saneFont(fonts[index])){
        Global::debug(0) << "Warning: erasing font `" << fonts[index]->getName() << "'" << endl;
        int where = 0;
        vector<Util::ReferenceCount<Menu::FontInfo> >::iterator it;
        for (it = fonts.begin(); it != fonts.end() && where != index; it++, where++){
        }
        fonts.erase(it);
        if (index >= (int) fonts.size()){
            index = fonts.size() - 1;
        }
    }

    Configuration::setMenuFont(fonts[index]);

    /* FIXME */
    /*
    if (fonts[index] == "Default"){
	Configuration::setMenuFont("");
    } else {
	Configuration::setMenuFont(fonts[index]);
    }
    */
}

OptionSpeed::OptionSpeed(const Gui::ContextBox & parent, const Token *token):
MenuOption(parent, token),
name(""),
lblue(255),
lgreen(255),
rblue(255),
rgreen(255){
    setRunnable(false);

    if ( *token != "speed" )
        throw LoadException(__FILE__, __LINE__, "Not speed option");

    readName(token);
}

OptionSpeed::~OptionSpeed(){
	// Nothing
}


std::string OptionSpeed::getText() const {
    ostringstream out;
    out << MenuOption::getText() << ": " << Configuration::getGameSpeed();
    return out.str();
}

void OptionSpeed::logic(){
    /*
	//ostringstream temp;
	char temp[255];
	sprintf( temp, "%s: %0.2f", name.c_str(), MenuGlobals::getGameSpeed() );
	setText(std::string(temp));
        */
	
}

void OptionSpeed::run(const Menu::Context & context){
}

bool OptionSpeed::leftKey(){
    Configuration::setGameSpeed(Configuration::getGameSpeed() - 0.05);
    if (Configuration::getGameSpeed() < 0.1){
        Configuration::setGameSpeed(0.1);
    }
    return false;
}
bool OptionSpeed::rightKey(){
    Configuration::setGameSpeed(Configuration::getGameSpeed() + 0.05);
    rblue = rgreen = 0;
    return false;
}

OptionTabMenu::OptionTabMenu(const Gui::ContextBox & parent, const Token *token, const Menu::OptionFactory & factory):
MenuOption(parent, token),
menu(0){
    if (token->numTokens() == 1){
        std::string temp;
        token->view() >> temp;
        menu = new Menu::Menu(Storage::instance().find(Filesystem::RelativePath(temp)), factory, Menu::Menu::Tabbed);
    } else {
        menu = new Menu::Menu(token, factory, Menu::Menu::Tabbed);
    }

    // this->setText(menu->getName());
    // token->print("Menu: ");
    const Token * tok = token->findToken("_/name");
    if (tok != NULL){
        std::string name;
        tok->view() >> name;
        // Global::debug(0, "menu") << "Menu name: " << name << endl;
        this->setText(name);
    } else {
        // No name?
        throw LoadException(__FILE__, __LINE__, "Menu has no name");
    }
}

OptionTabMenu::~OptionTabMenu(){
    // Delete our menu
    if (menu){
        delete menu;
    }
}

void OptionTabMenu::logic(){
	// Nothing
}

void OptionTabMenu::run(const Menu::Context & context){
    // Do our new menu
    // menu->run(context);
    try{
        menu->run(context);
    } catch (const Exception::Return ignore){
        throw Menu::Reload(__FILE__, __LINE__);
    }
}

OptionSound::OptionSound(const Gui::ContextBox & parent, const Token *token):
MenuOption(parent, token),
lblue(255),
lgreen(255),
rblue(255),
rgreen(255){
    setRunnable(false);

    if (*token != "sound" ){
        throw LoadException(__FILE__, __LINE__, "Not a sound option");
    }

    readName(token);
    originalName = getName();
}

OptionSound::~OptionSound(){
}

void OptionSound::logic(){
    ostringstream temp;
    temp << originalName << ": " << Configuration::getSoundVolume();
    setText(temp.str());
}

void OptionSound::run(const Menu::Context & context){
}

void OptionSound::changeSound(int much){
    int volume = Configuration::getSoundVolume();
    volume += much;
    if (volume < 0){
        volume = 0;
    }

    if (volume > 100){
        volume = 100;
    }

    Configuration::setSoundVolume(volume);
}

bool OptionSound::leftKey(){
    changeSound(-1);
    
    lblue = lgreen = 0;
    return true;
}

bool OptionSound::rightKey(){
    changeSound(+1);

    rblue = rgreen = 0;
    return true;
}

OptionMusic::OptionMusic(const Gui::ContextBox & parent, const Token *token):
MenuOption(parent, token),
lblue(255),
lgreen(255),
rblue(255),
rgreen(255){
    setRunnable(false);

    if (*token != "music" ){
        throw LoadException(__FILE__, __LINE__, "Not a music option");
    }

    readName(token);
    originalName = getName();
}

void OptionMusic::logic(){
    ostringstream temp;
    temp << originalName << ": " << Configuration::getMusicVolume();
    setText(temp.str());
}

void OptionMusic::run(const Menu::Context & context){
}

void OptionMusic::changeMusic(int much){
    int volume = Configuration::getMusicVolume();
    volume += much;
    if (volume < 0){
        volume = 0;
    }

    if (volume > 100){
        volume = 100;
    }

    Configuration::setMusicVolume(volume);
    Music::setVolume((double) volume / 100.0);
}

bool OptionMusic::leftKey(){
    changeMusic(-1);
    
    lblue = lgreen = 0;
    return true;
}

bool OptionMusic::rightKey(){
    changeMusic(+1);
    
    lblue = lgreen = 0;
    return true;
}

OptionMusic::~OptionMusic(){
}

OptionLanguage::OptionLanguage(const Gui::ContextBox & parent, const Token * token):
MenuOption(parent, token){
    readName(token);
#if 0
    const Token * start = token->getRootParent();
    vector<const Token*> tokens = start->findTokens("*/language");
    vector<string> all;
    for (vector<const Token*>::iterator it = tokens.begin(); it != tokens.end(); it++){
        string language;
        const Token * token = *it;
        if (token->match("language", language)){
            all.push_back(language);
        }
    }
    sort(all.begin(), all.end());
    unique_copy(all.begin(), all.end(), back_insert_iterator<vector<string> >(languages));
    // Global::debug(0) << "Found " << languages.size() << " languages" << endl;
#endif
}

void OptionLanguage::run(const Menu::Context & context){
    class LanguageOption: public MenuOption {
    public:
        LanguageOption(const Gui::ContextBox & parent, const string & language):
        MenuOption(parent, NULL){
            setText(language);
            setInfoText(language);
        }

        virtual void logic(){
        }

        virtual void run(const ::Menu::Context & context){
            Configuration::setLanguage(getText());
            Configuration::saveConfiguration();
            throw ::Menu::MenuException(__FILE__, __LINE__);
        }
    };

    Menu::Menu temp;
    /* FIXME don't hardcode arial.ttf */
    Util::ReferenceCount<Menu::FontInfo> info(new Menu::RelativeFontInfo(Filesystem::RelativePath("fonts/arial.ttf"), 24, 24));
    temp.setFont(info);

    const Gui::ContextBox & box = ((Menu::DefaultRenderer*) temp.getRenderer())->getBox();

    vector<string> languages = context.getLanguages();
    for (vector<string>::iterator it = languages.begin(); it != languages.end(); it++){
        temp.addOption(new LanguageOption(box, *it));
    }

    try {
        temp.run(context);
    } catch (const Exception::Return & ignore){
    } catch (const Menu::MenuException & ex){
    }

    throw Menu::Reload(__FILE__, __LINE__);
    // throw Exception::Return(__FILE__, __LINE__);
}
    
void OptionLanguage::logic(){
}

OptionGibs::OptionGibs(const Gui::ContextBox & parent, const Token *token):
MenuOption(parent, token){
    setRunnable(false);

    if (*token != "gibs" ){
        throw LoadException(__FILE__, __LINE__, "Not a gibs option");
    }

    readName(token);
    originalName = getName();
}

void OptionGibs::logic(){
    ostringstream temp;
    /* FIXME: we want to use Gib::GibProperty here but that would necessitate a
     * dependancy on the Paintown engine.
     */
    temp << originalName << ": " << Configuration::getProperty("paintown/gibs", 5);
    setText(temp.str());
}

void OptionGibs::run(const Menu::Context & context){
}

void OptionGibs::changeGibs(int much){
    int gibs = Configuration::getProperty("paintown/gibs", 5);
    gibs += much;
    if (gibs < 0){
        gibs = 0;
    }

    if (gibs > 10){
        gibs = 10;
    }

    Configuration::setProperty("paintown/gibs", gibs);
}

bool OptionGibs::leftKey(){
    changeGibs(-1);
    return true;
}

bool OptionGibs::rightKey(){
    changeGibs(+1);
    return true;
}

OptionGibs::~OptionGibs(){
}
