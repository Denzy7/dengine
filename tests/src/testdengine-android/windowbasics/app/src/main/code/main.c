#include <android_native_app_glue.h>

#include <dengine/window.h>
#include <dengine-utils/logging.h>

static void init(struct android_app* app)
{
    //Acquire win
    ANativeWindow_acquire(app->window);
    dengine_window_android_set_nativewindow(app->window);

    dengine_window_request_GL(2, 0, 0);

    if(dengine_window_init())
    {
        dengine_logging_log("Init Window");
    }
}

static void destroy(struct android_app* app)
{
    dengine_window_terminate();

    ANativeWindow_release(app->window);
}

static void draw()
{
    dengine_window_swapbuffers();
}

static void cmd_handle(struct android_app* app, int32_t cmd)
{
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            dengine_logging_log("Saving state...");
            break;

        case APP_CMD_INIT_WINDOW:
            dengine_logging_log("Getting window ready...");
            init(app);
            //Buggy fullscreen
            //ANativeActivity_setWindowFlags(app->activity, AWINDOW_FLAG_FULLSCREEN, 0);
            break;

        case APP_CMD_TERM_WINDOW:
            dengine_logging_log("Term window");
            destroy(app);
            break;

        case APP_CMD_GAINED_FOCUS:
            dengine_logging_log("Gained focus");
            break;

        case APP_CMD_LOST_FOCUS:
            dengine_logging_log("Lost focus");
            break;

        case APP_CMD_PAUSE:
            dengine_logging_log("Paused");
            break;

        case APP_CMD_RESUME:
            dengine_logging_log("Resumed");
            break;

        case APP_CMD_DESTROY:
            dengine_logging_log("Destroy");
            break;
    }
}

void android_main(struct android_app* state)
{
    //Set app callbacks
    state->onAppCmd = cmd_handle;

    if(state->savedState)
    {
        dengine_logging_log("Restoring from save state");
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
            dengine_logging_log("Destroy Requested");
            ANativeActivity_finish(state->activity);
            state->activity->vm->DetachCurrentThread();

            dengine_logging_log("Goodbye!");
            return;
        }

        draw();
    }
}
