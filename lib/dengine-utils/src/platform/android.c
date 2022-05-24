#include "dengine-utils/platform/android.h"
#include "dengine-utils/logging.h" //log

#include <android/asset_manager.h>
#include <android/window.h>

#include <stdlib.h> //malloc
#include <string.h> //memset

struct android_app* _app;

DengineAndroidAppFunc initfunc = NULL;
DengineAndroidAppFunc termfunc = NULL;

int iswindowrunning = 0;
AndroidInput input;

void _dengineutils_android_terminate(struct android_app* app);

int dengineutils_android_asset2file2mem(File2Mem* f2m)
{
    AAssetManager* asset_mgr = _app->activity->assetManager;
    if(!asset_mgr)
        return 0;

    AAsset* asset = AAssetManager_open(asset_mgr, f2m->file, AASSET_MODE_BUFFER);
    if(!asset)
        return 0;

    f2m->size = AAsset_getLength(asset);
    /* +1 = \0 */
    void* mem = malloc(f2m->size + 1);
    memset(mem, 0, f2m->size + 1);
    int rd = AAsset_read(asset, mem, f2m->size);

    f2m->mem = mem;
    AAsset_close(asset);
    return rd;
}

int dengineutils_android_pollevents()
{
    int events;
    struct android_poll_source* source;
    while((ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0)
    {
        if(source != NULL)
        {
            source->process(_app, source);
        }
    }
    return events;
}

static int32_t input_event(struct android_app* app, AInputEvent* event)
{
    uint32_t type = AInputEvent_getType(event);
    switch (type) {
        case AINPUT_EVENT_TYPE_MOTION:
        {
            int32_t action = AMotionEvent_getAction(event);
            int32_t id = AMotionEvent_getPointerId(event, 0);

            //-1 state for get_mouse_once
            if(input.pointer0.state != -1)
            {
                input.pointer0.x = AMotionEvent_getX(event, 0);
                input.pointer0.y = AMotionEvent_getY(event, 0);
            }

            if(action == AMOTION_EVENT_ACTION_POINTER_UP )
            {
                input.pointer0.x = 0.0;
                input.pointer0.y = 0.0;
                input.pointer0.state = 0;
                dengineutils_logging_log("rel");
            }
            dengineutils_logging_log("%f %f %d", input.pointer0.x, input.pointer0.y, id);

            break;
        }

        default:
            break;
    }
    return 0;
}

static void cmd_handle(struct android_app* app, int32_t cmd)
{
    //dengineutils_logging_log("cmd %u", cmd);
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            dengineutils_logging_log("Saving state...");
            break;

        case APP_CMD_INIT_WINDOW:
            dengineutils_logging_log("Getting window ready...");
            iswindowrunning = 1;
            ANativeWindow_acquire(_app->window);
            if(initfunc)
                initfunc(app);
            //Buggy fullscreen
            //ANativeActivity_setWindowFlags(app->activity, AWINDOW_FLAG_FULLSCREEN, 0);
            break;

        case APP_CMD_TERM_WINDOW:
            dengineutils_logging_log("Term window");
            if(termfunc)
                termfunc(app);
            ANativeWindow_release(app->window);
            ANativeActivity_finish(app->activity);
            iswindowrunning = 0;
            break;

        case APP_CMD_GAINED_FOCUS:
            dengineutils_logging_log("Gained focus");
            break;

        case APP_CMD_LOST_FOCUS:
            dengineutils_logging_log("Lost focus");
            break;

        case APP_CMD_PAUSE:
            dengineutils_logging_log("Paused");
            break;

        case APP_CMD_RESUME:
            dengineutils_logging_log("Resumed");
            break;

        case APP_CMD_DESTROY:
            dengineutils_logging_log("Destroy");
            break;
    }
}

void dengineutils_android_set_initfunc(DengineAndroidAppFunc func)
{
    initfunc = func;
}

void dengineutils_android_set_terminatefunc(DengineAndroidAppFunc func)
{
    termfunc = func;
}

void dengineutils_android_set_app(struct android_app* app)
{
    _app = app;
    _app->onAppCmd = cmd_handle;
    _app->onInputEvent = input_event;
}

void dengineutils_android_set_filesdir()
{
    JNIEnv* env;
    JavaVM* vm = _app->activity->vm;
    jint attached = (*vm)->AttachCurrentThread(vm, &env, NULL);
    if(attached < 0)
    {
        dengineutils_logging_log("ERROR::FAILED TO ATTACH VM");
        return;
    }
    jclass activity = (*env)->FindClass(env, "android/app/NativeActivity");
    if(!activity)
        goto detach;

    jmethodID getFilesDir = (*env)->GetMethodID(env, activity, "getFilesDir", "()Ljava/io/File;");
    jobject files_dir = (*env)->CallObjectMethod(env, _app->activity->clazz, getFilesDir);

    jclass  fileClass = (*env)->FindClass(env, "java/io/File");

    jmethodID  getPath = (*env)->GetMethodID(env, fileClass, "getPath", "()Ljava/lang/String;");
    jstring file_string = (*env)->CallObjectMethod( env,files_dir, getPath );

    const char* file_chars = (*env)->GetStringUTFChars( env, file_string, NULL );
    dengineutils_filesys_set_filesdir(file_chars);

    detach:

    if((*env)->ExceptionOccurred(env))
    {
        (*env)->ExceptionDescribe(env);
    }
    (*vm)->DetachCurrentThread(vm);
}

void dengineutils_android_set_cachedir()
{
    JNIEnv* env;
    JavaVM* vm = _app->activity->vm;
    jint attached = (*vm)->AttachCurrentThread(vm, &env, NULL);
    if(attached < 0)
    {
        dengineutils_logging_log("ERROR::FAILED TO ATTACH VM");
        return;
    }

    jclass activity = (*env)->FindClass(env, "android/app/NativeActivity");
    if(!activity)
        goto detach;

    jmethodID getCacheDir = (*env)->GetMethodID(env, activity, "getCacheDir", "()Ljava/io/File;");
    jobject cache_dir = (*env)->CallObjectMethod(env, _app->activity->clazz, getCacheDir);

    jclass  fileClass = (*env)->FindClass(env, "java/io/File");

    jmethodID  getPath = (*env)->GetMethodID(env, fileClass, "getPath", "()Ljava/lang/String;");
    jstring cache_string = (*env)->CallObjectMethod(env, cache_dir, getPath );

    const char* file_chars = (*env)->GetStringUTFChars(env, cache_string, NULL );
    dengineutils_filesys_set_cachedir(file_chars);
    detach:

    if((*env)->ExceptionOccurred(env))
    {
        (*env)->ExceptionDescribe(env);
    }
    (*vm)->DetachCurrentThread(vm);
}

ANativeWindow* dengineutils_android_get_window()
{
    return _app->window;
}

AAssetManager* dengineutils_android_get_assetmgr()
{
    return _app->activity->assetManager;
}

int dengineutils_android_iswindowrunning()
{
    return iswindowrunning;
}

AndroidInput* dengineutils_android_get_input()
{
    return &input;
}
