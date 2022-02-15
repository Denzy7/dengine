#include <dengine/android.h>
#include <dengine/window.h>
#include <dengine/loadgl.h>

#include <dengine-utils/logging.h>
#include <dengine-utils/timer.h>

double elapsed;
int window_init = 0;
static void init(struct android_app* app)
{
    //Acquire win
    ANativeWindow_acquire(app->window);
    dengine_window_android_set_nativewindow(app->window);

    dengine_window_request_GL(2, 0, 0);

    if(dengine_window_init())
    {
		if(!dengine_window_loadgl())
		{
			dengineutils_logging_log("ERROR::Cannot load GL!");
		}else
		{
			window_init = 1;
		}
		
        dengineutils_logging_log("init window success");

        dengineutils_logging_log("GL : %s", glGetString(GL_VERSION));
    }
}

static void term(struct  android_app* app)
{
    dengine_window_terminate();
    ANativeWindow_release(app->window);
    ANativeActivity_finish(app->activity);
}

static void draw()
{
    glClearColor(0.3, 0.2, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
	
    dengine_window_swapbuffers();
}

void android_main(struct android_app* state)
{
    dengine_android_set_app(state);
    dengine_android_set_initfunc(init);
    dengine_android_set_terminatefunc(term);

    if(state->savedState)
    {
        dengineutils_logging_log("Restoring from save state");
    }

    while(1)
    {
        dengine_android_pollevents();

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
		
		if(window_init)
        	draw();
    }
}
