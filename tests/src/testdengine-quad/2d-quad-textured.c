#include <dengine/window.h> //window_init, create
#include <dengine/loadgl.h> //gl
#include <dengine/shader.h> //shader
#include <dengine-utils/logging.h>//log
#include <dengine/texture.h>//
#include <dengine/primitive.h>//prim
#include <dengine/draw.h> //draw_prim
#include <string.h> //memset

#include <dengine-utils/os.h>//dialogopen
#include <stdlib.h> //free
int main(int argc, char** argv)
{
    DengineWindow* window;
    if(!dengine_window_init() || !(window=dengine_window_create(1280, 720, "testdengine-2d-quadtextured",NULL)))
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
    Shader shader;
    shader.vertex_code =
            "attribute vec3 aPos;"
            "attribute vec2 aTexCoord;"
            "varying vec2 TexCoord;"
            "void main()"
            "{"
                "gl_Position = vec4(aPos, 1.0);"
                "TexCoord = aTexCoord;"
            "}";
    shader.fragment_code =
            "uniform sampler2D texture;"
            "varying vec2 TexCoord;"
            "void main()"
            "{"
                "gl_FragColor = texture2D(texture, TexCoord);"
            "}";
    dengine_shader_create(&shader);
    dengine_shader_setup(&shader);

    Primitive quad;
    dengine_primitive_gen_quad(&quad, &shader);

    Texture texture;
    memset(&texture, 0, sizeof(Texture));
    texture.interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
    char* tex_file = dengineutils_os_dialog_fileopen("Select a 3 or 4 channel png or jpg");
    dengine_texture_load_file(tex_file, 1, &texture);
    free(tex_file);
    uint32_t fmt = texture.channels == 3 ? GL_RGB : GL_RGBA;
    texture.internal_format = fmt;
    texture.format = fmt;
    texture.type = GL_UNSIGNED_BYTE;
    texture.filter_min=GL_LINEAR;

    //resize to texture size
    if(texture.width > 0)
        dengine_window_resize(window, texture.width, texture.height);

    dengine_texture_gen(1, &texture);
    dengine_texture_bind(GL_TEXTURE_2D, &texture);

    dengine_texture_data(GL_TEXTURE_2D, &texture);
    dengine_texture_set_params(GL_TEXTURE_2D, &texture);
    dengine_texture_free_data(&texture);

    while(dengine_window_isrunning(window))
    {
        glClearColor(1.0, 0.5, 0.3, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        dengine_draw_primitive(&quad, &shader);

        dengine_window_swapbuffers(window);
        dengine_window_poll(window);
    }

    dengine_window_destroy(window);
    dengine_window_terminate();
    return 0;
}

//could use this as a DIY image viewer haha...
