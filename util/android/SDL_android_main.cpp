
/* Include the SDL main definition header */
#include "SDL_main.h"

/*******************************************************************************
                 Functions called by JNI
*******************************************************************************/
#include <jni.h>

#include "util/funcs.h"

// Called before SDL_main() to initialize JNI bindings in SDL library
extern "C" void SDL_Android_Init(JNIEnv* env, jclass cls);

// Library init
extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    return JNI_VERSION_1_4;
}

// Start up the SDL app
extern "C" void Java_org_libsdl_app_SDLActivity_nativeInit(JNIEnv* env, jclass cls, jobject obj)
{
    /* This interface could expand with ABI negotiation, calbacks, etc. */
    SDL_Android_Init(env, cls);

    /* Run the application code! */
    int status;
    char *argv[2];
    argv[0] = strdup("SDL_app");
    argv[1] = NULL;
    status = SDL_main(1, argv);

    /* We exit here for consistency with other platforms. */
    exit(status);
}

extern "C" void Java_org_libsdl_app_SDLActivity_setExternalLocation(JNIEnv* env, jclass cls, jstring jpath)
{
    jboolean iscopy;
    const char *path = env->GetStringUTFChars(jpath, &iscopy);
    const std::string & externalLocation = std::string(path);
    // Set data path in paintown
    Util::setDataPath(externalLocation + "/paintown/data/");
    env->ReleaseStringUTFChars(jpath, path);
}


/* vi: set ts=4 sw=4 expandtab: */
