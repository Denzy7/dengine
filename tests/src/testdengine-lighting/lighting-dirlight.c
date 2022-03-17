#include <dengine/window.h> //window_init, create
#include <dengine/loadgl.h> //gl
#include <dengine/shader.h> //shader
#include <dengine-utils/logging.h>//log
#include <dengine/primitive.h> //gen_plane
#include <dengine/draw.h> //draw_prim
#include <dengine/camera.h> //camera
#include <dengine/texture.h>

#include <dengine/lighting.h>
#include <dengine-gui/gui.h> //panel

#include <dengine-utils/os.h> //filedialogopen
#include <dengine/input.h>
#include <string.h> //memset, memcpy
#include <cglm/cglm.h>      //mat4

int main(int argc, char** argv)
{

    if(!dengine_window_init() || !dengine_window_glfw_create(1280, 720, "testdengine-dirlight"))
    {
        dengineutils_logging_log("ERROR::cannot init window\n");
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
            "#version 100\n"
            "attribute vec3 aPos;"
            "attribute vec2 aTexCoord;"
            "attribute vec3 aNormal;"

            "uniform mat4 projection;"
            "uniform mat4 view;"
            "uniform mat4 model;"
            "uniform mat4 ivTpModel;"

            "varying vec2 TexCoord;"
            "varying vec3 Normal;"
            "varying vec3 FragPos;"

            "void main()"
            "{"
                "gl_Position =  projection * view * model * vec4(aPos, 1.0);"
                "TexCoord = aTexCoord;"
                "Normal = mat3(ivTpModel) * aNormal;"
                "FragPos = vec3(model * vec4(aPos, 1.0));"
            "}";
    shader.fragment_code =
            "#version 100\n"
            "precision mediump float;"
            "varying vec2 TexCoord;"
            "varying vec3 Normal;"
            "varying vec3 FragPos;"

            "uniform vec3 ViewPos;"

            "uniform sampler2D texture;"
            "uniform sampler2D shadowmap;"
            "uniform vec3 lightDir;"
            "uniform mat4 projview_shadow;"

            "float shadow_bias = 0.0;"

            "float shadowCalc(vec4 pos)"
            "{"
                "float shadow = 0.0;"
                "vec3 coord = pos.xyz / pos.w;"
                "coord = coord * 0.5 + 0.5;"

                "float current = coord.z;"

                "float close = texture2D(shadowmap, coord.xy).r;"
                "shadow = current - shadow_bias > close ? 1.0 : 0.0;"
                "if(coord.z > 1.0)"
                "   shadow = 0.0;"
                "return shadow;"
            "}"

            "vec3 dirLight(vec3 dir, vec3 nNormal, vec3 viewDir){"
                "float diff = max(dot(nNormal, -dir), 0.0);"
                "vec3 diffuse = diff * vec3( texture2D(texture, TexCoord));"
                "float shadow = 1.0;"
                "shadow_bias = max(0.01 * (1.0 - dot(nNormal, dir)), 0.005);"
                "vec4 fPosLight = projview_shadow * vec4(FragPos, 1.0);"
                "shadow = shadowCalc(fPosLight);"
                "return (1.0 - shadow) * diffuse;"
            "}"

            "void main()"
            "{"
                "vec3 nNormal = normalize(Normal);"
                "vec3 viewDir = normalize(ViewPos - FragPos);"
                "vec3 FragColor = vec3(0.0);"
                "FragColor += dirLight(-normalize(lightDir), nNormal, viewDir);"
                "gl_FragColor = vec4(FragColor, 1.0);"
            "}";

    Shader shadow;
    shadow.vertex_code=
            "#version 100\n"
            "attribute vec3 aPos;"

            "uniform mat4 projview;"
            "uniform mat4 model;"
            "void main()"
            "{"
                "gl_Position = projview * model * vec4(aPos, 1.0);"
            "}";
    shadow.fragment_code=
            "#version 100\n"
            "precision mediump float;"
            "void main()"
            "{"
            "}";

    dengine_shader_create(&shadow);
    dengine_shader_setup(&shadow);

    dengine_shader_create(&shader);
    dengine_shader_setup(&shader);

    dengine_shader_set_int(&shader, "texture", 0);
    dengine_shader_set_int(&shader, "shadowmap", 1);

    DirLight dLight;
    dLight.shadow.enable = 1;
    dLight.shadow.shadow_map_size = 512;
    dLight.position[0] = 2.0f;
    dLight.position[1] = 4.0f;
    dLight.position[2] = -1.0f;
    dengine_lighting_setup_dirlight(&dLight);
    dengine_shader_set_vec3(&shader, "lightDir", dLight.position);

    Texture texture;
    memset(&texture, 0, sizeof(Texture));
    texture.interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
    char* texfile = dengineutils_os_dialog_fileopen("Open a 3/4 channel image(png, jpg)");
    if(!texfile)
    {
        dengineutils_logging_log("ERROR::no file selected!");
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

    //"wirframe" mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    //3d depth
    glEnable(GL_DEPTH_TEST);

    //face culling
    glEnable(GL_CULL_FACE);
    denginegui_init();
    dengine_input_init();
    while(dengine_window_isrunning())
    {
        //FIXME : Break on resize window framebuffer
        dengine_camera_project_perspective((float)w / (float)h, &camera);
        dengine_camera_lookat(target, &camera);
        dengine_camera_apply(&shader, &camera);

        double srcl = dengine_input_get_mousescroll_y();

        if(srcl > 0)
            camera.fov-=1;
        else if (srcl < 0)
            camera.fov+=1;

        glClearColor(1.0, 0.5, 0.3, 1.0);
        //clear depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        dengine_lighting_shadowop_clear(&dLight.shadow);

        glActiveTexture(GL_TEXTURE0);
        dengine_texture_bind(GL_TEXTURE_2D, &texture);

        glActiveTexture(GL_TEXTURE1);
        dengine_texture_bind(GL_TEXTURE_2D, &dLight.shadow.shadow_map.depth);

        mat4 model, ivTpModel;

        dengine_shader_set_mat4(&shader, "projview_shadow", dLight.shadow_projview);

        glm_mat4_identity(model);
        vec3 pos = {0.0, 1.0, 0.0};
        glm_translate(model, pos);
        glm_mat4_inv(model, ivTpModel);
        glm_mat4_transpose(ivTpModel);
        dengine_shader_set_mat4(&shader, "model", model[0]);
        dengine_shader_set_mat4(&shader, "ivTpModel", ivTpModel[0]);
        dengine_draw_primitive(&cube, &shader);
        dengine_lighting_shadow_dirlight_draw(&dLight, &shadow, &cube, model[0]);

        vec3 scale = {5.0, 5.0, 5.0};
        glm_mat4_identity(model);
        glm_scale(model, scale);
        glm_mat4_inv(model, ivTpModel);
        glm_mat4_transpose(ivTpModel);
        dengine_shader_set_mat4(&shader, "model", model[0]);
        dengine_shader_set_mat4(&shader, "ivTpModel", ivTpModel[0]);
        dengine_draw_primitive(&plane, &shader);
        dengine_lighting_shadow_dirlight_draw(&dLight, &shadow, &plane, model[0]);



        float rgba[] = {0.0, 0.0, 0.0, 1.0};
        denginegui_panel(10, 10, 200, 200, &dLight.shadow.shadow_map.depth, NULL, rgba);

        dengine_window_swapbuffers();
        dengine_window_glfw_pollevents();
    }

    dengine_window_terminate();
    return 0;
}
