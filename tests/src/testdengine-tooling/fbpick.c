#include <dengine/window.h> //window_init, create
#include <dengine/loadgl.h> //gl
#include <dengine/shader.h> //shader
#include <dengine-utils/logging.h>//log
#include <dengine/primitive.h> //gen_plane
#include <dengine/draw.h> //draw_prim
#include <dengine/camera.h> //camera
#include <string.h> //memset, memcpy
#include <cglm/cglm.h>      //mat4

#include <dengine/input.h> //mousepos
#include <dengine-gui/gui.h>//text
#include <dengine-utils/filesys.h> //f2m
#include <dengine-utils/os.h> //filedialog

#include <stdlib.h> //free
#include <stdio.h> //snprintf
#include <math.h> //round
int main(int argc, char** argv)
{
    if(!dengine_window_init() || !dengine_window_glfw_create(1280, 720, "testdengine-tooling-fbpick"))
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

    int w, h;
    dengine_window_get_window_width(&w);
    dengine_window_get_window_height(&h);
    dengineutils_logging_log("INFO::init window %dx%d\n", w, h);

    //use fullscreen 60Hz on primary monitor
    //dengine_window_glfw_set_monitor(glfwGetPrimaryMonitor(), 0, 0, 60);

    dengineutils_logging_log("INFO::GL : %s\n", glGetString(GL_VERSION));

    Shader shader;
    shader.vertex_code =
            "attribute vec2 aPos;"
            "uniform mat4 projection;"
            "uniform mat4 model;"
            "void main()"
            "{"
                "gl_Position =  projection * model * vec4(aPos, 0.0, 1.0);"
            "}";
    shader.fragment_code =
            "uniform float red;"
            "void main()"
            "{"
                "gl_FragColor = vec4(red, 0.0, 0.0, 1.0);"
            "}";
    dengine_shader_create(&shader);
    dengine_shader_setup(&shader);


    Primitive quad;
    dengine_primitive_gen_quad(&quad, &shader);

    mat4 proj, model;
    glm_ortho(0.0, w, 0.0, h, -1.0, 1.0, proj);
    dengine_shader_set_mat4(&shader, "projection", proj[0]);
    float scl = 25.0f;
    vec3 pos, scale = {scl, scl, scl};

    dengine_input_init();
    denginegui_init();

    char* fontfile = dengineutils_os_dialog_fileopen("Open a .tff or .otf font file...");
    if(!fontfile)
    {
        dengineutils_logging_log("ERROR::Please pick a font!");
        return 1;
    }

    File2Mem ttf;
    ttf.file = fontfile;
    dengineutils_filesys_file2mem_load(&ttf);
    free(fontfile);
    float fontsz = 32.0f;
    if(!denginegui_set_font(ttf.mem, fontsz, 512))
    {
        dengineutils_logging_log("ERROR::cannot load that font!");
        return 1;
    }

    char txtbuf[150];

    float space = 0.3f;

    while(dengine_window_isrunning())
    {
        glClearColor(0.0, 0.0, 0.0, 0.0); //zero out to avoid color mixup
        glClear(GL_COLOR_BUFFER_BIT);

        dengine_draw_primitive(&quad, &shader);

        uint32_t ids = 20;
        for(uint32_t i = 0; i < ids; i++)
        {
            glm_mat4_identity(model);
            pos[0] = (float)(2.0 + space) * i * scl + scl, pos[1] = scl, pos[2] = 0.0;
            glm_translate(model, pos);
            glm_scale(model, scale);
            float red = (float)(i + 1) / (float)ids;
            dengine_shader_set_float(&shader, "red", red);
            dengine_shader_set_mat4(&shader, "model", model[0]);

            dengine_draw_primitive(&quad, &shader);

            snprintf(txtbuf, sizeof(txtbuf), "%d", i + 1);
            denginegui_text((2.0 + space) * i * scl + (scl / 2), scl + fontsz , txtbuf, NULL);
        }

        double mx = dengine_input_get_mousepos_x();
        double my = dengine_input_get_mousepos_y();
        uint32_t px = 0;
        glFinish();
        glReadPixels((int)mx, (int)my, 1, 1, GL_RED, GL_UNSIGNED_INT, &px);
        double id_raw = ((double)px / (double)UINT32_MAX) * (double)ids;
        uint32_t id_rnd = round(id_raw);

        snprintf(txtbuf, sizeof(txtbuf), "px(red = uint32_t) : %u, hovering on : %u", px, id_rnd);
        denginegui_text(0.0, scl + (2 * fontsz) , txtbuf, NULL);

        denginegui_text(0.0, scl + (3 * fontsz), "This is just a demo reading window framebuffer. In an actual situation, a sole MRT framebuffer is used", NULL);

        dengine_window_swapbuffers();
        dengine_input_pollevents();
    }
    denginegui_terminate();
    dengineutils_filesys_file2mem_free(&ttf);
    dengine_window_terminate();
    return 0;
}
