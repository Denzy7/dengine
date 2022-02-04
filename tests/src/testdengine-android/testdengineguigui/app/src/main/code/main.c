#include <android_native_app_glue.h>
#include <jni.h>
#include <android/asset_manager.h>

#include <dengine/window.h>
#include <dengine-utils/logging.h>
#include <dengine/loadgl.h>
#include <dengine-utils/timer.h>

#include <dengine/primitive.h>
#include <dengine/draw.h>
#include <dengine/shader.h>
#include <string.h>
#include <dengine/camera.h>
#include <cglm/cglm.h>
double elapsed;

#include <dengine-gui/gui.h>

AAssetManager* asset_mgr;

void* a2m(const char* file, size_t* sz)
{
    if(!asset_mgr)
        return NULL;
    AAsset* asset = AAssetManager_open(asset_mgr, file, AASSET_MODE_BUFFER);
    if(!asset)
        return NULL;

    *sz = AAsset_getLength(asset);

    //nullterm for txt
    void* mem = malloc(*sz + 1);
    memset(mem, 0, *sz + 1);
    AAsset_read(asset, mem, *sz);
    AAsset_close(asset);
    return mem;
}


static void init(struct android_app* app)
{
    //Acquire win
    ANativeWindow_acquire(app->window);
    dengine_window_android_set_nativewindow(app->window);

    dengine_window_request_GL(2, 0, 0);

    if(dengine_window_init())
    {
        dengineutils_logging_log("init window success");

        dengineutils_logging_log("GL : %s", glGetString(GL_VERSION));

        if(denginegui_init())
            dengineutils_logging_log("init gui");

        size_t sz = 0;
        void* ttf = a2m("OpenSans-Regular.ttf", &sz);
        if(denginegui_set_font(ttf, 50.0f, 512))
            free(ttf);
    }
}

static void destroy(struct android_app* app)
{
    dengine_window_terminate();

    ANativeWindow_release(app->window);
}

static void draw()
{
    glClearColor(0.3, 0.2, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    denginegui_text(10.0f, 10.0f, "HELLLO WORLD! ANDROID?", NULL);

    float rgba[] = {1.0f, 1.0f, 0.0f, 1.0f};
    denginegui_text(50.0f, 50.0f, "Colored Text...~", rgb);

    dengine_window_swapbuffers();
}

static void cmd_handle(struct android_app* app, int32_t cmd)
{
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            dengineutils_logging_log("Saving state...");
            break;

        case APP_CMD_INIT_WINDOW:
            dengineutils_logging_log("Getting window ready...");
            init(app);
            //Buggy fullscreen
            //ANativeActivity_setWindowFlags(app->activity, AWINDOW_FLAG_FULLSCREEN, 0);
            break;

        case APP_CMD_TERM_WINDOW:
            dengineutils_logging_log("Term window");
            destroy(app);
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

void android_main(struct android_app* state)
{
    //Set app callbacks
    state->onAppCmd = cmd_handle;
    asset_mgr = state->activity->assetManager;

    if(state->savedState)
    {
        dengineutils_logging_log("Restoring from save state");
    }

    while(1)
    {
        //Read events
        int events;
        struct android_poll_source* source;

        while((ALooper_pollAll(1 ? 0 : -1, NULL, &events, (void**)&source)) >= 0)
        {
            //Process event
            if(source != NULL)
            {
                source->process(state, source);
            }
        }

        //Quit and detach
        if(state->destroyRequested != 0)
        {
            dengineutils_logging_log("Destroy Requested");
            ANativeActivity_finish(state->activity);

            //state->activity->vm->DetachCurrentThread();

            dengineutils_logging_log("Goodbye!");
            return;
        }

        dengineutils_timer_update();

        //print every 1secs
        elapsed += dengineutils_timer_get_delta();
        if(elapsed > 1000.0)
        {
            dengineutils_logging_log("step");
            elapsed = 0;
        }

        draw();
    }
}
