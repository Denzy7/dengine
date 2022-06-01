#include <dengine/window.h> //window_init, create
#include <dengine/loadgl.h> //gl
#include <dengine/shader.h> //shader
#include <dengine-utils/logging.h>//log
#include <dengine/primitive.h> //gen_plane
#include <dengine/draw.h> //draw_prim
#include <dengine/camera.h> //camera
#include <dengine/texture.h>
#include <dengine/input.h>

#include <dengine-utils/os.h>//filedialog

#include <stdlib.h> //free
#include <string.h> //memset, memcpy
#include <cglm/cglm.h>      //mat4

int main(int argc, char** argv)
{
    DengineWindow* window;
    if(!dengine_window_init() || !(window=dengine_window_create(1280, 720, "testdengine-lightingnormal", NULL)))
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
            "attribute vec2 aTexCoord;"
            "attribute vec3 aNormal;"

            "uniform mat4 projection;"
            "uniform mat4 view;"
            "uniform mat4 model;"

            "varying vec2 TexCoord;"
            "varying vec3 Normal;"
            "varying vec3 FragPos;"

            "void main()"
            "{"
                "gl_Position =  projection * view * model * vec4(aPos, 1.0);"
                "TexCoord = aTexCoord;"
                "Normal = aNormal;"
                "FragPos = vec3(model * vec4(aPos, 1.0));"
            "}";
    shader.fragment_code =
            "varying vec2 TexCoord;"
            "varying vec3 Normal;"
            "varying vec3 FragPos;"

            "uniform vec3 ViewPos;"

            "uniform sampler2D texture;"

            "vec3 dirLight(vec3 dir, vec3 nNormal, vec3 viewDir){"
            "   float diff = max(dot(nNormal, -dir), 0.0);"
            "   vec3 diffuse = diff * vec3( texture2D(texture, TexCoord));"
            "   return diffuse;"
            "}"

            "void main()"
            "{"
                "vec3 nNormal = normalize(Normal);"
                "vec3 viewDir = normalize(ViewPos - FragPos);"
                "vec3 FragColor = vec3(0.0);"
                "vec3 lightDir = vec3(4.0, 5.0, 6.0);"
                "FragColor += dirLight(-normalize(lightDir), nNormal, viewDir);"
                "gl_FragColor = vec4(FragColor, 1.0);"
            "}";

    dengine_shader_create(&shader);
    dengine_shader_setup(&shader);

    Texture texture;
    memset(&texture, 0, sizeof(Texture));
    texture.interface = DENGINE_TEXTURE_INTERFACE_8_BIT;

    char* texfile = dengineutils_os_dialog_fileopen("Select a 3/4 channel png or jpg");
    if(!texfile)
    {
        dengineutils_logging_log("ERROR::No file selected!");
        return 1;
    }

    dengine_texture_load_file(texfile, 1, &texture);
    free(texfile);

    uint32_t fmt = texture.channels == 3 ? GL_RGB : GL_RGBA;
    texture.internal_format = fmt;
    texture.format = fmt;
    texture.type = GL_UNSIGNED_BYTE;
    texture.filter_min=GL_LINEAR;

    dengine_texture_gen(1, &texture);
    //Dont unbind
    dengine_texture_bind(GL_TEXTURE_2D, &texture);
    dengine_texture_data(GL_TEXTURE_2D, &texture);
    dengine_texture_set_params(GL_TEXTURE_2D, &texture);
    dengine_texture_free_data(&texture);

    Primitive cube;
    dengine_primitive_gen_cube(&cube, &shader);

    Primitive plane;
    dengine_primitive_gen_plane(&plane, &shader);

    Camera camera;
    camera.near = 0.01f;
    camera.far = 100.0f;
    camera.fov = 60.0f;
    float distance = 7.0f;
    float position[] = {distance, distance, distance};
    float target[] = {0.0f, 1.0f, 0.0f};

    memcpy(camera.position, position, sizeof(position));

    //FIXME : Break on resize window framebuffer
    dengine_camera_project_perspective((float)w / (float)h, &camera);
    dengine_camera_lookat(target, &camera);

    dengine_shader_set_mat4(&shader, "projection", camera.projection_mat);
    dengine_shader_set_mat4(&shader, "view", camera.view_mat);
    dengine_shader_set_vec3(&shader, "ViewPos", camera.position);

    //"wirframe" mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    //3d depth
    glEnable(GL_DEPTH_TEST);

    //face culling
    glEnable(GL_CULL_FACE);

    while(dengine_window_isrunning(window))
    {
        glClearColor(1.0, 0.5, 0.3, 1.0);
        //clear depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mat4 model;

        vec3 scale = {5.0, 5.0, 5.0};
        glm_mat4_identity(model);
        glm_scale(model, scale);
        dengine_shader_set_mat4(&shader, "model", model[0]);
        dengine_draw_primitive(&plane, &shader);

        vec3 pos = {0.0, 1.0, 0.0};
        glm_mat4_identity(model);
        glm_translate(model, pos);
        dengine_shader_set_mat4(&shader, "model", model[0]);
        dengine_draw_primitive(&cube, &shader);

        dengine_window_swapbuffers(window);
        dengine_window_poll(window);
    }

    dengine_window_destroy(window);
    dengine_window_terminate();
    return 0;
}
