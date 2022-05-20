#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <cglm/cglm.h>

#include <dengine/window.h>
#include <dengine/input.h>
#include <dengine/loadgl.h>
#include <dengine/primitive.h>
#include <dengine/camera.h>
#include <dengine/draw.h>
#include <dengine/lighting.h>
#include <dengine/material.h>

#include <dengine-utils/logging.h>
#include <dengine-utils/filesys.h>
#include <dengine-utils/timer.h>
#include <dengine-utils/debug.h>
#include <dengine-utils/platform/android.h>
#include <dengine-utils/macros.h>

#include <dengine-gui/gui.h>

double elapsed = 0.0;
static float fontsz = 20.0f;
int use_shadow3d = 1;
int w, h;
int window_init = 0;
Shader stdshader, shadow2d, shadow3d, dftshader;
Primitive plane, cube, aquad, axis;
Material plane_mat, cube_mat;
Camera camera;
DirLight dLight;
PointLight pLight;
SpotLight sLight;
char fps[20];
char shadowtogglestr[50];
int curpstr = 0;
mat4 model;
vec3 scale = {5,5,5};
vec3 pos = {0,1,0};
float tgt[3] = {1.5f,0.3f,0.f};
char* postrs[]=
        {
                "Cube", "Camera", "DirectionalLight (pos)'", "PointLight", "Spotlight", "Spotlight target"
        };

float* posflt[]=
        {
                pos, camera.position, dLight.position, pLight.position, sLight.pointLight.position, tgt
        };
char postr[60];


vec3 gizmoscl = {.75f, .75f, .75f};
vec3 up = {.0f, 1.f, .0f};
mat4 view;

static void draw_axis(Primitive* _axis, Shader* _shader)
{
    float color[3];
    for (int i = 0; i < 3; i++) {
        color[0] = i == 0 ? 1.0f : 0.0f, color[1] = i == 1 ? 1.0f : 0.0f, color[2] = i == 2 ? 1.0f : 0.0f;
        _axis->offset = (void*)(i * 2 * sizeof (uint16_t));
        dengine_shader_set_vec3(_shader, "color", color);
        dengine_draw_primitive(_axis, _shader);
    }
}

