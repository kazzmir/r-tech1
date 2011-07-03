#ifdef NACL

/* talks to chrome via the nacl stuff */

#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/var.h>
#include <ppapi/cpp/dev/scriptable_object_deprecated.h>
#include <SDL/SDL_nacl.h>
#include <string>
#include "../../main.h"
#include "../network/network-system.h"

namespace nacl{
    class PaintownScript: public pp::deprecated::ScriptableObject {
    public:
        virtual bool HasMethod(const pp::Var& method, pp::Var* exception);
        virtual pp::Var Call(const pp::Var& method, const std::vector<pp::Var>& args,pp::Var* exception);
    };

    class PaintownInstance : public pp::Instance {
    public:
        explicit PaintownInstance(PP_Instance instance) : pp::Instance(instance) {
            the_instance = this;
        }
        virtual ~PaintownInstance() {}
        static PaintownInstance * the_instance;

        /* set up the viewport and run the game as usual */
        void run(){
            SDL_NACL_SetInstance(pp_instance(), 640, 480);
            int argc = 1;
            char * argv[1] = {"paintown"};
            Storage::setInstance(new Storage::NetworkSystem());
            paintown_main(argc, argv);
        }

        virtual pp::Var GetInstanceObject(){
            PaintownScript * object = new PaintownScript();
            return pp::Var(this, object);
        }

        /* when the browser does paintownModule.postMessage('run') this will
         * get executed.
         */
        virtual void HandleMessage(const pp::Var& var_message){
            if (!var_message.is_string()){
                return;
            }

            std::string message = var_message.AsString();
            if (message == "run"){
                run();
            }
        }
    };
    PaintownInstance * PaintownInstance::the_instance;

    bool PaintownScript::HasMethod(const pp::Var& method, pp::Var* exception){
        if (!method.is_string()){
            return false;
        }

        std::string name = method.AsString();
        if (name == "run"){
            return true;
        }
        return false;
    }

    pp::Var PaintownScript::Call(const pp::Var& method, const std::vector<pp::Var>& args,pp::Var* exception){
        if (!method.is_string()){
            return false;
        }

        std::string name = method.AsString();
        if (name == "run"){
            PaintownInstance::the_instance->run();
        }

        return pp::Var();
    }

    class PaintownModule : public pp::Module {
    public:
        PaintownModule() : pp::Module() {}
        virtual ~PaintownModule() {}

        virtual pp::Instance* CreateInstance(PP_Instance instance) {
            return new PaintownInstance(instance);
        }
    };
}

namespace pp{
    Module * CreateModule(){
        return new nacl::PaintownModule();
    }
}

#endif
