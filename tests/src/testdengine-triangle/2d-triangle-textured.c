#include <dengine/window.h> //window_init, create
#include <dengine/loadgl.h> //gl
#include <dengine/shader.h> //shader
#include <dengine-utils/logging.h>//log
#include <dengine/buffer.h> //buffer
#include <dengine/vertex.h> //vertex
#include <dengine/texture.h>//texture

#include <string.h> //memset
int main(int argc, char** argv)
{
    if(argc == 1)
    {
        dengineutils_logging_log("ERROR::Enter any 3 channel jpg or png as a texture to argv[1]");
    }

    if(!dengine_window_init() || !dengine_window_glfw_create(1280, 720, "testdengine-2dtriangle"))
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


    float vertices[] =
    {-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
      0.0f,  1.0f, 0.0f, 0.5f, 1.0f,
      1.0f, -1.0f, 0.0f, 1.0f, 0.0f
     //aPos             //aTexCoord
    };

    Buffer buffer;
    buffer.data = vertices;
    buffer.size = sizeof(vertices);
    buffer.usage = GL_STATIC_DRAW;

    dengine_buffer_gen(1, &buffer);
    dengine_buffer_bind(GL_ARRAY_BUFFER, &buffer);
    dengine_buffer_data(GL_ARRAY_BUFFER, &buffer);

    VertexAttribute aPos;
    aPos.size = 3;
    aPos.stride = sizeof(float) * 5;
    aPos.type = GL_FLOAT;
    aPos.ptr = NULL;
    dengine_vertex_attribute_indexfromshader(&shader, &aPos, "aPos");
    dengine_vertex_attribute_setup(&aPos);
    dengine_vertex_attribute_enable(&aPos);

    VertexAttribute aTexCoord;
    aTexCoord.size = 2;
    aTexCoord.stride = sizeof(float) * 5;
    aTexCoord.type = GL_FLOAT;
    aTexCoord.ptr = (void*)(3 * sizeof(float));
    dengine_vertex_attribute_indexfromshader(&shader, &aTexCoord, "aTexCoord");
    dengine_vertex_attribute_setup(&aTexCoord);
    dengine_vertex_attribute_enable(&aTexCoord);

    Texture brickwall;
    memset(&brickwall, 0, sizeof(Texture));
    brickwall.interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
    brickwall.internal_format = GL_RGB;
    brickwall.format = GL_RGB;
    brickwall.type = GL_UNSIGNED_BYTE;

    dengine_texture_gen(1, &brickwall);
    dengine_texture_bind(GL_TEXTURE_2D, &brickwall);
    dengine_texture_load_file(argv[1], 1, &brickwall);
    dengine_texture_data(GL_TEXTURE_2D, &brickwall);
    dengine_texture_set_params(GL_TEXTURE_2D, &brickwall);
    dengine_texture_free_data(&brickwall);

    while(dengine_window_isrunning())
    {
        glClearColor(1.0, 0.5, 0.3, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        dengine_shader_use(&shader);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        dengine_window_swapbuffers();
        dengine_window_glfw_pollevents();
    }

    dengine_window_terminate();
    return 0;
}
