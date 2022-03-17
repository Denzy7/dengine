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

#include <dengine-utils/filesys.h>

#include <dengine-gui/gui.h>

int main(int argc, char *argv[])
{
    Window window;
    dengine_window_init();
    dengine_window_create(1280, 720, "testdengine-camera-foward", &window);
    dengine_window_makecurrent(&window);
    dengine_window_loadgl();

    denginegui_init();

    Camera camera;
    dengine_camera_setup(&camera);
    dengine_camera_set_rendermode(DENGINE_CAMERA_RENDER_FOWARD, &camera);

    Shader stdshader;
    dengine_shader_create(&stdshader);

    char* stdshadersrc[2];
    char* stdshadersrcfile[]=
    {
        "/home/denzy/dengine/assets/shaders/standard.vert.glsl",
        "/home/denzy/dengine/assets/shaders/standard.frag.glsl"
    };
    File2Mem f2m;
    for (int i = 0; i < 2; i++) {
        f2m.file =  stdshadersrcfile[i];
        dengineutils_filesys_file2mem_load(&f2m);
        stdshadersrc[i] = strdup(f2m.mem);
        dengineutils_filesys_file2mem_free(&f2m);
    }

    stdshader.vertex_code= stdshadersrc[0];
    stdshader.fragment_code = stdshadersrc[1];

    dengine_shader_setup(&stdshader);

    for (int i = 0; i < 2; i++) {
        free(stdshadersrc[i]);
    }

    Material cube_mat;
    dengine_material_setup(&cube_mat);
    dengine_material_set_shader_color(&stdshader, &cube_mat);

    Primitive cube;
    dengine_primitive_gen_cube(&cube, &stdshader);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    mat4 model;
    vec3 pos;

    DirLight dLight;
    memset(&dLight, 0, sizeof(DirLight));
    dengine_lighting_setup_dirlight(&dLight);
    dLight.light.strength = 1.15;

    while (dengine_window_isrunning()) {
        glClearColor(.3, .2, .1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        denginegui_panel(0,0,1280/2,720/2, &camera.framebuffer.color[0], NULL, NULL);

        dengine_camera_use(&camera);
        dengine_camera_apply(&stdshader, &camera);
        dengine_lighting_apply_dirlight(&dLight, &stdshader);
        dengine_material_use(&cube_mat);
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                glm_mat4_identity(model);
                pos[0] = (float)i+(float)i*(float)j;
                pos[2] = (float)j+(float)i*(float)j;
                glm_translate(model, pos);
                dengine_shader_set_mat4(&stdshader, "model", model[0]);

                dengine_draw_primitive(&cube, &stdshader);
            }
        }

        dengine_material_use(NULL);
        dengine_camera_use(NULL);

        dengine_input_pollevents();
        dengine_window_swapbuffers();
    }

    dengine_window_terminate();
    return 0;
}

