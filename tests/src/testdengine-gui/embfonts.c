#include <dengine/window.h> //window_init, create
#include <dengine/loadgl.h> //gl
#include <dengine/input.h>//io

#include <dengine-utils/logging.h>//log
#include <dengine-utils/filesys.h> //f2m
#include <dengine-utils/os.h> //fileopen

#include <dengine-gui/gui.h>    //text
#include <dengine-gui/embfonts.h>
#include <stdio.h> //snprintf
#include <stdlib.h> //free
float fontsz = 32.0;

int main(int argc, char** argv)
{
    DengineWindow* window;
    if(!dengine_window_init() || !(window=dengine_window_create(1280, 720, "testdengine-gui-embfonts",NULL)))
    {
        dengineutils_logging_log("ERROR::cannot create window\n");
        return 1;
    }
    dengine_window_makecurrent(window);
    if(!dengine_window_loadgl(window))
    {
        dengineutils_logging_log("ERROR::cannot load gl!\n");
        return 1;
    }

    int w, h;
    dengine_window_get_dim(window, &w, &h);
    dengineutils_logging_log("INFO::init window %dx%d\n", w, h);

    dengineutils_logging_log("INFO::GL : %s\n", glGetString(GL_VERSION));
    dengineutils_filesys_init();

    //OpenSans Light is always embedded unless its not!
    denginegui_set_font(OpenSans_Light_ttf, fontsz, 512);

    if(!denginegui_init())
        dengineutils_logging_log("ERROR::init gui failed!");

    glEnable(GL_CULL_FACE );

    while(dengine_window_isrunning(window))
    {
        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        float yellow[] = {1.0, 1.0, 0.0, 1.0};

        denginegui_text(40.0, 40.0, "textwindow abcgy ABCGY !.:/\\ 1234", NULL);

        denginegui_text(100.0, 100.0, "coloured text", yellow);

        float red_trans[] = {1.0, 0.0, 0.0, 0.6};
        denginegui_text(150.0, 100.0, "coloured tranparent text", red_trans);

        int h;
        dengine_window_get_dim(window, NULL, &h);

        char fontmsg[100];
        snprintf(fontmsg, sizeof(fontmsg), "Press Q or E to increase or decrease font size by +/- 1.0. Current : %.1f", fontsz);

        denginegui_text(10.0, h - 40.0, fontmsg, NULL);

        if(dengine_input_get_key_once('Q'))
        {
            fontsz -= 1.0;
            denginegui_set_font(OpenSans_Light_ttf, fontsz, 512);
        }else if(dengine_input_get_key_once('E')){
            fontsz += 1.0;
            denginegui_set_font(OpenSans_Light_ttf, fontsz, 512);
        }


        dengine_window_swapbuffers(window);
        dengine_window_poll(window);
    }
    denginegui_terminate();
    dengineutils_filesys_terminate();

    dengine_window_destroy(window);
    dengine_window_terminate();
    return 0;
}
