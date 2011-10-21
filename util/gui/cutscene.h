#ifndef util_gui_cutscene_h
#define util_gui_cutscene_h

#include "fadetool.h"
#include "util/pointer.h"
#include "animation.h"

#include <string>
#include <vector>

class Token;
namespace Graphics{
class Bitmap;
}

namespace Gui{

/*! Cut scenes or story boards */
class Scene{
    public:
        Scene(const Token *);
        virtual ~Scene();
        
        virtual void act();
        virtual void render(const Graphics::Bitmap &);
        
        virtual void setAnimation(Util::ReferenceCount<Gui::Animation>);
        
        virtual inline void setEnd(int ticks){
            this->endTicks = ticks;
        }
        virtual inline bool done() const {
            return (this->ticks >= this->endTicks);
        }
        
    protected:
        int ticks;
        int endTicks;
        
        Gui::AnimationManager backgrounds;
        
        Gui::FadeTool fader;
};
    
class CutScene{
    public:
        CutScene();
        CutScene(const Token *);
        virtual ~CutScene();
        
        virtual void setName(const std::string &);
        virtual inline const std::string & getName() const {
            return this->name;
        }
        virtual void setResolution(int w, int h);
        virtual void setScene(unsigned int scene);
        virtual inline int getScene() const{
            return this->current;
        }
        virtual void next();
        virtual bool hasMore();
        
    protected:
        std::string name;
        int width;
        int height;
        std::vector< Util::ReferenceCount<Scene> > scenes;
        unsigned int current;
};
}
#endif