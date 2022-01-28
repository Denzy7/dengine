#include <stdio.h>

#include <dengine/window.h>
#include <dengine/loadgl.h>

#include <dengine/input.h>
int padhint;
int main()
{
    if(!dengine_window_init() || !dengine_window_glfw_create(1280, 720, "testdengine-windowcolored"))
    {
        printf("cannot create window\n");
        return 1;
    }
    GLFWwindow* current = dengine_window_glfw_get_currentwindow();
    dengine_window_glfw_context_makecurrent(current);

    dengine_input_init();

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
    //dengine_window_glfw_set_monitor(glfwGetPrimaryMonitor(), 0, 0, 60);

    printf("GL : %s\n", glGetString(GL_VERSION));
    printf("mouse mouse around to change color based on position\n");
    printf("press w, x, lmb or rmb\n");
    printf("drag scroll wheel\n");

    while(dengine_window_isrunning())
    {
        if(dengine_input_gamepad_get_isconnected(0) && !padhint)
        {
            printf("connected : %s\n", dengine_input_gamepad_get_name(0));
            printf("hold left trigger and right trigger to print their values\n");
            padhint = 1;
        }

        //most important function
        dengine_window_glfw_pollevents();

        double mousex = dengine_input_get_mousepos_x();
        double mousey = dengine_input_get_mousepos_y();

        glClearColor(mousex / 1280.0, mousey / 720.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        dengine_window_swapbuffers();

        if(dengine_input_get_key('X'))
        {
            printf("pressing x\n");
        }

        if(dengine_input_get_key_once('W'))
        {
            printf("pressed w once\n");
        }

        double scrolly = dengine_input_get_mousescroll_y();

        if(scrolly)
            printf("scrolling y %f\n", scrolly);

        if(dengine_input_get_mousebtn(0))
            printf("pressing lmb\n");

        if(dengine_input_get_mousebtn_once(1))
            printf("pressing rmb once\n");

        if(dengine_input_gamepad_get_isconnected(0))
        {
            if(dengine_input_gamepad_get_btn(0, 0))
            {
                printf("pressing btn 0 (A)\n");
            }

            //4 = lt, 5 = rt
            float lt = dengine_input_gamepad_get_axis(0, 4);
            float rt = dengine_input_gamepad_get_axis(0, 5);
            if(lt > 0.0f || rt > 0.0f)
            {
                printf("LT : %f, RT : %f\n", lt, rt);
            }
        }
    }

    dengine_window_terminate();


    printf("Hello world!\n");
    return 0;
}
