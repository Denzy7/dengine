#include <android_native_app_glue.h>
#include <jni.h>

#include <dengine/window.h>
#include <dengine-utils/logging.h>
#include <dengine/loadgl.h>
#include <dengine-utils/timer.h>

#include <dengine/primitive.h>
#include <dengine/draw.h>
#include <dengine/shader.h>
double elapsed;


Primitive quad;
Shader shader;

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

        shader.vertex_code =
                "attribute vec3 aPos;"
                "void main()"
                "{"
                "gl_Position = vec4(aPos, 1.0);"
                "}";
        shader.fragment_code =
                "precision mediump float;"
                "void main()"
                "{"
                "gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);"
                "}";
        dengine_shader_create(&shader);
        dengine_shader_setup(&shader);

        dengine_primitive_gen_quad(&quad, &shader);
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

    dengine_draw_primitive(&quad, &shader);

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
