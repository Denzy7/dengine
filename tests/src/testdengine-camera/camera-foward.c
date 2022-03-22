#include <string.h>
#include <stdlib.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <cglm/cglm.h>

#include <dengine/window.h>
#include <dengine/input.h>
#include <dengine/loadgl.h>
#include <dengine/camera.h>
#include <dengine/primitive.h>
#include <dengine/draw.h>
#include <dengine/lighting.h>
#include <dengine/material.h>

#include <dengine-utils/filesys.h>
#include <dengine-utils/logging.h>
#include <dengine-utils/os.h>
#include <dengine-utils/rng.h>
#include <dengine-utils/debug.h>

#include <dengine-gui/gui.h>

int main(int argc, char *argv[])
{
    dengineutils_debug_init();

    Window window;
    dengine_window_init();
    dengine_window_create(1280, 720, "testdengine-camera-foward", &window);
    dengine_window_makecurrent(&window);
    dengine_window_loadgl();
    dengineutils_filesys_init();

    const int prtbf_sz=1024;
    char* prtbf=malloc(prtbf_sz);

    denginegui_init();
    dengine_input_init();
    float fontsz=24.0f;
    denginegui_set_font(NULL,fontsz,512);

    dengineutils_rng_set_seedwithtime();

    Shader* stdshader=dengine_shader_new_shader_standard(DENGINE_SHADER_STANDARD);

    Camera camera;
    dengine_camera_setup(&camera);
    dengine_camera_set_rendermode(DENGINE_CAMERA_RENDER_FOWARD, &camera);
    dengine_camera_apply(stdshader, &camera);

    Material cube_mat;
    dengine_material_setup(&cube_mat);
    dengine_material_set_shader_color(stdshader, &cube_mat);

    Primitive cube;
    dengine_primitive_gen_cube(&cube, stdshader);

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

    dengine_lighting_apply_dirlight(&dLight, stdshader);
    dengine_lighting_apply_pointlight(&pLight,stdshader);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    while (dengine_window_isrunning()) {

        glClearColor(.3, .2, .1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        denginegui_panel(0,0,1280/2,720/2, &camera.framebuffer.color[0], NULL, NULL);
        if(denginegui_button(0,360,200,50,"Dump to fb.jpg",NULL))
        {
            dengine_framebuffer_bind(GL_FRAMEBUFFER,&camera.framebuffer);
            uint8_t* rgb=calloc(1280*720*3,sizeof(uint8_t));
            glFinish();
            glReadPixels(0,0,1280,720,GL_RGB,GL_UNSIGNED_BYTE,rgb);

            stbi_flip_vertically_on_write(1);
            stbi_write_jpg("fb.jpg",1280,720,3,rgb,95);

            free(rgb);

            snprintf(prtbf,prtbf_sz,"dumped to %s/fb.jpg",dengineutils_os_get_cwd());
            dengineutils_os_dialog_messagebox("dump success",prtbf,0);

            dengine_framebuffer_bind(GL_FRAMEBUFFER,NULL);
        }

        if(denginegui_button(205,360,200,50,"render",NULL))
        {
            dengine_camera_use(&camera);
            dengine_material_use(&cube_mat);

            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    glm_vec3_zero(pos);
                    glm_mat4_identity(model);

                    pos[0] = (float)i+(float)i*(float)j;
                    pos[1] = (float)dengineutils_rng_int(10)/8.5;
                    pos[2] = (float)j+(float)i*(float)j;

                    glm_translate(model, pos);
                    dengine_shader_set_mat4(stdshader, "model", model[0]);

                    dengine_draw_primitive(&cube, stdshader);
                }
            }

            dengine_material_use(NULL);
            dengine_camera_use(NULL);
        }

        dengine_input_pollevents();
        dengine_window_swapbuffers();
    }

    free(prtbf);
    dengine_shader_destroy(stdshader);
    free(stdshader);

    dengine_material_destroy(&cube_mat);

    denginegui_terminate();

    dengineutils_filesys_terminate();

    dengineutils_debug_terminate();
    dengine_window_terminate();


    return 0;
}

