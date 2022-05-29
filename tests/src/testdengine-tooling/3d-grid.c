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
    if(!dengine_window_init() || !(window=dengine_window_create(1280, 720, "testdengine-tool-grid",NULL)))
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
            "void main()"
            "{"
                "gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);"
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
    vec3 scale = {3.0f, 3.0f, 3.0f};
    glm_scale(model, scale);
    dengine_shader_set_mat4(&shader, "model", model[0]);

    Primitive grid;
    dengine_primitive_gen_grid(10, &grid, &shader);

    //Change line size
    glLineWidth(2.0f);

    while(dengine_window_isrunning(window))
    {
        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        dengine_draw_primitive(&grid, &shader);

        dengine_window_swapbuffers(window);
        dengine_window_poll(window);
    }

    dengine_window_destroy(window);
    dengine_window_terminate();


    printf("Hello world!\n");
    return 0;
}
