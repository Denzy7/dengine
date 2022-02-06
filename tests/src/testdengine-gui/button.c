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
    if(!dengine_window_init() || !dengine_window_glfw_create(1280, 720, "testdengine-windowtext"))
    {
        dengineutils_logging_log("ERROR::cannot create window\n");
        return 1;
    }
    GLFWwindow* current = dengine_window_glfw_get_currentwindow();
    dengine_window_glfw_context_makecurrent(current);

    if(!dengine_window_glfw_context_gladloadgl())
    {
        dengineutils_logging_log("ERROR::cannot load gl!\n");
        return 1;
    }

    dengineutils_logging_log("INFO::GL : %s\n", glGetString(GL_VERSION));
    File2Mem fontmem;

    char* ttf = dengineutils_os_dialog_fileopen("Select a .ttf font file");
    if(!ttf)
    {
        dengineutils_logging_log("ERROR::no file selected");
        return 1;
    }
    fontmem.file = ttf;
    dengineutils_filesys_file2mem_load(&fontmem);
    free(ttf);
    if(!denginegui_set_font(fontmem.mem, fontsz, 512))
        dengineutils_logging_log("ERROR::cannot load font");

    if(!denginegui_init())
        dengineutils_logging_log("ERROR::init gui failed!");

    dengine_input_init();

    glEnable(GL_CULL_FACE );

    while(dengine_window_isrunning())
    {
        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        char fontmsg[100];
        snprintf(fontmsg, sizeof(fontmsg), "Press Q or E to increase or decrease font size by +/- 1.0. Current : %.1f", fontsz);
        int h;
        dengine_window_get_window_height(&h);
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

        dengine_window_swapbuffers();
        dengine_window_glfw_pollevents();
    }

    dengineutils_filesys_file2mem_free(&fontmem);
    dengine_window_terminate();
    return 0;
}
