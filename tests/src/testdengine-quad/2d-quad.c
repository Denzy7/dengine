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
    if(!dengine_window_init() || !dengine_window_glfw_create(1280, 720, "testdengine-2dquad"))
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
                "gl_FragColor = texture2D(texture, TexCoord) + vec4(1.0, 0.0, 1.0, 1.0);"
            "}";
    dengine_shader_create(&shader);
    dengine_shader_setup(&shader);

    dengine_primitive_gen_quad(&quad, &shader);


    while(dengine_window_isrunning())
    {
        glClearColor(1.0, 0.5, 0.3, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        dengine_draw_primitive(&quad, &shader);

        dengine_window_swapbuffers();
        dengine_window_glfw_pollevents();
    }

    dengine_window_terminate();
    return 0;
}
