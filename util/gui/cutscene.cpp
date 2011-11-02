#include "cutscene.h"

#include "util/bitmap.h"
#include "util/init.h"
#include "util/input/input-map.h"
#include "util/input/input-manager.h"
#include "util/input/input-source.h"
#include "util/stretch-bitmap.h"
#include "util/token.h"
#include "util/tokenreader.h"

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
    backgrounds.act();
    fader.act();
    // Increment ticks
    ticks++;
    // Set fadeout when near the end
    if ((endTicks - ticks) == fader.getFadeOutTime()){
        fader.setState(Gui::FadeTool::FadeOut);
    }
}
void Scene::render(const Graphics::Bitmap & work){
    // Backgrounds
    backgrounds.render(Gui::Animation::BackgroundBottom, work);
    backgrounds.render(Gui::Animation::BackgroundMiddle, work);
    backgrounds.render(Gui::Animation::BackgroundTop, work);
    
    // Foregrounds
    backgrounds.render(Gui::Animation::ForegroundBottom, work);
    backgrounds.render(Gui::Animation::ForegroundMiddle, work);
    backgrounds.render(Gui::Animation::ForegroundTop, work);
    
    fader.draw(work);
}

void Scene::setAnimation(Util::ReferenceCount<Gui::Animation> animation){
    backgrounds.add(animation);
}

CutScene::CutScene():
width(0),
height(0),
current(0){
}
    
CutScene::CutScene(const Filesystem::AbsolutePath & path):
width(0),
height(0),
current(0){
    TokenReader reader;
    load(reader.readTokenFromFile(path.path().c_str()));
}

CutScene::CutScene(const Token * token):
width(0),
height(0),
current(0){
    load(token);
}

CutScene::~CutScene(){
}

void CutScene::load(const Token * token){
    if (*token != "cutscene"){
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

void CutScene::playAll(){
    for (int i = 0; i < scenes.size(); i++){
        playScene(i);
    }
}

void CutScene::playScene(int scene){
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

    if (scene >= 0 && scene < scenes.size()){
        InputMap<Keys> input;
        input.set(Keyboard::Key_ESC, 0, true, Esc);
        input.set(Joystick::Quit, 0, true, Esc);

        Logic logic(input, scenes[scene]);
        Draw draw(logic, scenes[scene], width, height);

        Util::standardLoop(logic, draw);
    }
}

void CutScene::playScene(){
    playScene(current);
}
    
void CutScene::next(){
    if (current < scenes.size()){
        current += 1;
    }
}

bool CutScene::hasMore(){
    return (current < scenes.size());
}