static void init(struct android_app* app)
{
    //Acquire win
    ANativeWindow_acquire(app->window);
    dengine_window_request_GL(2, 0, 0);
    dengineutils_debug_init();

    if(dengine_window_init())
    {
		dengine_window_loadgl();

		dengine_window_set_swapinterval(2);

        window_init = 1;

        dengine_window_get_window_width(&w);
        dengine_window_get_window_height(&h);
        dengineutils_logging_log("INFO::init window %dx%d\n", w, h);

        dengineutils_logging_log("INFO::GL : %s\n", glGetString(GL_VERSION));
        dengineutils_logging_log("INFO::GLSL : %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

        denginegui_init();
        denginegui_set_font(NULL, fontsz, 512);
        File2Mem f2m;

        //STDSHDR
        static char *stdshadersrcfile[2]=
        {
            "shaders/standard.vert.glsl",
            "shaders/standard.frag.glsl"
        };

        char *stdshadersrc[2];

        for (int i = 0; i < 2; i++) {
            f2m.file = stdshadersrcfile[i];
            dengineutils_android_asset2file2mem(&f2m);
            stdshadersrc[i] = strdup(f2m.mem);
            dengineutils_filesys_file2mem_free(&f2m);
        }

        stdshader.vertex_code = stdshadersrc[0];
        stdshader.fragment_code = stdshadersrc[1];

        dengine_shader_create(&stdshader);
        dengine_shader_setup(&stdshader);

        for (int i = 0; i < 2; i++) {
            free(stdshadersrc[i]);
        }

        //SHADOW2D
        char *shadow2dsrcfile[2]=
        {
            "shaders/shadow2d.vert.glsl",
            "shaders/shadow2d.frag.glsl"
        };
        char *shadow2dsrc[2];
        for (int i = 0; i < 2; i++) {
            f2m.file = shadow2dsrcfile[i];
            dengineutils_android_asset2file2mem(&f2m);
            shadow2dsrc[i] = strdup(f2m.mem);
            dengineutils_filesys_file2mem_free(&f2m);
        }

        shadow2d.vertex_code = shadow2dsrc[0];
        shadow2d.fragment_code = shadow2dsrc[1];

        dengine_shader_create(&shadow2d);
        dengine_shader_setup(&shadow2d);

        for (int i = 0; i < 2; i++) {
            free(shadow2dsrc[i]);
        }

        //SHADOW3D
        dengine_shader_create(&shadow3d);
        char *shadow3dsrcfile[3]=
        {
            "shaders/shadow3d.vert.glsl",
            "shaders/shadow3d.frag.glsl",
            "shaders/shadow3d.geom.glsl"
        };
        char *shadow3dsrc[3];
        for (int i = 0; i < 3; i++) {
            f2m.file = shadow3dsrcfile[i];
            dengineutils_android_asset2file2mem(&f2m);
            shadow3dsrc[i] = strdup(f2m.mem);
            dengineutils_filesys_file2mem_free(&f2m);
        }

        shadow3d.vertex_code = shadow3dsrc[0];
        shadow3d.fragment_code = shadow3dsrc[1];
        shadow3d.geometry_code = shadow3dsrc[2];


        dengine_shader_setup(&shadow3d);

        for (int i = 0; i < 3; i++) {
            free(shadow3dsrc[i]);
        }

        //SHADOW2D

        //DFTSHDR
        dengine_shader_create(&dftshader);
        char *dftsrcfile[3]=
        {
            "shaders/default.vert.glsl",
            "shaders/default.frag.glsl",
        };
        char *dftsrc[2];
        for (int i = 0; i < 2; i++) {
            f2m.file = dftsrcfile[i];
            dengineutils_android_asset2file2mem(&f2m);
            dftsrc[i] = strdup(f2m.mem);
            dengineutils_filesys_file2mem_free(&f2m);
        }

        dftshader.vertex_code = dftsrc[0];
        dftshader.fragment_code = dftsrc[1];

        dengine_shader_setup(&dftshader);

        for (int i = 0; i < 2; i++) {
            free(dftsrc[i]);
        }

        //TEXTURES
        dengine_primitive_gen_plane(&plane, &stdshader);
        dengine_primitive_gen_cube(&cube, &stdshader);

        dengine_primitive_gen_axis(&axis, &dftshader);
        axis.index_count = 2;
        dengine_primitive_gen_quad(&aquad, &dftshader);

        dengine_material_setup(&plane_mat);
        dengine_material_setup(&cube_mat);

        dengine_material_set_shader_color(&stdshader, &plane_mat);
        dengine_material_set_shader_color(&stdshader, &cube_mat);

        char* shaderSamplers[]=
        {
                "diffuseTex",
                "specularTex"
        };

        char* cubeTextureFile[]=
        {
                "textures/2d/cube_diff.png",
                "textures/2d/cube_spec.png"
        };

        char* planeTextureFile[]=
        {
                "textures/2d/plane_diff.png",
                "textures/2d/plane_spec.png"
        };

        Texture planeTex[2], cubeTex[2];

        memset(planeTex, 0, sizeof (planeTex));
        memset(cubeTex, 0, sizeof (cubeTex));

        dengine_texture_gen(2, planeTex);
        dengine_texture_gen(2, cubeTex);

        for (int i = 0; i < 2; i++)
        {
            Texture* tex_plane = &planeTex[i];
            dengine_texture_bind(GL_TEXTURE_2D, tex_plane);
            tex_plane->interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
            f2m.file = planeTextureFile[i];
            dengineutils_android_asset2file2mem(&f2m);
            dengine_texture_load_mem(f2m.mem, f2m.size, 1, tex_plane);
            dengineutils_filesys_file2mem_free(&f2m);
            tex_plane->filter_min = GL_LINEAR;
            tex_plane->filter_mag = GL_LINEAR;
            tex_plane->wrap = GL_CLAMP_TO_EDGE;
            uint32_t fmt = tex_plane->channels == 3 ? GL_RGB : GL_RGBA;
            tex_plane->format = fmt;
            tex_plane->internal_format = fmt;
            tex_plane->type = GL_UNSIGNED_BYTE;

            dengine_texture_data(GL_TEXTURE_2D, tex_plane);
            dengine_texture_set_params(GL_TEXTURE_2D, tex_plane);
            dengine_texture_bind(GL_TEXTURE_2D,NULL);
            dengine_texture_free_data(tex_plane);

            dengine_material_set_texture( tex_plane, shaderSamplers[i], &plane_mat);

            Texture* tex_cube = &cubeTex[i];
            dengine_texture_bind(GL_TEXTURE_2D, tex_cube);
            tex_cube->interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
            f2m.file = cubeTextureFile[i];
            dengineutils_android_asset2file2mem(&f2m);
            dengine_texture_load_mem(f2m.mem, f2m.size, 1, tex_cube);
            dengineutils_filesys_file2mem_free(&f2m);
            tex_cube->filter_min = GL_LINEAR;
            tex_cube->filter_mag = GL_LINEAR;
            tex_cube->wrap = GL_CLAMP_TO_EDGE;
            fmt = tex_cube->channels == 3 ? GL_RGB : GL_RGBA;
            tex_cube->format = fmt;
            tex_cube->internal_format = fmt;
            tex_cube->type = GL_UNSIGNED_BYTE;

            dengine_texture_data(GL_TEXTURE_2D, tex_cube);
            dengine_texture_set_params(GL_TEXTURE_2D, tex_cube);
            dengine_texture_bind(GL_TEXTURE_2D,NULL);
            dengine_texture_free_data(tex_cube);

            dengine_material_set_texture(tex_cube, shaderSamplers[i], &cube_mat);
        }

        //CAMERA AND 3D
        dengine_camera_setup(&camera);

        //LIGHTING
        memset(&dLight, 0, sizeof(DirLight));
        dLight.shadow.enable = 1;
        dLight.shadow.shadow_map_size = 512;
        dengine_lighting_setup_dirlight(&dLight);
        dLight.light.strength = .5f;

        memset(&pLight, 0, sizeof(PointLight));
//        pLight.shadow.enable = 1;
//        pLight.shadow.shadow_map_size = 512;
        dengine_lighting_setup_pointlight(&pLight);
        pLight.position[0] = 0.5f;
        pLight.position[1] = 5.5f;
        pLight.position[2] = -1.5f;
        pLight.light.diffuse[2] = 0;
        pLight.light.specular[2] = 0;
        pLight.light.strength = 1.75f;

        memset(&sLight, 0, sizeof(SpotLight));
//        sLight.pointLight.shadow.enable = 1;
//        sLight.pointLight.shadow.shadow_map_size = 512;
        sLight.pointLight.position[0] = 1.f;
        sLight.pointLight.position[1] = 3.f;
        sLight.pointLight.position[2] = .5f;
        dengine_lighting_setup_spotlight(&sLight);
        sLight.pointLight.light.diffuse[1] = 0;
        sLight.pointLight.light.specular[1] = 0;
        sLight.pointLight.light.strength = 3.0f;

        dengine_material_set_texture(&dLight.shadow.shadow_map.depth, "dLightShadow", &plane_mat);
        dengine_material_set_texture(&dLight.shadow.shadow_map.depth, "dLightShadow", &cube_mat);

        dengine_material_set_texture( &pLight.shadow.shadow_map.depth, "pLightsShadow0", &plane_mat);
        dengine_material_set_texture( &pLight.shadow.shadow_map.depth, "pLightsShadow0", &cube_mat);

        dengine_material_set_texture( &sLight.pointLight.shadow.shadow_map.depth, "sLightsShadow0", &cube_mat);
        dengine_material_set_texture( &sLight.pointLight.shadow.shadow_map.depth, "sLightsShadow0", &plane_mat);

        glEnable(GL_DEPTH_TEST);

        //"wirframe" mode
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glLineWidth(4.0f);
    }
}

static void term(struct  android_app* app)
{
    dengineutils_debug_terminate();
    dengine_material_destroy(&cube_mat);
    dengine_material_destroy(&plane_mat);

    window_init = 0;
    dengineutils_logging_log("ptr : %p", app->window);
    dengine_window_terminate();
    ANativeWindow_release(app->window);
    ANativeActivity_finish(app->activity);
}

static void draw()
{
    snprintf(shadowtogglestr, sizeof (shadowtogglestr), "Click to toggle 3D shadows : %d", use_shadow3d);

    dengineutils_timer_update();
    double delta = dengineutils_timer_get_delta();
    elapsed+=delta;
    if (elapsed > 1000) {
        snprintf(fps, sizeof (fps), "FPS : %.1f", 1 / delta * 1000);
        elapsed = 0.0;
    }
    float* ptr = posflt[curpstr];

    dengine_window_get_window_dim(&w,&h);

    dengine_camera_lookat(NULL, &camera);
    dengine_camera_project_perspective((float)w / (float)h, &camera);
    dengine_camera_apply(&stdshader, &camera);
    dengine_camera_apply(&dftshader, &camera);

    sLight.direction[0] = (tgt[0] - sLight.pointLight.position[0]);
    sLight.direction[1] = (tgt[1] - sLight.pointLight.position[1]);
    sLight.direction[2] = (tgt[2] - sLight.pointLight.position[2]);

    dengine_lighting_apply_dirlight(&dLight, &stdshader);
    dengine_lighting_apply_pointlight(&pLight, &stdshader);
    dengine_lighting_apply_spotlight(&sLight, &stdshader);

    glClearColor(.1, .1, .1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    dengine_lighting_shadowop_clear(&pLight.shadow);
    dengine_lighting_shadowop_clear(&dLight.shadow);
    dengine_lighting_shadowop_clear(&sLight.pointLight.shadow);

    //Cull face for drawing  regular
    glEnable(GL_CULL_FACE);

    //Do shadow
    glm_mat4_identity(model);
    glm_translate(model, pos);
    dengine_lighting_shadow_dirlight_draw(&dLight, &shadow2d, &cube, model[0]);
    if(use_shadow3d)
    {
        dengine_lighting_shadow_pointlight_draw(&pLight, &shadow3d, &cube, model[0]);
        dengine_lighting_shadow_spotlight_draw(&sLight, &shadow3d, &cube, model[0]);
    }

    glm_mat4_identity(model);
    glm_scale(model, scale);
    dengine_lighting_shadow_dirlight_draw(&dLight, &shadow2d, &plane, model[0]);
    if(use_shadow3d)
    {
        dengine_lighting_shadow_pointlight_draw(&pLight, &shadow3d, &plane, model[0]);
        dengine_lighting_shadow_spotlight_draw(&sLight, &shadow3d, &plane, model[0]);
    }

    //Do color
    dengine_material_use(&cube_mat);
    glm_mat4_identity(model);
    glm_translate(model, pos);
    dengine_shader_set_mat4(&stdshader, "model", model[0]);
    dengine_draw_primitive(&cube, &stdshader);

    dengine_material_use(&plane_mat);
    glm_mat4_identity(model);
    glm_scale(model, scale);
    dengine_shader_set_mat4(&stdshader, "model", model[0]);
    dengine_draw_primitive(&plane, &stdshader);

    dengine_material_use(NULL);

    //GIZMO pass
    //Cull face for drawing  regular
    glDisable(GL_CULL_FACE);

    //pLight quad + axis
    glm_mat4_identity(model);
    glm_translate(model, pLight.position);
    glm_scale(model, gizmoscl);
    dengine_shader_set_mat4(&dftshader, "model", model[0]);
    dengine_shader_set_vec3(&dftshader, "color", pLight.light.diffuse);
    dengine_draw_primitive(&aquad, &dftshader);
    draw_axis(&axis,&dftshader);

    //sLight quad + axis
    glm_mat4_identity(model);
    glm_mat4_zero(view);
    glm_look(sLight.pointLight.position, sLight.direction, up, view);
    glm_mat4_inv(view, view);
    glm_mat4_mul(model, view, model);
    glm_scale(model, gizmoscl);
    dengine_shader_set_mat4(&dftshader, "model", model[0]);
    dengine_shader_set_vec3(&dftshader, "color", sLight.pointLight.light.diffuse);
    dengine_draw_primitive(&aquad, &dftshader);
    draw_axis(&axis,&dftshader);

    //sLight target
    glm_mat4_identity(model);
    glm_translate(model, tgt);
    glm_scale(model, gizmoscl);
    dengine_shader_set_mat4(&dftshader, "model", model[0]);
    draw_axis(&axis,&dftshader);

    //GUI PASS
    float offset = fontsz / 4;
    const int tex_widths = 128;

    denginegui_text(offset,offset, (char*) glGetString(GL_VERSION), NULL);
    denginegui_panel(0, fontsz, tex_widths,tex_widths,&dLight.shadow.shadow_map.depth, NULL, NULL);

    int postrs_count = DENGINE_ARY_SZ(postrs);
    snprintf(postr, sizeof (postr), "Click to change pos mode : %s <%d of %d>", postrs[curpstr], curpstr + 1, postrs_count);
    if(denginegui_button(offset, h - 4 * fontsz, w - 2 * offset, 2 * fontsz, postr, NULL))
    {
        curpstr++;
        if (curpstr >= DENGINE_ARY_SZ(postrs)) {
            curpstr = 0;
        }
    }
    denginegui_text(offset,  (float)h - 5 * fontsz, "Use WASD to change pos. EC to up/down", NULL);
    denginegui_text(offset, (float)h - fontsz, fps, NULL);


    const int wid = 75;
    const float wid_off = 5.0f;

    if (denginegui_button(offset, (float)h - 8 * fontsz, 550, 2 * fontsz, shadowtogglestr, NULL))
    {
        use_shadow3d =!use_shadow3d;
        dengineutils_logging_log("Disable shadow");
    }

    if (denginegui_button(offset, (float)h - 11 * fontsz, wid, 2 * fontsz, "FWD", NULL))
    {
        ptr[2] -=.01f * delta;
    }

    if (denginegui_button(offset + wid, (float)h - 11 * fontsz, wid, 2 * fontsz, "BAK", NULL))
    {
        ptr[2] +=.01f * delta;
    }

    if (denginegui_button(offset + 2 * wid + 2 * wid_off, (float)h - 11 * fontsz, wid, 2 * fontsz, "L", NULL))
    {
        ptr[0] -=.01f * delta;
    }

    if (denginegui_button(offset + 3 * wid + 3 * wid_off, (float)h - 11 * fontsz, wid, 2 * fontsz, "R", NULL))
    {
        ptr[0] +=.01f * delta;
    }

    if (denginegui_button(offset + 4 * wid + 4 * wid_off, (float)h - 11 * fontsz, wid, 2 * fontsz, "UP", NULL))
    {
        ptr[1] +=.01f * delta;
    }

    if (denginegui_button(offset + 5 * wid + 5 * wid_off, (float)h - 11 * fontsz, wid, 2 * fontsz, "DOWN", NULL))
    {
        ptr[1] -=.01f * delta;
    }

    dengine_window_swapbuffers();
    dengine_input_pollevents();
}

void android_main(struct android_app* state)
{
    dengineutils_android_set_app(state);
    dengineutils_android_set_initfunc(init);
    dengineutils_android_set_terminatefunc(term);

    if(state->savedState)
    {
        dengineutils_logging_log("Restoring from save state");
    }

    while(1)
    {
        dengineutils_android_pollevents();
		
        //Quit and detach
        if(state->destroyRequested != 0)
        {
            dengineutils_logging_log("Destroy Requested");
            dengineutils_logging_log("Goodbye!");
            return;
        }

        if (window_init)
            draw();
    }
}
