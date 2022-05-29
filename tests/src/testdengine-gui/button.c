#include <dengine/window.h> //window_init, create
#include <dengine/loadgl.h> //gl
#include <dengine/input.h>//io

#include <dengine-utils/logging.h>//log
#include <dengine-utils/filesys.h> //f2m
#include <dengine-utils/os.h> //fileopen

#include <dengine-gui/gui.h>    //text

#include <stdio.h> //snprintf
#include <stdlib.h> //free
#include <string.h> //memset
float fontsz = 32.0;

int main(int argc, char** argv)
{
    DengineWindow* window;
    if(!dengine_window_init() || !(window=dengine_window_create(1280, 720, "testdengine-gui-button",NULL)))
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

    File2Mem fontmem;

    char* ttf = dengineutils_os_dialog_fileopen("Select a .ttf font file");
    if(!ttf)
    {
        dengineutils_logging_log("ERROR::no file selected");
        return 1;
    }

    dengineutils_filesys_init();
    fontmem.file = ttf;
    dengineutils_filesys_file2mem_load(&fontmem);
    free(ttf);
    if(!denginegui_set_font(fontmem.mem, fontsz, 512))
        dengineutils_logging_log("ERROR::cannot load font");

    if(!denginegui_init())
        dengineutils_logging_log("ERROR::init gui failed!");

    glEnable(GL_CULL_FACE );

    while(dengine_window_isrunning(window))
    {
        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        char fontmsg[100];
        snprintf(fontmsg, sizeof(fontmsg), "Press Q or E to increase or decrease font size by +/- 1.0. Current : %.1f", fontsz);
        dengine_window_get_dim(window, NULL, &h);
        denginegui_text(10.0, h - 40.0, fontmsg, NULL);

        if(dengine_input_get_key_once('Q'))
        {
            fontsz -= 1.0;
            denginegui_set_font(fontmem.mem, fontsz, 512);
        }else if(dengine_input_get_key_once('E')){
            fontsz += 1.0;
            denginegui_set_font(fontmem.mem, fontsz, 512);
        }

        if(denginegui_button(100.0, 100.0, 140.0, 50.0, "Click Here", NULL))
        {
            dengineutils_logging_log("INFO::Clicked!");
            dengineutils_os_dialog_messagebox("clicked button", "ok", 0);
        }
        float red[4] = {1.0, 0.0, 0.0, 0.4};
        if(denginegui_button(300.0, 100.0, 200.0, 50.0, "Don't click me!", red))
        {
            dengineutils_logging_log("ERROR::Don't click me!!");
        }

        dengine_window_swapbuffers(window);
        dengine_window_poll(window);
    }

    denginegui_terminate();
    dengineutils_filesys_file2mem_free(&fontmem);

    dengineutils_filesys_terminate();
    dengine_window_destroy(window);
    dengine_window_terminate();
    return 0;
}
