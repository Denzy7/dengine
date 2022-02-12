#include <stdio.h>

#include <dengine/window.h>
#include <dengine/loadgl.h>

#include <dengine/primitive.h>
#include <dengine/draw.h>
#include <dengine/camera.h>

#include <cglm/cglm.h>
#include <string.h>
int main()
{
    if(!dengine_window_init() || !dengine_window_glfw_create(1280, 720, "testdengine-tooling3dgrid"))
    {
        printf("cannot create window\n");
        return 1;
    }

    GLFWwindow* current = dengine_window_glfw_get_currentwindow();
    dengine_window_glfw_context_makecurrent(current);

    if(!dengine_window_glfw_context_gladloadgl())
    {
        printf("cannot load gl!\n");
        return 1;
    }

    int w, h;
    dengine_window_get_window_width(&w);
    dengine_window_get_window_height(&h);
    printf("init window %dx%d\n", w, h);

    //use fullscreen 60Hz on primary monitor
    //dengine_window_glfw_set_monitor(glfwGetPrimaryMonitor(), 0, 0, 60);

    printf("GL : %s\n", glGetString(GL_VERSION));

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

    while(dengine_window_isrunning())
    {
        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        dengine_draw_primitive(&grid, &shader);

        dengine_window_swapbuffers();
        dengine_window_glfw_pollevents();
    }

    dengine_window_terminate();


    printf("Hello world!\n");
    return 0;
}
