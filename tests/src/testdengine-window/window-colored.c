#include <stdio.h>

#include <dengine/window.h>
#include <dengine/loadgl.h>

int main()
{
    if(!dengine_window_init() || !dengine_window_glfw_create(1280, 720, "testdengine-windowcolored"))
    {
        printf("cannot create window\n");
        return 1;
    }
    GLFWwindow* current = dengine_window_glfw_get_currentwindow();
    dengine_window_glfw_context_makecurrent(current);

    if(!dengine_window_glfw_context_gladloadgl())
    {
        printf("cannot load gl!\n");
        return 1;
    }

    int w, h;
    dengine_window_get_window_width(&w);
    dengine_window_get_window_height(&h);
    printf("init window %dx%d\n", w, h);

    //use fullscreen 60Hz on primary monitor
    dengine_window_glfw_set_monitor(glfwGetPrimaryMonitor(), 0, 0, 60);

    printf("GL : %s\n", glGetString(GL_VERSION));

    while(dengine_window_isrunning())
    {
        glClearColor(1.0, 0.5, 0.3, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        dengine_window_swapbuffers();
        dengine_window_glfw_pollevents();
    }

    dengine_window_terminate();


    printf("Hello world!\n");
    return 0;
}
