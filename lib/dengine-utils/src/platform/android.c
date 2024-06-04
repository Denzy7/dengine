#include "dengine-utils/platform/android.h"
#include "dengine-utils/logging.h" //log

#include <android/asset_manager.h>
#include <android/window.h>

#include <stdlib.h> //malloc
#include <string.h> //memset

struct android_app* _app;

int iswindowrunning = 0;
AndroidInput input;
int isactivityfocused = 0;
int handlebackbutton = 0;

/* TODO: we'd want a vtor of callbacks*/
DengineAndroidAppFunc onbackfunc = NULL;
DengineAndroidAppFunc appfuncs[DENGINEUTILS_ANDROID_APPFUNC_COUNT];

void _dengineutils_android_terminate(struct android_app* app);
int dengineutils_android_set_immersivemode();

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

int _dengineutils_android_poll(int timeout)
{
    int events;
    struct android_poll_source* source;
    while((ALooper_pollAll(timeout, NULL, &events, (void**)&source)) >= 0)
    {
        source->process(_app, source);
    }
    return events;
}

int dengineutils_android_pollevents()
{
    return _dengineutils_android_poll(0);
}

int dengineutils_android_waitevents()
{
    return _dengineutils_android_poll(-1);
}

static int32_t input_event(struct android_app* app, AInputEvent* event)
{
    uint32_t type = AInputEvent_getType(event);
    int32_t keycode = AKeyEvent_getKeyCode(event);
    int32_t action = AMotionEvent_getAction(event);
    int state = action & AMOTION_EVENT_ACTION_MASK;

    switch (type) {
        case AINPUT_EVENT_TYPE_MOTION:
        {
            /* TODO: use ptrid
            uint32_t ptrid = action >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
            */
            input.pointer_count = 0;
            /*memset(input.pointers, 0, sizeof(input.pointers));*/
            size_t pointers = AMotionEvent_getPointerCount(event);
            for(size_t i = 0; i < pointers; i++)
            {
                input.pointers[i].x = AMotionEvent_getX(event, i);
                input.pointers[i].y = AMotionEvent_getY(event, i);
                if(state ==  AMOTION_EVENT_ACTION_MOVE ||
                        state == AMOTION_EVENT_ACTION_POINTER_DOWN ||
                        state == AMOTION_EVENT_ACTION_DOWN
                  )
                    input.pointers[i].state = 1;
                else
                    input.pointers[i].state = 0;
            }
            input.pointer_count = pointers;
            memcpy(&input.pointer0, &input.pointers[0], sizeof(AndroidPointer));

/*            for(size_t i = 0; i < 10; i++)*/
            /*{*/
                /*dengineutils_logging_log("%d = s:%d x:%f y:%f", i, input.pointers[i].state, input.pointers[i].x, input.pointers[i].y);*/
            /*}*/

            break;
        }


        default:
            break;
    }

    /* ありがとうございます！
     * https://stackoverflow.com/questions/12130618/android-ndk-how-to-override-onbackpressed-in-nativeactivity-without-java
     */
    if(keycode == AKEYCODE_BACK && action == AKEY_EVENT_ACTION_UP){
        if(handlebackbutton){
            if(onbackfunc)
                onbackfunc(app);
            return 1;
        }
    }
    return 0;
}

void callappfunc_secure(struct android_app* app, DengineAndroidAppFuncType type)
{
    if(appfuncs[type])
        appfuncs[type](app);
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
            ANativeWindow_acquire(_app->window);
            //Buggy fullscreen
            //ANativeActivity_setWindowFlags(app->activity, AWINDOW_FLAG_FULLSCREEN, 0);
            callappfunc_secure(app, DENGINEUTILS_ANDROID_APPFUNC_INITWINDOW);
            break;

        case APP_CMD_TERM_WINDOW:
            dengineutils_logging_log("Term window");
            ANativeWindow_release(app->window);
            callappfunc_secure(app, DENGINEUTILS_ANDROID_APPFUNC_TERMINATE);
            break;

        case APP_CMD_GAINED_FOCUS:
            isactivityfocused = 1;
            /* gained focus is last to be called so our window should be ready
             * for drawing
             */
            iswindowrunning = 1;
            dengineutils_logging_log("Gained focus");
            break;

        case APP_CMD_LOST_FOCUS:
            isactivityfocused = 0;
            dengineutils_logging_log("Lost focus");
            break;

        case APP_CMD_PAUSE:

            dengineutils_logging_log("Paused");
            callappfunc_secure(app, DENGINEUTILS_ANDROID_APPFUNC_PAUSE);
            break;

        case APP_CMD_RESUME:
            dengineutils_logging_log("Resumed");
            callappfunc_secure(app, DENGINEUTILS_ANDROID_APPFUNC_RESUME);
            break;

        case APP_CMD_DESTROY:
            /* art swaps our window even after terminate_Window. we only 
             * truly exit once we destroy. we need this so we can receive
             * on resume and block android_main until the surface is recreated
             *
             * we need this here so its immediately intercepted and stop rendering 
             * to a null surface. pause is first called once user switches app or goes home
             */
            iswindowrunning = 0;
            dengineutils_logging_log("Destroy");
            break;
    }
}

void dengineutils_android_set_appfunc(DengineAndroidAppFunc func, DengineAndroidAppFuncType type)
{
    appfuncs[type] = func;
}

void dengineutils_android_set_initfunc(DengineAndroidAppFunc func)
{
    dengineutils_android_set_appfunc(func, DENGINEUTILS_ANDROID_APPFUNC_INITWINDOW);
}

void dengineutils_android_set_terminatefunc(DengineAndroidAppFunc func)
{
    dengineutils_android_set_appfunc(func, DENGINEUTILS_ANDROID_APPFUNC_TERMINATE);
}

