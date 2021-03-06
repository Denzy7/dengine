#include <dengine/window.h>
#include <dengine/loadgl.h>

#include <dengine-utils/timer.h>
#include <dengine-utils/logging.h>
#include <dengine-utils/platform/android.h>

#include <dengine-gui/gui.h>

static void init(struct android_app* app)
{
    if(dengine_window_init())
    {
        dengineutils_logging_log("init window success");

        dengineutils_logging_log("GL : %s", glGetString(GL_VERSION));

        if(denginegui_init())
            dengineutils_logging_log("init gui");

        denginegui_set_font(NULL, 50.0f, 512);
    }
}

static void term(struct android_app* app)
{
    dengine_window_terminate();
}

static void draw()
{
    glClearColor(0.3, 0.2, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    denginegui_text(10.0f, 10.0f, "HELLLO WORLD! ANDROID?", NULL);

    float rgba[] = {1.0f, 1.0f, 0.0f, 1.0f};
    denginegui_text(50.0f, 50.0f, "Colored Text...~", rgba);
    denginegui_button(100.0f, 100.0f, 300.0f, 200.0f, "Click Me!", NULL);

    dengine_window_swapbuffers(DENGINE_WINDOW_CURRENT);
}

void android_main(struct android_app* app)
{
    dengineutils_android_set_app(app);
    dengineutils_android_set_initfunc(init);
    dengineutils_android_set_terminatefunc(term);

    while(1)
    {
        dengineutils_android_pollevents();

        //Quit and detach
        if(app->destroyRequested != 0)
        {
            dengineutils_logging_log("Goodbye!");
            return;
        }

        dengineutils_timer_update();

        static double elapsed;
        //print every 1secs
        elapsed += dengineutils_timer_get_delta();
        if(elapsed > 1000.0)
        {
            dengineutils_logging_log("step");
            elapsed = 0;
        }
        if(DENGINE_WINDOW_CURRENT)
        {
            draw();
        }
    }
}
