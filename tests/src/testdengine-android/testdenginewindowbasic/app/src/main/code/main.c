#include <dengine/window.h>
#include <dengine/loadgl.h>

#include <dengine-utils/logging.h>
#include <dengine-utils/timer.h>
#include <dengine-utils/platform/android.h>

double elapsed;

static void init(struct android_app* app)
{
    if(dengine_window_init())
    {
        dengineutils_logging_log("GL : %s", glGetString(GL_VERSION));
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
	
    dengine_window_swapbuffers(DENGINE_WINDOW_CURRENT);
}

void android_main(struct android_app* state)
{
    dengineutils_android_set_app(state);
    dengineutils_android_set_initfunc(init);
    dengineutils_android_set_terminatefunc(term);

    if(state->savedState)
    {
        dengineutils_logging_log("Restoring from save state");
    }

    //wait for window
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

        //print every 5secs
        elapsed += dengineutils_timer_get_delta();
        if(elapsed > 5000.0)
        {
            dengineutils_logging_log("step");
            elapsed = 0;
        }
        if(DENGINE_WINDOW_CURRENT)
            draw();
    }
}
