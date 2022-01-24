#include <dengine/window.h> //window_init, create
#include <dengine/loadgl.h> //gl
#include <dengine/shader.h> //shader
#include <dengine-utils/logging.h>//log
#include <dengine/primitive.h> //gen_plane
#include <dengine/draw.h> //draw_prim
#include <dengine/camera.h> //camera
#include <dengine/texture.h> //tex
#include <string.h> //memset, memcpy
#include <cglm/cglm.h>      //mat4

#include <dengine-utils/os.h> //dialogopen
#include <stdlib.h> //free
int main(int argc, char** argv)
{
    if(!dengine_window_init() || !dengine_window_glfw_create(1280, 720, "testdengine-3dplane"))
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

            "uniform mat4 projection;"
            "uniform mat4 view;"
            "uniform mat4 model;"

            "varying vec2 TexCoord;"

            "void main()"
            "{"
                "gl_Position =  projection * view * model * vec4(aPos, 1.0);"
                "TexCoord = aTexCoord;"
            "}";
    shader.fragment_code =
            "varying vec2 TexCoord;"
            "uniform sampler2D texture;"
            "void main()"
            "{"
                "gl_FragColor = texture2D(texture, TexCoord);"
            "}";
    dengine_shader_create(&shader);
    dengine_shader_setup(&shader);

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

    dengine_texture_gen(1, &texture);
    //Dont unbind
    dengine_texture_bind(GL_TEXTURE_2D, &texture);
    dengine_texture_data(GL_TEXTURE_2D, &texture);
    dengine_texture_set_params(GL_TEXTURE_2D, &texture);
    dengine_texture_free_data(&texture);

    Primitive plane;
    dengine_primitive_gen_plane(&plane, &shader);

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

    glEnable(GL_CULL_FACE);

    while(dengine_window_isrunning())
    {
        glClearColor(1.0, 0.5, 0.3, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        dengine_draw_primitive(&plane, &shader);

        dengine_window_swapbuffers();
        dengine_window_glfw_pollevents();
    }

    dengine_window_terminate();
    return 0;
}
