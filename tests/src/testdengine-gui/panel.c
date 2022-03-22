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
    //glEnable(GL_DEPTH_TEST);

    char* paneltex = dengineutils_os_dialog_fileopen("Choose a 3 or 4 channel image...");
    if(!paneltex)
    {
        dengineutils_logging_log("ERROR::no file selected");
        return 1;
    }

    Texture texture;
    memset(&texture, 0, sizeof(Texture));
    texture.interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
    dengine_texture_load_file(paneltex, 1, &texture);
    free(paneltex);
    uint32_t fmt = texture.channels == 3 ? GL_RGB : GL_RGBA;
    texture.internal_format = fmt;
    texture.format = fmt;
    texture.type = GL_UNSIGNED_BYTE;
    texture.filter_min=GL_LINEAR;

    dengine_texture_gen(1, &texture);
    //Dont unbind
    dengine_texture_bind(GL_TEXTURE_2D, &texture);
    dengine_texture_data(GL_TEXTURE_2D, &texture);
    dengine_texture_set_params(GL_TEXTURE_2D, &texture);
    dengine_texture_bind(GL_TEXTURE_2D, NULL);
    dengine_texture_free_data(&texture);

    while(dengine_window_isrunning())
    {
        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        float panel_col[4] = {0.4f, 0.2f, 0.6f, .25f};
        denginegui_panel(100.0, 100.0 + (fontsz / 2), 400.0, (3*fontsz) + (fontsz / 2), NULL, NULL, panel_col);

        float yellow[] = {1.0, 1.0, 0.0, 1.0};
        denginegui_text(100.0 + (fontsz / 4), 100.0 + fontsz, "Text in a Panel", NULL);

        denginegui_text(100.0 + (fontsz / 4), 100.0 + (2*fontsz), "Colored Text in a Panel", yellow);

        float red_trans[] = {1.0, 0.0, 0.0, 0.6};
        denginegui_text(100.0 + (fontsz / 4), 100.0 + (3*fontsz), "Colored Transparent Text in a Panel", red_trans);

        denginegui_panel(450.0, 450.0 + (fontsz / 2) , 300.0, fontsz + (fontsz / 2), &texture, NULL, NULL);
        denginegui_text(450.0 + (fontsz / 4), 450.0 + fontsz, "This panel has a texture", NULL);
        int h;
        dengine_window_get_window_height(&h);

        char fontmsg[100];
        snprintf(fontmsg, sizeof(fontmsg), "Press Q or E to increase or decrease font size by +/- 1.0. Current : %.1f. Panels are also scaled", fontsz);

        denginegui_text(10.0, h - 40.0, fontmsg, NULL);

        if(dengine_input_get_key_once('Q'))
        {
            fontsz -= 1.0;
            denginegui_set_font(fontmem.mem, fontsz, 512);
        }else if(dengine_input_get_key_once('E')){
            fontsz += 1.0;
            denginegui_set_font(fontmem.mem, fontsz, 512);
        }


        dengine_window_swapbuffers();
        dengine_input_pollevents();
    }

    denginegui_terminate();
    dengineutils_filesys_file2mem_free(&fontmem);
    dengine_window_terminate();
    return 0;
}
