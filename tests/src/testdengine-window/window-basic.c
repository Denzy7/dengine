#include <stdio.h>
#include <stdlib.h>

#include <dengine/window.h>

int main()
{
    if(!dengine_window_init() || !dengine_window_glfw_create(1280, 720, "testdengine-windowbasic"))
    {
        printf("cannot create window!\n");
        return 1;
    }

    int w, h;
    dengine_window_get_window_width(&w);
    dengine_window_get_window_height(&h);
    printf("init window %dx%d\n", w, h);

    //use fullscreen 60Hz on primary monitor
    dengine_window_glfw_set_monitor(glfwGetPrimaryMonitor(), 0, 0, 60);

    while(dengine_window_isrunning())
    {
        dengine_window_glfw_pollevents();
        dengine_window_swapbuffers();
    }

    dengine_window_terminate();

    return 0;
}
