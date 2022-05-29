#include <dengine/window.h> //window_init, create
#include <dengine/loadgl.h> //gl
#include <dengine/shader.h> //shader
#include <dengine/primitive.h>//prim
#include <dengine/draw.h> //draw_prim
#include <dengine-utils/logging.h>//log

Primitive quad;
Shader shader;

int main(int argc, char** argv)
{
    DengineWindow* window;
    if(!dengine_window_init() || !(window=dengine_window_create(1280, 720, "testdengine-2d-quad",NULL)))
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

    shader.vertex_code =
            "attribute vec3 aPos;"
            "void main()"
            "{"
                "gl_Position = vec4(aPos, 1.0);"
            "}";
    shader.fragment_code =
            "void main()"
            "{"
                "gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);"
            "}";
    dengine_shader_create(&shader);
    dengine_shader_setup(&shader);

    dengine_primitive_gen_quad(&quad, &shader);

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
