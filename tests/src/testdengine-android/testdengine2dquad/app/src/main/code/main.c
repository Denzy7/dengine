#include <dengine/window.h>
#include <dengine/loadgl.h>

#include <dengine/primitive.h>
#include <dengine/draw.h>

#include <dengine-utils/logging.h>
#include <dengine-utils/timer.h>
#include <dengine-utils/platform/android.h>
double elapsed;

Primitive quad;
Shader shader;

static void init(struct android_app* app)
{
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

static void term(struct  android_app* app)
{
    dengine_window_terminate();
}

static void draw()
{
    glClearColor(0.3, 0.2, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    dengine_draw_primitive(&quad, &shader);

    dengine_window_swapbuffers(DENGINE_WINDOW_CURRENT);
}

void android_main(struct android_app* state)
{
    //Set app callbacks
    dengineutils_android_set_app(state);
    dengineutils_android_set_initfunc(init);
    dengineutils_android_set_terminatefunc(term);

    if(state->savedState)
    {
        dengineutils_logging_log("Restoring from save state");
    }

    while(1)
    {
        dengineutils_android_pollevents();

        //Quit and detach
        if(state->destroyRequested != 0)
        {
            dengineutils_logging_log("Destroy Requested");
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
        if(DENGINE_WINDOW_CURRENT)
        	draw();
    }
}
