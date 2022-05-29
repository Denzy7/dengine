#include <stdio.h>

#include <dengine/window.h>
#include <dengine/loadgl.h>

#include <dengine/primitive.h>
#include <dengine/draw.h>
#include <dengine/camera.h>
#include <dengine-utils/logging.h>
#include <cglm/cglm.h>
#include <string.h>
int main()
{
    DengineWindow* window;
    if(!dengine_window_init() || !(window=dengine_window_create(1280, 720, "testdengine-tool-axis",NULL)))
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
    shader.vertex_code=
            "#version 100\n"
            "attribute vec3 aPos;"
            "uniform mat4 projection;"
            "uniform mat4 view;"
            "uniform mat4 model;"
            "void main()"
            "{"
                "gl_Position = projection * view * model * vec4(aPos, 1.0);"
            "}";
    shader.fragment_code =
            "#version 100\n"
            "precision mediump float;"
            "uniform vec3 color;"
            "void main()"
            "{"
                "gl_FragColor = vec4(color, 1.0);"
            "}";
    dengine_shader_create(&shader);
    dengine_shader_setup(&shader);

    Camera camera;
    camera.near = 0.01f;
    camera.far = 100.0f;
    camera.fov = 60.0f;
    float position[] = {3.0f, 3.0f, 3.0f};
    float target[] = {0.0f, 0.0f, 0.0f};

    memcpy(camera.position, position, sizeof(position));

    //FIXME : Break on resize window framebuffer
    dengine_camera_project_perspective((float)w / (float)h, &camera);
    dengine_camera_lookat(target, &camera);
    dengine_camera_apply(&shader, &camera);

    mat4 model;
    glm_mat4_identity(model);
    dengine_shader_set_mat4(&shader, "model", model[0]);

    Primitive axis;
    dengine_primitive_gen_axis(&axis, &shader);

    //By default we get 6 indices. Which is fine if you want a stale single colored axis
    //So we'll use 2 indices then offset to set each line colour (color for the american folk)
    axis.index_count = 2;

    //Change line size
    glLineWidth(4.0f);
    float color[3];
    while(dengine_window_isrunning(window))
    {
        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        //green (y)
        color[0] = 0.0f, color[1] = 1.0f, color[2] = 0.0f;
        axis.offset = NULL;
        dengine_shader_set_vec3(&shader, "color", color);
        dengine_draw_primitive(&axis, &shader);

        //red (x)
        color[0] = 1.0f, color[1] = 0.0f, color[2] = 0.0f;
        axis.offset = (void*)(2 * sizeof(uint16_t));
        dengine_shader_set_vec3(&shader, "color", color);
        dengine_draw_primitive(&axis, &shader);

        //blue (z)
        color[0] = 0.0f, color[1] = 0.0f, color[2] = 1.0f;
        axis.offset = (void*)(4 * sizeof(uint16_t));
        dengine_shader_set_vec3(&shader, "color", color);
        dengine_draw_primitive(&axis, &shader);

        dengine_window_swapbuffers(window);
        dengine_window_poll(window);
    }

    dengine_window_destroy(window);
    dengine_window_terminate();

    printf("Hello world!\n");
    return 0;
}