struct android_app* dengineutils_android_get_app()
{
    return _app;
}

void dengineutils_android_set_app(struct android_app* app)
{
    _app = app;
    _app->onAppCmd = cmd_handle;
    _app->onInputEvent = input_event;
    memset(appfuncs, 0, sizeof(appfuncs));
    dengineutils_android_set_immersivemode();
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

int dengineutils_android_set_immersivemode(){
    int success = 1;
    JNIEnv* env;
    JavaVM* vm = _app->activity->vm;
    jint attached = (*vm)->AttachCurrentThread(vm, &env, NULL);
    if(attached < 0)
    {
        dengineutils_logging_log("ERROR::FAILED TO ATTACH VM");
        return 0;
    }

    jclass activityClass = (*env)->FindClass(env, "android/app/NativeActivity");
    jclass windowClass = (*env)->FindClass(env, "android/view/Window");
    jclass viewClass = (*env)->FindClass(env, "android/view/View");
    jmethodID getWindow = (*env)->GetMethodID(env, activityClass, "getWindow", "()Landroid/view/Window;");
    jmethodID getDecorView = (*env)->GetMethodID(env, windowClass, "getDecorView", "()Landroid/view/View;");
    jmethodID setSystemUiVisibility = (*env)->GetMethodID(env, viewClass, "setSystemUiVisibility", "(I)V");

    jobject windowObj = (*env)->CallObjectMethod(env, _app->activity->clazz, getWindow);
    jobject decorViewObj = (*env)->CallObjectMethod(env, windowObj, getDecorView);

    // Get flag ids
    jfieldID id_SYSTEM_UI_FLAG_LAYOUT_STABLE = (*env)->GetStaticFieldID(env, viewClass, "SYSTEM_UI_FLAG_LAYOUT_STABLE", "I");
    jfieldID id_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION = (*env)->GetStaticFieldID(env, viewClass, "SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION", "I");
    jfieldID id_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN = (*env)->GetStaticFieldID(env, viewClass, "SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN", "I");
    jfieldID id_SYSTEM_UI_FLAG_HIDE_NAVIGATION = (*env)->GetStaticFieldID(env, viewClass, "SYSTEM_UI_FLAG_HIDE_NAVIGATION", "I");
    jfieldID id_SYSTEM_UI_FLAG_FULLSCREEN = (*env)->GetStaticFieldID(env, viewClass, "SYSTEM_UI_FLAG_FULLSCREEN", "I");
    jfieldID id_SYSTEM_UI_FLAG_IMMERSIVE_STICKY = (*env)->GetStaticFieldID(env, viewClass, "SYSTEM_UI_FLAG_IMMERSIVE_STICKY", "I");

    // Get flags
    const int flag_SYSTEM_UI_FLAG_LAYOUT_STABLE = (*env)->GetStaticIntField(env, viewClass, id_SYSTEM_UI_FLAG_LAYOUT_STABLE);
    const int flag_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION = (*env)->GetStaticIntField(env, viewClass, id_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION);
    const int flag_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN = (*env)->GetStaticIntField(env, viewClass, id_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);
    const int flag_SYSTEM_UI_FLAG_HIDE_NAVIGATION = (*env)->GetStaticIntField(env, viewClass, id_SYSTEM_UI_FLAG_HIDE_NAVIGATION);
    const int flag_SYSTEM_UI_FLAG_FULLSCREEN = (*env)->GetStaticIntField(env, viewClass, id_SYSTEM_UI_FLAG_FULLSCREEN);
    const int flag_SYSTEM_UI_FLAG_IMMERSIVE_STICKY = (*env)->GetStaticIntField(env, viewClass, id_SYSTEM_UI_FLAG_IMMERSIVE_STICKY);

    const int flag =
        flag_SYSTEM_UI_FLAG_LAYOUT_STABLE |
        flag_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
        flag_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
        flag_SYSTEM_UI_FLAG_HIDE_NAVIGATION |
        flag_SYSTEM_UI_FLAG_FULLSCREEN |
        flag_SYSTEM_UI_FLAG_IMMERSIVE_STICKY;
    (*env)->CallVoidMethod(env, decorViewObj, setSystemUiVisibility, flag);
    if((*env)->ExceptionOccurred(env))
    {
        (*env)->ExceptionDescribe(env);
        jthrowable e = (*env)->ExceptionOccurred(env);
        (*env)->ExceptionClear(env); // clears the exception; e seems to remain valid
        jclass clazz = (*env)->GetObjectClass(env, e);
        jmethodID getMessage = (*env)->GetMethodID(env, clazz, "getMessage", "()Ljava/lang/String;");
        jstring message = (jstring)(*env)->CallObjectMethod(env, e, getMessage);
        const char *mstr = (*env)->GetStringUTFChars(env, message, NULL);
        (*env)->ReleaseStringUTFChars(env, message, mstr);
        (*env)->DeleteLocalRef(env, message);
        (*env)->DeleteLocalRef(env, clazz);
        (*env)->DeleteLocalRef(env, e);
        dengineutils_logging_log("set_immersive exception [%s]", mstr);
        success = 0;
    }else
    {
        dengineutils_logging_log("set_immersive success");
    }

    (*env)->DeleteLocalRef(env, windowObj);
    (*env)->DeleteLocalRef(env, decorViewObj);

    (*vm)->DetachCurrentThread(vm);

    return success;
}

int dengineutils_android_get_activityfocused()
{
    return isactivityfocused;
}

void dengineutils_android_handle_backbutton(int state)
{
    handlebackbutton = state;
}
void dengineutils_android_set_backbuttonfunc(DengineAndroidAppFunc func)
{
    onbackfunc = func;
}

