#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <dengine-scene/ecs.h>
#include <dengine-scene/scene.h>

#include <dengine-utils/logging.h>
#include <dengine-utils/timer.h>
#include <dengine-utils/filesys.h>
#include <dengine-utils/os.h>
#include <dengine-utils/macros.h>
#include <dengine-utils/rng.h>

#include <dengine/input.h>
#include <dengine/window.h>
#include <dengine/draw.h>
#include <dengine/camera.h>
#include <dengine/loadgl.h>
#include <dengine-gui/gui.h>

#include <dengine-model/model.h>

#include <dengine-script/script.h>

//void prt (Entity* ent)
//{
//    dengineutils_logging_log("INFO::prt parent %u. children %zu", ent->entity_id, ent->children_count);
//    size_t children_count = ent->children_count;

//    for (size_t i = 0; i < children_count; i++) {
//        Entity* child = ent->children[i];
//        dengineutils_logging_log("WARNING::prt child %u. parent %u", child->entity_id, child->parent->entity_id);
//        prt(child);
//    }
//}

int main(int argc, char *argv[])
{
    int camscl = 0;
    int usensl = 0;

    for(int i = 0; i < argc; i++)
    {
        char* arg = argv[i];
        char* v = argv[i + 1];
        if(arg && v && strstr("-camscl", arg))
        {
            camscl = strtod(v, NULL);
        }

        if(arg && strstr("-usensl", arg))
        {
            usensl = 1;
        }
    }

    DengineWindow* window;
    dengine_window_init();
    window = dengine_window_create(1280,720,"testdengine-scene-ecs",NULL);
    dengine_window_makecurrent(window);
    dengine_window_loadgl(window);

    dengineutils_filesys_init();

    NSL nsl = NULL;

    if(!usensl)
    {
        if(!denginescript_init())
        {
            usensl = 1;
        }else
        {
            dengineutils_logging_log("you can pass -usensl to use native scripting library alternative");
        }
    }


    if(usensl)
    {
        nsl = denginescript_nsl_load("./nsl-test.nsl");
        if(!nsl)
        {
            dengineutils_logging_log("ERROR::nsl-test.nsl not found. place it next to executable");
            return -1;
        }
        dengineutils_logging_log("INFO::NSL loaded!");
    }

    Entity* ent1 = denginescene_ecs_new_entity();
    Entity* ent2 = denginescene_ecs_new_entity();
    Entity* ent3 = denginescene_ecs_new_entity();
    Entity* ent4 = denginescene_ecs_new_entity();
    Entity* ent5 = denginescene_ecs_new_entity();
    Entity* ent6 = denginescene_ecs_new_entity();
    Entity* ent7 = denginescene_ecs_new_entity();
    Entity* ent8 = denginescene_ecs_new_entity();
    Entity* ent9 = denginescene_ecs_new_entity();

    Entity* ent10 = denginescene_ecs_new_entity();
    Entity* ent11 = denginescene_ecs_new_entity();
    Entity* ent12 = denginescene_ecs_new_entity();

    Entity* ent13 = denginescene_ecs_new_entity();

    Entity* ent14 = denginescene_ecs_new_entity();

    denginescene_ecs_parent(ent6, ent7);
    denginescene_ecs_parent(ent6, ent8);
    denginescene_ecs_parent(ent6, ent9);
    denginescene_ecs_parent(ent4, ent5);
    denginescene_ecs_parent(ent4, ent6);
    denginescene_ecs_parent(ent2, ent3);
    denginescene_ecs_parent(ent2, ent4);

    denginescene_ecs_parent(ent10, ent11);
    denginescene_ecs_parent(ent10, ent12);

    Camera cam;
    dengine_camera_setup(&cam);
    dengine_camera_set_rendermode(DENGINE_CAMERA_RENDER_FOWARD,&cam);

    if(camscl)
    {
        int max;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
        if(camscl * 16 > max)
        {
            dengineutils_logging_log("WARNING::you passed a scale your GPU cannot possibly handle. Try passing %d you madlad!", max / 16);
        }

        dengine_camera_resize(&cam, 16 * camscl, 9 * camscl);
        dengineutils_logging_log("INFO::scaled to %dx%d", cam.render_width, cam.render_height);
    }else
    {
        dengineutils_logging_log("pass a scaler 1 - 100 to scale camera aspect ratio with -camscl <scaler>");
    }

    cam.clearcolor[0] = 0.2f;
    cam.clearcolor[1] = 0.2f;
    cam.clearcolor[2] = 0.2f;

    CameraComponent* camc = denginescene_ecs_new_cameracomponent(&cam);
    ent13->camera_component=camc;

    Shader stdshdr;
    Shader dftshdr;
    Shader shadow2d;
    Shader skycube;

    dengine_shader_make_standard(DENGINE_SHADER_STANDARD, &stdshdr);
    dengine_shader_make_standard(DENGINE_SHADER_DEFAULT, &dftshdr);
    dengine_shader_make_standard(DENGINE_SHADER_SHADOW2D, &shadow2d);
    dengine_shader_make_standard(DENGINE_SHADER_SKYBOXCUBE, &skycube);

    Material cube_mat,duck_mat,dft_mat, skymat;

    dengine_material_setup(&cube_mat);
    dengine_material_setup(&duck_mat);
    dengine_material_setup(&dft_mat);
    dengine_material_setup(&skymat);

    dengine_material_set_shader_color(&stdshdr,&cube_mat);
    dengine_material_set_shader_color(&stdshdr,&duck_mat);
    dengine_material_set_shader_color(&dftshdr,&dft_mat);
    dengine_material_set_shader_color(&skycube,&skymat);

    dengine_material_set_shader_shadow(&shadow2d, &cube_mat);
    dengine_material_set_shader_shadow(&shadow2d, &duck_mat);

    Primitive cube,plane,grid;
    dengine_primitive_gen_cube(&cube,&stdshdr);
    dengine_primitive_gen_plane(&plane,&stdshdr);
    dengine_primitive_gen_grid(10,&grid,&dftshdr);

    const int prtbf_sz=2048;
    char* prtbf=malloc(prtbf_sz);
    File2Mem f2m;

    snprintf(prtbf,prtbf_sz,"%s/models/duck.obj",dengineutils_filesys_get_assetsdir());
    Primitive* duck = denginemodel_load_file(DENGINE_MODEL_FORMAT_OBJ,prtbf,NULL,&stdshdr);

    Script duckscript;
    if(usensl)
    {
        denginescript_nsl_get_script("moveduck", &duckscript, nsl);
    }else
    {
#ifdef DENGINE_SCRIPTING_PYTHON
    const char* duckscriptfile = "scripts/moveduck.py";
    snprintf(prtbf, prtbf_sz, "%s/%s", dengineutils_filesys_get_assetsdir(), duckscriptfile);
    f2m.file = prtbf;
    dengineutils_filesys_file2mem_load(&f2m);
    denginescript_python_compile(f2m.mem, duckscriptfile, &duckscript);
    dengineutils_filesys_file2mem_free(&f2m);
#endif
    }
    denginescene_ecs_add_script(ent3, &duckscript);
    //note the same script can be added to other entities for the same effects:
    //denginescene_ecs_add_script(ent1, &duckscript);
    //denginescene_ecs_add_script(ent2, &duckscript);

    Script pingpongscale;
    if(usensl)
    {
        denginescript_nsl_get_script("pingpongscale", &pingpongscale, nsl);
    }else
    {
    #ifdef DENGINE_SCRIPTING_PYTHON
        char* pingpongscalefile = "scripts/pingpongscale.py";
        snprintf(prtbf, prtbf_sz, "%s/%s", dengineutils_filesys_get_assetsdir(), pingpongscalefile);
        f2m.file = prtbf;
        dengineutils_filesys_file2mem_load(&f2m);
        denginescript_python_compile(f2m.mem, pingpongscalefile, &pingpongscale);
        dengineutils_filesys_file2mem_free(&f2m);
    #endif
    }
    denginescene_ecs_add_script(ent7, &pingpongscale);

    //load separated planes
    snprintf(prtbf,prtbf_sz,"%s/models/sperated-planes.obj",dengineutils_filesys_get_assetsdir());
    size_t n_planes = 0;
    Primitive* sep_planes = denginemodel_load_file(DENGINE_MODEL_FORMAT_OBJ,prtbf,&n_planes,&stdshdr);

    //Entity** child_sep = NULL;
    Entity* ent15 = denginescene_ecs_new_entity();

    Material sep_planes_mat;
    dengine_material_setup(&sep_planes_mat);

    dengine_material_set_shader_color(&stdshdr,&sep_planes_mat);
    dengine_material_set_shader_shadow(&shadow2d, &sep_planes_mat);

    Texture sep_plane_tex;
    memset(&sep_plane_tex,0,sizeof (Texture));
    sep_plane_tex.auto_dataonload=1;
    sep_plane_tex.interface=DENGINE_TEXTURE_INTERFACE_8_BIT;

    snprintf(prtbf,prtbf_sz,"%s/textures/2d/cube_diff.png",dengineutils_filesys_get_assetsdir());
    dengine_texture_load_file(prtbf,1,&sep_plane_tex);

    dengine_material_set_texture(&sep_plane_tex,"diffuseTex",&sep_planes_mat);

    if(n_planes)
    {
        //child_sep = denginescene_ecs_new_entity_n(n_planes);
        for(size_t i = 0; i < n_planes; i++)
        {
            Entity* ent = denginescene_ecs_new_entity();
            if(ent)
            {
                ent->transform.position[1] = (float)i / (float)n_planes;
                ent->transform.rotation[1] = i * 10.0f;

                MeshComponent* sep_plane_mesh = denginescene_ecs_new_meshcomponent(&sep_planes[i], &sep_planes_mat);
                ent->mesh_component = sep_plane_mesh;
                denginescene_ecs_parent(ent15, ent);
            }
        }
    }
    Script constantrotation;
    if(usensl)
    {
        denginescript_nsl_get_script("constantrotation", &constantrotation, nsl);
    }else
    {
#ifdef DENGINE_SCRIPTING_PYTHON
        char* contantrotationfile = "scripts/constantrotation.py";
        snprintf(prtbf, prtbf_sz, "%s/%s", dengineutils_filesys_get_assetsdir(), contantrotationfile);
        f2m.file = prtbf;
        dengineutils_filesys_file2mem_load(&f2m);
        denginescript_python_compile(f2m.mem, contantrotationfile, &constantrotation);
        dengineutils_filesys_file2mem_free(&f2m);
#endif
    }
    denginescene_ecs_add_script(ent15, &constantrotation);

    DirLight dLight;
    memset(&dLight,0,sizeof (DirLight));
    dLight.shadow.enable = 1;
    dLight.shadow.shadow_map_size = 1024;
    dengine_lighting_setup_dirlight(&dLight);
    dLight.light.strength=0.2f;

    PointLight pLight;
    memset(&pLight,0,sizeof (PointLight));
    dengine_lighting_setup_pointlight(&pLight);
    pLight.light.diffuse[2] = 0;
    pLight.light.specular[2] = 0;
    pLight.light.strength=0.95f;

    LightComponent* dlight_comp = denginescene_ecs_new_lightcomponent(DENGINE_LIGHT_DIR,
                                                                      &dLight);

    LightComponent* plight_comp = denginescene_ecs_new_lightcomponent(DENGINE_LIGHT_POINT,
                                                                      &pLight);



    Entity* ent_plight = denginescene_ecs_new_entity();
    ent_plight->transform.position[0] = 5.5f;
    ent_plight->transform.position[1] = 5.5f;
    ent_plight->light_component = plight_comp;

    Entity* ent_dlight = denginescene_ecs_new_entity();
    ent_dlight->light_component = dlight_comp;
    ent_dlight->transform.position[0] = 7.1f;
    ent_dlight->transform.position[1] = 7.1f;
    ent_dlight->transform.position[2] = 3.1f;

    Script pingpongposition;
    if(usensl)
    {
        denginescript_nsl_get_script("pingpongposition", &pingpongposition, nsl);
    }else
    {
#ifdef DENGINE_SCRIPTING_PYTHON
         char* pingpongpositionfile = "scripts/pingpongposition.py";
        snprintf(prtbf, prtbf_sz, "%s/%s", dengineutils_filesys_get_assetsdir(), pingpongpositionfile);
        f2m.file = prtbf;
        dengineutils_filesys_file2mem_load(&f2m);
        denginescript_python_compile(f2m.mem, pingpongpositionfile, &pingpongposition);
        dengineutils_filesys_file2mem_free(&f2m);
#endif
    }
    denginescene_ecs_add_script(ent_plight, &pingpongposition);

    Texture duck_tex;
    memset(&duck_tex,0,sizeof (Texture));
    duck_tex.auto_dataonload=1;
    duck_tex.interface=DENGINE_TEXTURE_INTERFACE_8_BIT;

    snprintf(prtbf,prtbf_sz,"%s/textures/2d/duck.png",dengineutils_filesys_get_assetsdir());
    dengine_texture_load_file(prtbf,1,&duck_tex);

    dengine_material_set_texture(&duck_tex,"diffuseTex",&duck_mat);
    ent3->mesh_component = denginescene_ecs_new_meshcomponent(duck, &duck_mat);

    dengineutils_rng_set_seedwithtime();
    //feel free to increase pool
    Texture cube_tex_pool[6];
    for(int i = 0; i < DENGINE_ARY_SZ(cube_tex_pool); i++)
    {
        Texture* cube_tex = &cube_tex_pool[i];
        memset(cube_tex, 0, sizeof(Texture));
        float col[] = { (float)dengineutils_rng_int(255) / 255.,
                          (float)dengineutils_rng_int(255) / 255.,
                          (float)dengineutils_rng_int(255) / 255.};
        //simple color
        //dengine_texture_make_color(8, 8, col, 3, cube_tex);

        //fancy checkerboard
        float bg[] = {255. - col[2], 255. - col[1], 255. - col[0]};
        dengine_texture_make_checkerboard(8, 8, 2, col, bg, 0, 3, cube_tex);
    }
    ent1->mesh_component=denginescene_ecs_new_meshcomponent(&plane, &cube_mat);
    dengine_material_set_texture(&cube_tex_pool[dengineutils_rng_int(DENGINE_ARY_SZ(cube_tex_pool))],"diffuseTex",ent1->mesh_component->material);
    ent6->mesh_component=denginescene_ecs_new_meshcomponent(&cube, &cube_mat);
    dengine_material_set_texture(&cube_tex_pool[dengineutils_rng_int(DENGINE_ARY_SZ(cube_tex_pool))],"diffuseTex",ent6->mesh_component->material);
    ent7->mesh_component=denginescene_ecs_new_meshcomponent(&cube, &cube_mat);
    dengine_material_set_texture(&cube_tex_pool[dengineutils_rng_int(DENGINE_ARY_SZ(cube_tex_pool))],"diffuseTex",ent7->mesh_component->material);
    ent14->mesh_component=denginescene_ecs_new_meshcomponent(&grid, &dft_mat);
    /*
     *            SCENE -- 13(camera),14=grid
     *            |    |
     *            1    10
     *                 / \
     *                11 12
     *           ...
     *            2
     *           / \
     *          3   4
     *            / \
     *           5   6
     *             / | \
     *            7  8 9
     */

//    prt(ent1);
    vec3 p={6.1f,6.18f,5.4f};
    memcpy(ent13->transform.position,p,sizeof (vec3));
    p[0] = -45.0f, p[1] = 225.0f, p[2] = 0.0f;
    memcpy(ent13->transform.rotation,p,sizeof (vec3));

    p[0]=1.3f,p[1]=2.9f,p[2]=1.0f;
    memcpy(ent6->transform.position,p,sizeof (vec3));

    p[0]=2.0f,p[1]=-2.0f,p[2]=1.0f;
    memcpy(ent7->transform.position,p,sizeof (vec3));

    p[0]=5.0f,p[1]=5.0f,p[2]=5.0f;
    memcpy(ent1->transform.scale,p,sizeof (vec3));

    p[0]=4.0f,p[1]=2.0f,p[2]=1.0f;
    memcpy(ent3->transform.position,p,sizeof (vec3));

    p[0]=10.0f,p[1]=10.0f,p[2]=10.0f;
    memcpy(ent14->transform.scale,p,sizeof (vec3));

    p[0]=3.0f,p[1]=3.0f,p[2]=3.0f;
    memcpy(ent15->transform.position,p,sizeof (vec3));

    ent3->transform.rotation[1]=45.f;

    ent6->transform.rotation[1]=5.f;

    Scene* scene = denginescene_new();

    denginescene_add_entity(scene, ent1);
    denginescene_add_entity(scene, ent2);
    denginescene_add_entity(scene, ent10);
    denginescene_add_entity(scene, ent13);
    denginescene_add_entity(scene, ent14);
    denginescene_add_entity(scene, ent15);

    denginescene_add_entity(scene, ent_plight);
    denginescene_add_entity(scene, ent_dlight);

    denginegui_init();
    static float fontsz=18.0f;
    denginegui_set_font(NULL,fontsz,512);
    char fpsstr[30];

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    float viewportcol[4]={.0,.0,.0,1.};
    vec4 yellow = {1.0, 1.0, 0.0, 1.0};

    Texture cubemap;
    memset(&cubemap, 0, sizeof(Texture));
    cubemap.type = GL_UNSIGNED_BYTE;
    cubemap.filter_min = GL_LINEAR;
    cubemap.interface = DENGINE_TEXTURE_INTERFACE_8_BIT;

    dengine_texture_gen(1, &cubemap);
    dengine_texture_bind(GL_TEXTURE_CUBE_MAP, &cubemap);
    for(int i = 0; i < 6; i++)
    {
        snprintf(prtbf, prtbf_sz, "%s/textures/cubemaps/sea/sea%d.jpg",
                 dengineutils_filesys_get_assetsdir(),
                 i + 1);
        dengine_texture_load_file(prtbf, 0, &cubemap);
        cubemap.format = cubemap.channels == 3 ? GL_RGB : GL_RGBA;
        cubemap.internal_format = cubemap.format;
        dengine_texture_data(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, &cubemap);
        dengine_texture_free_data(&cubemap);
    }
    dengine_texture_set_params(GL_TEXTURE_CUBE_MAP, &cubemap);
    dengine_texture_bind(GL_TEXTURE_CUBE_MAP, NULL);

    dengine_material_set_texture(&cubemap, "cubemap", &skymat);

    Skybox* sky = denginescene_new_skybox(&cube, &skymat);
    scene->skybox = sky;

    int poly = 1;

    vec3 gridcolor = {0.0f, 1.0f, 0.0f};
    dengine_shader_set_vec3(&dftshdr, "color", gridcolor);

    denginescene_ecs_do_script_scene(scene, DENGINE_SCRIPT_FUNC_START);

    while (dengine_window_isrunning(window)) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE + poly);

        dengine_window_poll(window);

        static double elapsed=9999.0;

        dengineutils_timer_update();
        double delta = dengineutils_timer_get_delta();

        if(dengine_input_get_key_once('F'))
            poly = !poly;

        if(dengine_input_get_key_once('T'))
        {
            Texture rd;
            dengine_texture_make_canreadback_color(cam.render_width, cam.render_height, &rd);
            dengine_framebuffer_readback(&rd, &cam.framebuffer);
            if(dengine_texture_writeout("fb.jpg", 1, &rd))
                dengineutils_os_dialog_messagebox( "screenshot successful","write to fb.jpg", 0);
            dengine_texture_free_data(&rd);
        }

        if(dengine_input_get_key_once('G'))
        {
            if(glIsEnabled(GL_CULL_FACE))
                glDisable(GL_CULL_FACE);
            else
                glEnable(GL_CULL_FACE);
        }

        elapsed+=delta;
        if(elapsed>1000.0)
        {
            snprintf(fpsstr,sizeof (fpsstr),"FPS : %.1f (%.2fms)",1/delta*1000,delta);
            elapsed=0.0;
        }

        denginescene_update(scene);

        glClearColor(0.1,0.1,0.1,1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        denginegui_panel(0,0,854,480,&cam.framebuffer.color[0],NULL,viewportcol);
        denginegui_text(10,10,fpsstr, yellow);

        denginegui_text(0, 480 - fontsz, "USE WASD, EC TO MOVE DUCKAROO!", NULL);

        denginegui_text(0, 480 - 2 * fontsz, "USE ZX TO ROTATE DUCKAROO!", NULL);

        snprintf(prtbf, prtbf_sz, "PRESS F TO SWITCH POLYGON MODE : %d", poly);
        denginegui_text(0, 480 - 3 * fontsz, prtbf, NULL);

        snprintf(prtbf, prtbf_sz, "PRESS G TO SWITCH FACE CULLING (Note FPS change) : %d", (int)glIsEnabled(GL_CULL_FACE));
        denginegui_text(0, 480 - 4 * fontsz, prtbf, NULL);

        denginegui_text(0, 480 - 5 * fontsz, "PRESS T TO TAKE A \"SCREENSHOT\"", NULL);

        denginegui_panel(0, 480, 720 - 480, 720 - 480, &dLight.shadow.shadow_map.depth, NULL, NULL);

        dengine_window_swapbuffers(window);
    }

    denginescene_destroy(scene);

    dengine_material_destroy(&cube_mat);
    dengine_material_destroy(&duck_mat);
    dengine_material_destroy(&dft_mat);
    dengine_material_destroy(&sep_planes_mat);
    dengine_material_destroy(&skymat);

    dengine_shader_destroy(&stdshdr);
    dengine_shader_destroy(&dftshdr);

    free(prtbf);
    free(duck);

//    if(child_sep)
//        free(child_sep);

    if(sep_planes)
        free(sep_planes);

    if(nsl)
        denginescript_nsl_free(nsl);

    denginegui_terminate();
    denginescript_terminate();
    dengineutils_filesys_terminate();
    dengine_window_destroy(window);
    dengine_window_terminate();

    return 0;
}
