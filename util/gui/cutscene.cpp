#include "cutscene.h"

#include "util/bitmap.h"
#include "animation.h"
#include "util/init.h"
#include "util/input/input-map.h"
#include "util/input/input-manager.h"
#include "util/input/input-source.h"
#include "util/stretch-bitmap.h"
#include "util/token.h"

using namespace Gui;

Scene::Scene(const Token * token):
ticks(0),
endTicks(0){
    if ( *token != "scene" ){
        throw LoadException(__FILE__, __LINE__, "Not a Scene");
    }
    TokenView view = token->view();
    while (view.hasMore()){
        try{
            const Token * tok;
            view >> tok;
            if ( *tok == "time" ){
                tok->view() >> endTicks;
            } else if ( *tok == "animation" || *tok == "anim" ){
                setAnimation(Util::ReferenceCount<Gui::Animation>(new Gui::Animation(tok)));
            } else if ( *tok == "fade" ){
                fader.parseDefaults(tok);
            } else {
                Global::debug(3) << "Unhandled Scene attribute: " << std::endl;
                if (Global::getDebug() >= 3){
                    tok->print(" ");
                }
            }
        } catch ( const TokenException & ex ) {
            throw LoadException(__FILE__, __LINE__, ex, "Scene parse error");
        } catch ( const LoadException & ex ) {
            throw ex;
        }
    }
}
Scene::~Scene(){
}
void Scene::act(){
    if (backgroundBottom != NULL){
        backgroundBottom->act();
    }
    if (backgroundMiddle != NULL){
        backgroundMiddle->act();
    }
    if (backgroundTop != NULL){
        backgroundTop->act();
    }
    if (foregroundBottom != NULL){
        backgroundBottom->act();
    }
    if (foregroundMiddle != NULL){
        foregroundMiddle->act();
    }
    if (foregroundTop != NULL){
        foregroundTop->act();
    }
    fader.act();
    // Increment ticks
    ticks++;
    // Set fadeout when near the end
    if ((endTicks - ticks) == fader.getFadeOutTime()){
        fader.setState(Gui::FadeTool::FadeOut);
    }
}
void Scene::render(const Graphics::Bitmap & work){
    
    if (backgroundBottom != NULL){
        backgroundBottom->draw(work);
    }
    if (backgroundMiddle != NULL){
        backgroundMiddle->draw(work);
    }
    if (backgroundTop != NULL){
        backgroundTop->draw(work);
    }
    if (foregroundBottom != NULL){
        backgroundBottom->draw(work);
    }
    if (foregroundMiddle != NULL){
        foregroundMiddle->draw(work);
    }
    if (foregroundTop != NULL){
        foregroundTop->draw(work);
    }
    fader.draw(work);
}

void Scene::setAnimation(Util::ReferenceCount<Gui::Animation> animation){
    switch (animation->getDepth()){
        case Gui::Animation::BackgroundBottom:
            backgroundBottom = animation;
            break;
        case Gui::Animation::BackgroundMiddle:
            backgroundMiddle = animation;
            break;
        case Gui::Animation::BackgroundTop:
            backgroundTop = animation;
            break;
        case Gui::Animation::ForegroundBottom:
            foregroundBottom = animation;
            break;
        case Gui::Animation::ForegroundMiddle:
            foregroundMiddle = animation;
            break;
        case Gui::Animation::ForegroundTop:
            foregroundTop = animation;
            break;
        default:
            break;
    }
}

CutScene::CutScene():
width(0),
height(0),
current(0){
}

CutScene::CutScene(const Token * token):
width(0),
height(0),
current(0){
    if ( *token != "cutscene" ){
        throw LoadException(__FILE__, __LINE__, "Not a CutScene");
    }
    TokenView view = token->view();
    while (view.hasMore()){
        try{
            const Token * tok;
            view >> tok;
            if ( *tok == "name"){
                tok->view() >> name;
            } else if ( *tok == "scene" ){
                scenes.push_back(Util::ReferenceCount<Scene>(new Scene(tok)));
            } else {
                Global::debug(3) << "Unhandled Cutscene attribute: " << std::endl;
                if (Global::getDebug() >= 3){
                    tok->print(" ");
                }
            }
        } catch ( const TokenException & ex ) {
            throw LoadException(__FILE__, __LINE__, ex, "Cutscene parse error");
        } catch ( const LoadException & ex ) {
            throw ex;
        }
    }
}

CutScene::~CutScene(){
}

void CutScene::setName(const std::string & n){
    name = n;
}
 
void CutScene::setResolution(int w, int h){
    width = w;
    height = h;
}
void CutScene::setScene(unsigned int scene){
    if (scene >= scenes.size()){
        current = scenes.size()-1;
    } else {
        current = scene;
    }
}

enum Keys{
    Esc,
};
void CutScene::next(){
    
    class Logic: public Util::Logic {
        public:
            Logic(InputMap<Keys> & input, Util::ReferenceCount<Scene> scene):
            is_done(false),
            input(input),
            scene(scene){
            }

            bool is_done;
            InputMap<Keys> & input;
            Util::ReferenceCount<Scene> scene;
        
            bool done(){
                return is_done;
            }

        void run(){
            std::vector<InputMap<Keys>::InputEvent> out = InputManager::getEvents(input, InputSource());
            for (std::vector<InputMap<Keys>::InputEvent>::iterator it = out.begin(); it != out.end(); it++){
                const InputMap<Keys>::InputEvent & event = *it;
                if (event.enabled){
                    if (event.out == Esc){
                        is_done = true;
                    }
                }
            }
            
            scene->act();
            
            if (scene->done()){
                is_done = true;
            }
        }

        double ticks(double system){
            return system * Global::LOGIC_MULTIPLIER;
        }
    };

    class Draw: public Util::Draw {
        public:
            Draw(const Logic & logic, Util::ReferenceCount<Scene> scene, int width, int height):
            logic(logic),
            scene(scene),
            width(width),
            height(height){
            }
            
            const Logic & logic;
            Util::ReferenceCount<Scene> scene;
            int width, height;

            void draw(const Graphics::Bitmap & buffer){
                
                Graphics::StretchedBitmap work(width, height, buffer);
                work.start();
                scene->render(work);
                work.finish();
                buffer.BlitToScreen();
            }
    };
    
    InputMap<Keys> input;
    input.set(Keyboard::Key_ESC, 0, true, Esc);
    
    Logic logic(input, scenes[current]);
    Draw draw(logic, scenes[current], width, height);

    Util::standardLoop(logic, draw);
    
    current++;
}
bool CutScene::hasMore(){
    return (current < scenes.size());
}
