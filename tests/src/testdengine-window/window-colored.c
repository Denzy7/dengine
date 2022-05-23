#include <stdio.h>

#include <dengine/window.h>
#include <dengine/loadgl.h>

int main()
{
    if(!dengine_window_init())
    {
        printf("cannot create init window!\n");
        return 1;
    }

    DengineWindow* window = dengine_window_create(1280, 720, "testdengine-window-colored", NULL);
    if(!window)
    {
        printf("cannot create create window!\n");
        return 1;
    }
    if(!dengine_window_makecurrent(window))
    {
        printf("cannot create makecurrent window!\n");
        return 1;
    }

    if(!dengine_window_loadgl(window))
    {
        printf("cannot loadgl window!\n");
        return 1;
    }

    int w, h;
    dengine_window_get_dim(window, &w, &h);
    printf("init window %dx%d\n", w, h);

    //use fullscreen 60Hz on primary monitor
    //dengine_window_glfw_set_monitor(glfwGetPrimaryMonitor(), 0, 0, 60);

    printf("GL : %s\n", glGetString(GL_VERSION));

    while(dengine_window_isrunning(window))
    {
        glClearColor(1.0, 0.5, 0.3, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        dengine_window_swapbuffers(window);
        dengine_window_poll(window);
    }

    dengine_window_terminate();
    return 0;
}
