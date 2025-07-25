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
#include <dengine/viewport.h>

#include <dengine-utils/logging.h>
#include <dengine-utils/filesys.h>
#include <dengine-utils/timer.h>
#include <dengine-utils/os.h>
#include <dengine-utils/debug.h>
#include <dengine-utils/macros.h>
#include <dengine-utils/assets.h>
#ifdef DENGINE_ANDROID
#define SWBTNS
#endif

#include <dengine-gui/gui.h>

void draw_axis(Primitive* axis, Shader* shader)
{
    float color[3];
    for (int i = 0; i < 3; i++) {
        color[0] = i == 0 ? 1.0f : 0.0f, color[1] = i == 1 ? 1.0f : 0.0f, color[2] = i == 2 ? 1.0f : 0.0f;
        axis->offset = (void*)(i*2*sizeof (uint16_t));
        dengine_shader_set_vec3(shader, "color", color);
        dengine_draw_primitive(axis, shader);
    }
}

/* for this to be run with dengine glue like applications, we 
 * need to set the window. this is better since we won't have 
 * to rewrite it for android :)))
 */
DengineWindow* window;
void testdengine_lighting_standard_setwindow(DengineWindow* some_window_created_elsewhere)
{
    window = some_window_created_elsewhere;
}

int testdengine_lighting_standard(int argc, char** argv)
{
    float fontsz = denginegui_get_fontsz();

    dengineutils_logging_log("INFO::GL : %s", glGetString(GL_VERSION));
    dengineutils_logging_log("INFO::GLSL : %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

    File2Mem f2m;
    dengineutils_assets_zip_load(NULL);

    //STDSHADER
    Shader stdshader, shadow2d, shadow3d, dftshader;
    dengine_shader_make_standard(DENGINE_SHADER_STANDARD, &stdshader);

    //SHADOW2D
    dengine_shader_make_standard(DENGINE_SHADER_SHADOW2D, &shadow2d);

    //SHADOW3D
    dengine_shader_make_standard(DENGINE_SHADER_SHADOW3D, &shadow3d);

    //DEFAULT
    dengine_shader_make_standard(DENGINE_SHADER_DEFAULT, &dftshader);

    int w, h;
    Camera camera;
    dengine_camera_setup(&camera);

    Primitive plane, cube, quad, axis;
    Material plane_mat, cube_mat;

    dengine_primitive_gen_plane(&plane, &stdshader);
    dengine_primitive_gen_cube(&cube, &stdshader);

    dengine_primitive_gen_axis(&axis, &dftshader);
    axis.index_count = 2;
    dengine_primitive_gen_quad(&quad, &dftshader);

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
        dengineutils_assets_load(planeTextureFile[i], &f2m.mem, &f2m.size);
        dengine_texture_load_mem(f2m.mem, f2m.size, 1, tex_plane);
        free(f2m.mem);
        tex_plane->filter_min = GL_LINEAR;
        tex_plane->filter_mag = GL_LINEAR;
        uint32_t fmt = tex_plane->channels == 3 ? GL_RGB : GL_RGBA;
        tex_plane->format = fmt;
        tex_plane->internal_format = fmt;
        tex_plane->wrap = GL_CLAMP_TO_EDGE;
        tex_plane->type = GL_UNSIGNED_BYTE;

        dengine_texture_data(GL_TEXTURE_2D, tex_plane);
        dengine_texture_set_params(GL_TEXTURE_2D, tex_plane);
        dengine_texture_bind(GL_TEXTURE_2D,NULL);
        dengine_texture_free_data(tex_plane);

        dengine_material_set_texture(tex_plane, shaderSamplers[i], &plane_mat);

        Texture* tex_cube = &cubeTex[i];
        dengine_texture_bind(GL_TEXTURE_2D, tex_cube);
        tex_cube->interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
        dengineutils_assets_load(cubeTextureFile[i], &f2m.mem, &f2m.size);
        dengine_texture_load_mem(f2m.mem, f2m.size, 1, tex_cube);
        free(f2m.mem);
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

    mat4 model;
    vec3 scale = {5,5,5};
    vec3 pos = {0,1,0};

    DirLight dLight;
    memset(&dLight, 0, sizeof(DirLight));
    dLight.shadow.enable = 1;
    dLight.shadow.shadow_map_size = 512;
    dengine_lighting_setup_dirlight(&dLight);
    dLight.shadow.far_shadow = 125.0f;
    dLight.shadow_ortho = 10.0f;
    dLight.light.strength = .5;

    PointLight pLight;
    memset(&pLight, 0, sizeof(PointLight));
    pLight.shadow.enable = 1;
    pLight.shadow.shadow_map_size = 512;
    dengine_lighting_setup_pointlight(&pLight);
    pLight.position[0] = 0.5;
    pLight.position[1] = 5.5;
    pLight.position[2] = -1.5;
    pLight.light.diffuse[2] = 0;
    pLight.light.specular[2] = 0;
    pLight.light.strength = 1.75;

    SpotLight sLight;
    memset(&sLight, 0, sizeof(SpotLight));
    sLight.pointLight.position[0] = 1.f;
    sLight.pointLight.position[1] = 3.f;
    sLight.pointLight.position[2] = .5f;
    dengine_lighting_setup_spotlight(&sLight);
    sLight.pointLight.light.diffuse[1] = 0;
    sLight.pointLight.light.specular[1] = 0;
    sLight.pointLight.light.strength = 3.0f;

    float tgt[3] = {1.5f,0.3f,0.f};

    dengine_material_set_texture(&dLight.shadow.shadow_map.depth, "dLightShadow", &plane_mat);
    dengine_material_set_texture(&dLight.shadow.shadow_map.depth, "dLightShadow", &cube_mat);

    dengine_material_set_texture( &pLight.shadow.shadow_map.depth, "pLightsShadow0", &plane_mat);
    dengine_material_set_texture( &pLight.shadow.shadow_map.depth, "pLightsShadow0", &cube_mat);

    dengine_material_set_texture( &sLight.pointLight.shadow.shadow_map.depth, "sLightsShadow0", &cube_mat);
    dengine_material_set_texture( &sLight.pointLight.shadow.shadow_map.depth, "sLightsShadow0", &plane_mat);

    int use_shadow3d = 1;

    glEnable(GL_DEPTH_TEST);

    //"wirframe" mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    double elapsed = 0.0;

    char fps[20];
    char shadowtogglestr[50];

    int curpstr = 0;
    char* postrs[]=
    {
        "Cube", "Camera", "DirectionalLight (pos)'", "PointLight", "Spotlight", "Spotlight target"
    };

    float* posflt[]=
    {
        pos, camera.position, dLight.direction, pLight.position, sLight.pointLight.position, tgt
    };
    char postr[128];


    vec3 gizmoscl = {.75f, .75f, .75f};
    vec3 up = {.0f, 1.0f, .0f};
    mat4 view;

    glLineWidth(4.0f);
    /*TODO: before we actually get a working lighting system,
     * we set number of dynamic lights manually */
    dengine_shader_set_int(&stdshader, "nr_pointLights", 1);
    dengine_shader_set_int(&stdshader, "nr_spotLights", 1);
    while (dengine_window_isrunning(window)) {
        snprintf(shadowtogglestr, sizeof (shadowtogglestr), "Click to toggle 3D shadows : %d", use_shadow3d);

        dengineutils_timer_update();
        double delta = dengineutils_timer_get_delta();
        elapsed+=delta;
        if (elapsed > 1000) {
            snprintf(fps, sizeof (fps), "FPS : %.1f", 1 / delta * 1000);
            elapsed = 0.0;
        }
        float* ptr = posflt[curpstr];
        if (dengine_input_get_key('A'))
            ptr[0] -=.01 * delta;

        if (dengine_input_get_key('D'))
            ptr[0] +=.01 * delta;

        if (dengine_input_get_key('E'))
            ptr[1] +=.01 * delta;

        if (dengine_input_get_key('C'))
            ptr[1] -=.01 * delta;

        if (dengine_input_get_key('W'))
            ptr[2] -=.01 * delta;

        if (dengine_input_get_key('S'))
            ptr[2] +=.01 * delta;

        dengine_viewport_get(0, 0, &w, &h);

        dengine_camera_lookat(NULL, &camera);
        dengine_camera_project_perspective((float)w / (float)h, &camera);
        dengine_camera_apply(&stdshader, &camera);
        dengine_camera_apply(&dftshader, &camera);

        for(int i = 0; i < 3; i++)
        {
            sLight.direction[i] = sLight.pointLight.position[i] - tgt[i]; 
        }
        if(dengine_input_get_key('L'))
            sLight.innerCutOff += (delta / 1000.0) * 10.0;
        if(dengine_input_get_key('K'))
            sLight.innerCutOff -= (delta / 1000.0) * 10.0;


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
        ;}

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

        //dirLight  quad + axis
        glm_mat4_identity(model);
        glm_mat4_zero(view);
        glm_look(dLight.direction, dLight.direction, up, view);
        glm_mat4_inv(view, view);
        glm_mat4_mul(model, view, model);
        glm_scale(model, gizmoscl);
        dengine_shader_set_mat4(&dftshader, "model", model[0]);
        dengine_shader_set_vec3(&dftshader, "color", dLight.light.diffuse);
        dengine_draw_primitive(&quad, &dftshader);
        draw_axis(&axis,&dftshader);

        //pLight quad + axis
        glm_mat4_identity(model);
        glm_translate(model, pLight.position);
        glm_scale(model, gizmoscl);
        dengine_shader_set_mat4(&dftshader, "model", model[0]);
        dengine_shader_set_vec3(&dftshader, "color", pLight.light.diffuse);
        dengine_draw_primitive(&quad, &dftshader);
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
        dengine_draw_primitive(&quad, &dftshader);
        draw_axis(&axis,&dftshader);

        //sLight target
        glm_mat4_identity(model);
        glm_translate(model, tgt);
        glm_scale(model, gizmoscl);
        dengine_shader_set_mat4(&dftshader, "model", model[0]);
        draw_axis(&axis,&dftshader);

        //GUI PASS
        double offset = fontsz / 4;
        const int tex_widths = 128;

#ifdef SWBTNS
        static const char* swbtns_map[] = 
        {
            "A", "D", "E", "C", "W", "S"
        };
        static const float btnwid = 100.0f, pad = 5.0f;
        denginegui_set_button_repeatable(1);
        for(int i = 0; i < 6; i++)
        {
            if(denginegui_button(
                        fontsz + (i * btnwid) + (i * pad), h / 2.0f ,
                        btnwid, 100.0f, swbtns_map[i], NULL))
            {
                if(i % 2 != 0)
                    ptr[i / 2] += .01f * delta;
                else
                    ptr[i / 2] -= .01f * delta;
            }
        }
        denginegui_set_button_repeatable(0);
#endif

        denginegui_text(offset,offset, (char*) glGetString(GL_VERSION), NULL);
        denginegui_panel(0, fontsz, tex_widths,tex_widths,&dLight.shadow.shadow_map.depth, NULL, NULL);

        int postrs_count = DENGINE_ARY_SZ(postrs);
        snprintf(postr, sizeof (postr), "Click to change pos mode : %s <%d of %d> , x:%.2f, y:%.2f, z:%.2f", postrs[curpstr], curpstr + 1, postrs_count, ptr[0], ptr[1], ptr[2]);
        if(denginegui_button(offset, h - 4 * fontsz, w - 2 * offset, 2 * fontsz, postr, NULL))
        {
            curpstr++;
            if (curpstr >= DENGINE_ARY_SZ(postrs)) {
                curpstr = 0;
            }
        }
        denginegui_text(offset, h - 5 * fontsz, "Use WASD to change pos. EC to up/down", NULL);
        denginegui_text(offset, h - fontsz, fps, NULL);

        if (denginegui_button(offset, h - 8 * fontsz, 550, 2 * fontsz, shadowtogglestr, NULL))
        {
            use_shadow3d =!use_shadow3d;
        }

        dengine_window_swapbuffers(window);
        dengine_window_poll(window);
    }
    dengine_material_destroy(&cube_mat);
    dengine_material_destroy(&plane_mat);

    dengineutils_assets_zip_free();

    return 0;
}
#ifndef DENGINE_ANDROID
int main(int argc, char** argv)
{
    dengineutils_debug_init();

    dengine_window_init();
    dengine_window_request_MSAA(4);

    dengineutils_logging_set_filelogging(1);

    dengine_window_request_GL(3,2,0);
    DengineWindow* window_int_main = dengine_window_create(1280,720,"testdengine-lighting-standard", NULL);
    if(!window_int_main)
    {
        //Use defaults then without shadows
        dengine_window_request_defaultall();
        window_int_main = dengine_window_create(1280, 720, "testdengine-lighting-standard(noshadow)", NULL);
        if(!window_int_main)
        {
            dengineutils_logging_log("ERROR::cannot request an OpenGL 3.0 window!");
            return 1;
        }
    }
    dengine_window_makecurrent(window_int_main);

    //Remove vsync (not recommended)
    //dengine_window_set_swapinterval(0);
    if (!dengine_window_loadgl(window_int_main)) {
        dengineutils_logging_log("ERROR::cannot load gl");
        return 1;
    }

    //Init systems...
    dengineutils_filesys_init();

    denginegui_init();
    denginegui_set_font(NULL, 20.0f, 512);

    testdengine_lighting_standard_setwindow(window_int_main);

    /* main */
    testdengine_lighting_standard(argc, argv);

    dengineutils_filesys_terminate();
    denginegui_terminate();
    dengine_window_destroy(window);
    dengine_window_terminate();

    dengineutils_debug_terminate();

}
#endif
