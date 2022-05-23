#include <stdio.h>
#include <stdlib.h>

#include <dengine/window.h>

int main()
{
    if(!dengine_window_init())
    {
        printf("cannot create init window!\n");
        return 1;
    }
    DengineWindow* window = dengine_window_create(1280, 720, "testdengine-window-basic", NULL);
    if(!window)
    {
        printf("cannot create create window!\n");
        return 1;
    }
    int w, h;
    dengine_window_get_dim(window, &w, &h);
    printf("init window %dx%d\n", w, h);

    //use fullscreen 60Hz on primary monitor
    //dengine_window_glfw_set_monitor(glfwGetPrimaryMonitor(), 0, 0, 60);

    while(dengine_window_isrunning(window))
    {
        dengine_window_poll(window);
        dengine_window_swapbuffers(window);
    }

    dengine_window_terminate();

    return 0;
}
