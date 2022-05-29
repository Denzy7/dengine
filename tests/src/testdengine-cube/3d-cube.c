#include <dengine/window.h> //window_init, create
#include <dengine/loadgl.h> //gl
#include <dengine/shader.h> //shader
#include <dengine-utils/logging.h>//log
#include <dengine/primitive.h> //gen_plane
#include <dengine/draw.h> //draw_prim
#include <dengine/camera.h> //camera
#include <string.h> //memset, memcpy
#include <cglm/cglm.h>      //mat4

int main(int argc, char** argv)
{
    DengineWindow* window = NULL;
    if(!dengine_window_init() || !(window = dengine_window_create(1280, 720, "testdengine-3d-cube", NULL)))
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

    //use fullscreen 60Hz on primary monitor
    //dengine_window_glfw_set_monitor(glfwGetPrimaryMonitor(), 0, 0, 60);

    dengineutils_logging_log("INFO::GL : %s\n", glGetString(GL_VERSION));

    Shader shader;
    shader.vertex_code =
            "attribute vec3 aPos;"
            "uniform mat4 projection;"
            "uniform mat4 view;"
            "uniform mat4 model;"
            "void main()"
            "{"
                "gl_Position =  projection * view * model * vec4(aPos, 1.0);"
            "}";
    shader.fragment_code =
            "void main()"
            "{"
                "gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);"
            "}";
    dengine_shader_create(&shader);
    dengine_shader_setup(&shader);


    Primitive cube;
    dengine_primitive_gen_cube(&cube, &shader);

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

    mat4 model;
    glm_mat4_identity(model);

    dengine_shader_set_mat4(&shader, "projection", camera.projection_mat);
    dengine_shader_set_mat4(&shader, "view", camera.view_mat);
    dengine_shader_set_mat4(&shader, "model", model[0]);

    //"wirframe" mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while(dengine_window_isrunning(window))
    {
        glClearColor(1.0, 0.5, 0.3, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        dengine_draw_primitive(&cube, &shader);

        dengine_window_swapbuffers(window);
        dengine_window_poll(window);
    }
    dengine_window_destroy(window);
    dengine_window_terminate();
    return 0;
}
