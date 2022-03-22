#include <dengine/window.h> //window_init, create
#include <dengine/loadgl.h> //gl
#include <dengine/shader.h> //shader
#include <dengine-utils/logging.h>//log
#include <dengine/primitive.h> //gen_plane
#include <dengine/draw.h> //draw_prim
#include <dengine/camera.h> //camera
#include <dengine/input.h> //pollevents
#include <string.h> //memset, memcpy
#include <cglm/cglm.h>      //mat4

#include <dengine-model/model.h> //loadobj
#include <dengine-utils/os.h> //filedialog
#include <stdlib.h> //free
int main(int argc, char** argv)
{
    if(!dengine_window_init() || !dengine_window_glfw_create(1280, 720, "testdengine-modelobj"))
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
            "attribute vec3 aNormal;"
            "uniform mat4 projection;"
            "uniform mat4 view;"
            "uniform mat4 model;"

            "varying vec3 Normal;"

            "void main()"
            "{"
                "gl_Position =  projection * view * model * vec4(aPos, 1.0);"
                "Normal = aNormal;"
            "}";
    shader.fragment_code =
            "varying vec3 Normal;"
            "void main()"
            "{"
                "gl_FragColor = vec4(Normal, 1.0);"
            "}";
    dengine_shader_create(&shader);
    dengine_shader_setup(&shader);

    char* file = dengineutils_os_dialog_fileopen("Select a wavefront .obj file");
    if (!file) {
        dengineutils_logging_log("ERROR::No file selected!");
        return 1;
    }
    size_t meshes;
    Primitive* loaded = denginemodel_load_file(DENGINE_MODEL_FORMAT_OBJ, file, &meshes, &shader);
    free(file);

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

    //"wirframe" mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    while(dengine_window_isrunning())
    {
        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (size_t i=0; i<meshes; i++) {
            dengine_draw_primitive(&loaded[i], &shader);
        }

        dengine_window_swapbuffers();
        dengine_input_pollevents();
    }

    free(loaded);

    dengine_window_terminate();
    return 0;
}
