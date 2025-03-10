#include <stdio.h>

#include <dengine/window.h>
#include <dengine/loadgl.h>

#include <dengine/input.h>

int padhint;
int main()
{
    if(!dengine_window_init())
    {
        printf("cannot create init window!\n");
        return 1;
    }
    DengineWindow* window = dengine_window_create(1280, 720, "testdengine-input-basic", NULL);
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

    printf("GL : %s\n", glGetString(GL_VERSION));
    printf("mouse mouse around to change color based on position\n");
    printf("press w, x, lmb or rmb\n");
    printf("drag scroll wheel\n");

    while(dengine_window_isrunning(window))
    {
        if(dengine_input_gamepad_get_isconnected(DENGINE_INPUT_PAD0) && !padhint)
        {
            printf("connected : %s\n", dengine_input_gamepad_get_name(DENGINE_INPUT_PAD0));
            printf("hold left trigger and right trigger to print their values and control vibration\n");
            padhint = 1;
        }

        //most important function
        dengine_window_poll(window);

        double mousex = dengine_input_get_mousepos_x();
        double mousey = dengine_input_get_mousepos_y();

        glClearColor(mousex / 1280.0, mousey / 720.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        dengine_window_swapbuffers(window);

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

        if(dengine_input_get_mousebtn(DENGINE_INPUT_MSEBTN_PRIMARY))
            printf("pressing lmb\n");

        if(dengine_input_get_mousebtn_once(DENGINE_INPUT_MSEBTN_SECONDARY))
            printf("pressing rmb once\n");

        if(dengine_input_gamepad_get_isconnected(DENGINE_INPUT_PAD0))
        {
            if(dengine_input_gamepad_get_btn(DENGINE_INPUT_PAD0, DENGINE_INPUT_PAD_BUTTON_A))
            {
                printf("pressing gamepad btn (A)\n");
            }

            if(dengine_input_gamepad_get_btn_once(DENGINE_INPUT_PAD0, DENGINE_INPUT_PAD_BUTTON_B))
                printf("pressing B once\n");

            //4 = lt, 5 = rt
            float lt = dengine_input_gamepad_get_axis(DENGINE_INPUT_PAD0, DENGINE_INPUT_PAD_AXIS_LT);
            float rt = dengine_input_gamepad_get_axis(DENGINE_INPUT_PAD0, DENGINE_INPUT_PAD_AXIS_RT);
            if(lt > 0.0f || rt > 0.0f)
            {
                printf("LT : %f, RT : %f\n", lt, rt);
                dengine_input_gamepad_vibration_set_basic(DENGINE_INPUT_PAD0, lt, rt);
            }else{
                dengine_input_gamepad_vibration_set_basic(DENGINE_INPUT_PAD0, 0.0f, 0.0f);
            }
        }
    }

    dengine_window_terminate();


    printf("Hello world!\n");
    return 0;
}
