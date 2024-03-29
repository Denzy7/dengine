#include <string.h>
#include <stdlib.h>

#include <cglm/cglm.h>

#include <dengine/window.h>
#include <dengine/input.h>
#include <dengine/loadgl.h>
#include <dengine/camera.h>
#include <dengine/primitive.h>
#include <dengine/draw.h>
#include <dengine/lighting.h>
#include <dengine/material.h>
#include <dengine/viewport.h>

#include <dengine-utils/filesys.h>
#include <dengine-utils/logging.h>
#include <dengine-utils/os.h>
#include <dengine-utils/rng.h>
#include <dengine-utils/debug.h>

#include <dengine-gui/gui.h>

int main(int argc, char *argv[])
{
    int camscl = 0;

    for(int i = 0; i < argc; i++)
    {
        char* arg = argv[i];
        char* v = argv[i + 1];
        if(arg && v && strstr("-camscl", arg))
        {
            camscl = strtod(v, NULL);
        }
    }


    dengineutils_debug_init();

    DengineWindow* window;
    dengine_window_init();
    window = dengine_window_create(1280, 720, "testdengine-camera-foward", NULL);
    dengine_window_makecurrent(window);
    dengine_window_loadgl(window);
    dengineutils_filesys_init();

    const int prtbf_sz=1024;
    char* prtbf=malloc(prtbf_sz);

    denginegui_init();
    float fontsz=24.0f;
    denginegui_set_font(NULL,fontsz,512);

    dengineutils_rng_set_seedwithtime();

    Shader stdshader;
    dengine_shader_make_standard(DENGINE_SHADER_STANDARD, &stdshader);

    Camera camera;
    dengine_camera_setup(&camera);
    dengine_camera_set_rendermode(DENGINE_CAMERA_RENDER_FOWARD, &camera);

    if(camscl)
    {
        int max;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
        if(camscl * 16 > max)
        {
            dengineutils_logging_log("WARNING::you passed a scale your GPU cannot possibly handle. Try passing %d you madlad!", max / 16);
        }

        dengine_camera_resize(&camera, 16 * camscl, 9 * camscl);
        dengineutils_logging_log("INFO::scaled to %dx%d", camera.render_width, camera.render_height);
    }else
    {
        dengineutils_logging_log("pass a scaler 1 - 100 to scale camera aspect ratio with -camscl <scaler>");
    }

    dengine_camera_apply(&stdshader, &camera);

    Material cube_mat;
    dengine_material_setup(&cube_mat);
    dengine_material_set_shader_color(&stdshader, &cube_mat);

    Primitive cube;
    dengine_primitive_gen_cube(&cube, &stdshader);

    mat4 model;
    vec3 pos;

    DirLight dLight;
    memset(&dLight, 0, sizeof(DirLight));
    dengine_lighting_setup_dirlight(&dLight);

    PointLight pLight;
    memset(&pLight, 0, sizeof(PointLight));
    dengine_lighting_setup_pointlight(&pLight);

    pLight.position[1] = 5.5;
    pLight.light.diffuse[2] = 0;
    pLight.light.specular[2] = 0;

    dengine_lighting_apply_dirlight(&dLight, &stdshader);
    dengine_lighting_apply_pointlight(&pLight,&stdshader);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    while (dengine_window_isrunning(window)) {

        glClearColor(.3, .2, .1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        denginegui_panel(0,0,1280/2,720/2, &camera.framebuffer.color[0], NULL, NULL);
        if(denginegui_button(0,360,200,50,"Dump to fb.jpg",NULL))
        {
            dengine_framebuffer_bind(GL_FRAMEBUFFER,&camera.framebuffer);
            Texture readback;
            dengine_texture_make_canreadback_color(camera.render_width, camera.render_height, &readback);
            dengine_framebuffer_readback(&readback, &camera.framebuffer);
            dengine_texture_writeout("fb.jpg", 1, &readback);

            dengine_texture_free_data(&readback);
            snprintf(prtbf,prtbf_sz,"dumped to %s/fb.jpg",dengineutils_os_get_cwd());
            dengineutils_os_dialog_messagebox("dump success",prtbf,0);

            dengine_framebuffer_bind(GL_FRAMEBUFFER,NULL);
        }

        if(denginegui_button(205,360,200,50,"render",NULL))
        {
            int w, h;
            dengine_viewport_get(NULL, NULL, &w, &h);

            dengine_camera_use(&camera);
            dengine_viewport_set(0, 0, camera.render_width, camera.render_height);

            dengine_material_use(&cube_mat);

            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    glm_vec3_zero(pos);
                    glm_mat4_identity(model);

                    pos[0] = (float)i+(float)i*(float)j;
                    pos[1] = (float)dengineutils_rng_int(10)/8.5;
                    pos[2] = (float)j+(float)i*(float)j;

                    glm_translate(model, pos);
                    dengine_shader_set_mat4(&stdshader, "model", model[0]);

                    dengine_draw_primitive(&cube, &stdshader);
                }
            }

            dengine_material_use(NULL);
            dengine_camera_use(NULL);

            dengine_viewport_set(0, 0, w, h);
        }

        dengine_window_poll(window);
        dengine_window_swapbuffers(window);
    }

    free(prtbf);
    dengine_shader_destroy(&stdshader);

    dengine_material_destroy(&cube_mat);

    denginegui_terminate();

    dengineutils_filesys_terminate();

    dengine_window_destroy(window);
    dengine_window_terminate();
    dengineutils_debug_terminate();

    return 0;
}

