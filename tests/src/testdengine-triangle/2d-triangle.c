#include <dengine/window.h> //window_init, create
#include <dengine/loadgl.h> //gl
#include <dengine/shader.h> //shader
#include <dengine-utils/logging.h>//log
#include <dengine/buffer.h> //buffer
#include <dengine/vertex.h> //vertex
int main()
{
    DengineWindow* window;
    if(!dengine_window_init() || !(window=dengine_window_create(1280, 720, "testdengine-2d-triangle",NULL)))
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
            "void main()"
            "{"
                "gl_Position = vec4(aPos, 1.0);"
            "}";
    shader.fragment_code =
            "void main()"
            "{"
                "gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);"
            "}";
    dengine_shader_create(&shader);
    dengine_shader_setup(&shader);


    float vertices[] =
    {-1.0f, -1.0f, 0.0f,
      0.0f,  1.0f, 0.0f,
      1.0f, -1.0f, 0.0f
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
    aPos.stride = sizeof(float) * 3;
    aPos.type = GL_FLOAT;
    aPos.ptr = NULL;
    dengine_vertex_attribute_indexfromshader(&shader, &aPos, "aPos");
    dengine_vertex_attribute_setup(&aPos);
    dengine_vertex_attribute_enable(&aPos);

    while(dengine_window_isrunning(window))
    {
        glClearColor(1.0, 0.5, 0.3, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        dengine_shader_use(&shader);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        dengine_window_swapbuffers(window);
        dengine_window_poll(window);
    }

    dengine_window_destroy(window);
    dengine_window_terminate();
    return 0;
}
