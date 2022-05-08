#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dengine/dengine.h>

#include <android/native_window.h>
#include <android/native_activity.h>

typedef struct
{
    int windowinit;
    Material cube_mat,duck_mat,dft_mat, skymat, sep_planes_mat;
    Shader* stdshdr,* dftshdr,* shadow2d,* skycube;
    Scene* scene;
    Primitive* duck,* sep_planes;
    Camera cam;
    char prtbf[2048];
}dt_script;

static dt_script ds;

static void init(struct android_app* app)
{
    memset(&ds,0,sizeof(dt_script));
    //Acquire win
    ANativeWindow_acquire(app->window);
    dengine_window_android_set_nativewindow(app->window);

    dengine_window_request_GL(2, 0, 0);
    if(dengine_init())
    {
        ds.windowinit = 1;

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

        dengine_camera_setup(&ds.cam);
        dengine_camera_set_rendermode(DENGINE_CAMERA_RENDER_FOWARD,&ds.cam);

        //lower resolution
        dengine_camera_resize(&ds.cam, 854, 480);

        ds.cam.clearcolor[0] = 0.2f;
        ds.cam.clearcolor[1] = 0.2f;
        ds.cam.clearcolor[2] = 0.2f;

        CameraComponent* camc = denginescene_ecs_new_cameracomponent(&ds.cam);
        ent13->camera_component=camc;

        ds.stdshdr = dengine_shader_new_shader_standard(DENGINE_SHADER_STANDARD);
        ds.dftshdr = dengine_shader_new_shader_standard(DENGINE_SHADER_DEFAULT);
        ds.shadow2d = dengine_shader_new_shader_standard(DENGINE_SHADER_SHADOW2D);
        ds.skycube = dengine_shader_new_shader_standard(DENGINE_SHADER_SKYBOXCUBE);

        dengine_material_setup(&ds.cube_mat);
        dengine_material_setup(&ds.duck_mat);
        dengine_material_setup(&ds.dft_mat);
        dengine_material_setup(&ds.skymat);

        dengine_material_set_shader_color(ds.stdshdr,&ds.cube_mat);
        dengine_material_set_shader_color(ds.stdshdr,&ds.duck_mat);
        dengine_material_set_shader_color(ds.dftshdr,&ds.dft_mat);
        dengine_material_set_shader_color(ds.skycube,&ds.skymat);

        dengine_material_set_shader_shadow(ds.shadow2d, &ds.cube_mat);
        dengine_material_set_shader_shadow(ds.shadow2d, &ds.duck_mat);

        Primitive cube,plane,grid;
        dengine_primitive_gen_cube(&cube, ds.stdshdr);
        dengine_primitive_gen_plane(&plane, ds.stdshdr);
        dengine_primitive_gen_grid(10,&grid, ds.dftshdr);

        File2Mem f2m;
        f2m.file = "models/duck.obj";
        dengine_android_asset2file2mem(&f2m);
        ds.duck = denginemodel_load_mem(DENGINE_MODEL_FORMAT_OBJ, f2m.mem, f2m.size, NULL, ds.stdshdr);
        dengineutils_filesys_file2mem_free(&f2m);

// android input is not working atm
//        char* duckscriptfile = "scripts/moveduck.py";
//        Script duckscript;
//        f2m.file = duckscriptfile;
//        dengine_android_asset2file2mem(&f2m);
//        denginescript_python_compile(f2m.mem, duckscriptfile, &duckscript);
//        denginescene_ecs_add_script(ent3, &duckscript);
//        //note the same script can be added to other entities for the same effects:
//        //denginescene_ecs_add_script(ent1, &duckscript);
//        //denginescene_ecs_add_script(ent2, &duckscript);
//        dengineutils_filesys_file2mem_free(&f2m);

        char* pingpongscalefile = "scripts/pingpongscale.py";
        Script pingpongscale;
        f2m.file = pingpongscalefile;
        dengine_android_asset2file2mem(&f2m);
        denginescript_python_compile(f2m.mem, pingpongscalefile, &pingpongscale);
        dengineutils_filesys_file2mem_free(&f2m);

        denginescene_ecs_add_script(ent7, &pingpongscale);

        f2m.file = "models/sperated-planes.obj";
        dengine_android_asset2file2mem(&f2m);
        size_t n_planes = 0;
        ds.sep_planes = denginemodel_load_mem(DENGINE_MODEL_FORMAT_OBJ, f2m.mem, f2m.size, &n_planes, ds.stdshdr);
        dengineutils_filesys_file2mem_free(&f2m);

        Entity* ent15 = denginescene_ecs_new_entity();
        dengine_material_setup(&ds.sep_planes_mat);

        dengine_material_set_shader_color(ds.stdshdr,&ds.sep_planes_mat);
        dengine_material_set_shader_shadow(ds.shadow2d, &ds.sep_planes_mat);

        Texture sep_plane_tex;
        memset(&sep_plane_tex,0,sizeof (Texture));
        sep_plane_tex.auto_dataonload=1;
        sep_plane_tex.interface=DENGINE_TEXTURE_INTERFACE_8_BIT;
        f2m.file = "textures/2d/cube_diff.png";
        dengine_android_asset2file2mem(&f2m);
        dengine_texture_load_mem(f2m.mem, f2m.size, 1, &sep_plane_tex);

        dengine_material_set_texture(&sep_plane_tex,"diffuseTex",&ds.sep_planes_mat);

        if(n_planes)
        {
            for(size_t i = 0; i < n_planes; i++)
            {
                Entity* ent = denginescene_ecs_new_entity();
                if(ent)
                {
                    ent->transform.position[1] = (float)i / (float)n_planes;
                    ent->transform.rotation[1] = i * 10.0f;

                    MeshComponent* sep_plane_mesh = denginescene_ecs_new_meshcomponent(&ds.sep_planes[i], &ds.sep_planes_mat);
                    ent->mesh_component = sep_plane_mesh;
                    denginescene_ecs_parent(ent15, ent);
                }
            }
        }
        char* contantrotationfile = "scripts/constantrotation.py";
        Script constantrotation;
        f2m.file = contantrotationfile;
        dengine_android_asset2file2mem(&f2m);
        denginescript_python_compile(f2m.mem, contantrotationfile, &constantrotation);
        dengineutils_filesys_file2mem_free(&f2m);
        denginescene_ecs_add_script(ent15, &constantrotation);

        DirLight dLight;
        memset(&dLight,0,sizeof (DirLight));
        //uncomment to slow everything down with shadows
//        dLight.shadow.enable = 1;
//        dLight.shadow.shadow_map_size = 512;
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

        char* pingpongpositionfile = "scripts/pingpongposition.py";
        Script pingpongposition;
        f2m.file = pingpongpositionfile;
        dengine_android_asset2file2mem(&f2m);
        denginescript_python_compile(f2m.mem, pingpongpositionfile, &pingpongposition);
        dengineutils_filesys_file2mem_free(&f2m);
        denginescene_ecs_add_script(ent_plight, &pingpongposition);

        MeshComponent* cube_mesh, * cube_mesh2,* plane_mesh,* duck_mesh,* grid_mesh;
        cube_mesh = denginescene_ecs_new_meshcomponent(&cube, &ds.cube_mat);
        plane_mesh = denginescene_ecs_new_meshcomponent(&plane, &ds.cube_mat);
        duck_mesh = denginescene_ecs_new_meshcomponent(ds.duck, &ds.duck_mat);
        grid_mesh = denginescene_ecs_new_meshcomponent(&grid, &ds.dft_mat);
        cube_mesh2 = denginescene_ecs_new_meshcomponent(&cube, &ds.cube_mat);

        ent1->mesh_component=plane_mesh;
        ent6->mesh_component=cube_mesh;
        ent7->mesh_component=cube_mesh2;
        ent3->mesh_component=duck_mesh;
        ent14->mesh_component=grid_mesh;

        Texture duck_tex;
        memset(&duck_tex,0,sizeof (Texture));
        duck_tex.auto_dataonload=1;
        duck_tex.interface=DENGINE_TEXTURE_INTERFACE_8_BIT;
        f2m.file = "textures/2d/duck.png";
        dengine_android_asset2file2mem(&f2m);
        dengine_texture_load_mem(f2m.mem, f2m.size,1,&duck_tex);

        dengine_material_set_texture(&duck_tex,"diffuseTex",&ds.duck_mat);

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

        ds.scene = denginescene_new();

        denginescene_add_entity(ds.scene, ent1);
        denginescene_add_entity(ds.scene, ent2);
        denginescene_add_entity(ds.scene, ent10);
        denginescene_add_entity(ds.scene, ent13);
        denginescene_add_entity(ds.scene, ent14);
        denginescene_add_entity(ds.scene, ent15);

        denginescene_add_entity(ds.scene, ent_plight);
        denginescene_add_entity(ds.scene, ent_dlight);

        denginegui_set_font(NULL, 45.0f, 512);

        Texture cubemap;
        memset(&cubemap, 0, sizeof(Texture));
        cubemap.type = GL_UNSIGNED_BYTE;
        cubemap.filter_min = GL_LINEAR;
        cubemap.interface = DENGINE_TEXTURE_INTERFACE_8_BIT;

        dengine_texture_gen(1, &cubemap);
        dengine_texture_bind(GL_TEXTURE_CUBE_MAP, &cubemap);
        static char prtbf[1024];
        for(int i = 0; i < 6; i++)
        {
            snprintf(prtbf, sizeof(prtbf), "textures/cubemaps/sea/sea%d.jpg",
                     i + 1);
            f2m.file = prtbf;
            dengine_android_asset2file2mem(&f2m);
            dengine_texture_load_mem(f2m.mem, f2m.size, 0, &cubemap);
            cubemap.format = cubemap.channels == 3 ? GL_RGB : GL_RGBA;
            cubemap.internal_format = cubemap.format;
            dengine_texture_data(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, &cubemap);
            dengine_texture_free_data(&cubemap);
            dengineutils_filesys_file2mem_free(&f2m);
        }
        dengine_texture_set_params(GL_TEXTURE_CUBE_MAP, &cubemap);
        dengine_texture_bind(GL_TEXTURE_CUBE_MAP, NULL);

        dengine_material_set_texture(&cubemap, "cubemap", &ds.skymat);

        Skybox* sky = denginescene_new_skybox(&cube, &ds.skymat);
        ds.scene->skybox = sky;

        vec3 gridcolor = {0.0f, 1.0f, 0.0f};
        dengine_shader_set_vec3(ds.dftshdr, "color", gridcolor);

        printf("test stdout to logcat!");

        denginescene_ecs_do_script_scene(ds.scene, DENGINE_SCRIPT_FUNC_START);
    }
}

static void term(struct  android_app* app)
{
    dengine_material_destroy(&ds.cube_mat);
    dengine_material_destroy(&ds.duck_mat);
    dengine_material_destroy(&ds.dft_mat);
    dengine_material_destroy(&ds.sep_planes_mat);
    dengine_material_destroy(&ds.skymat);

    free(ds.stdshdr);
    free(ds.dftshdr);
    free(ds.shadow2d);
    free(ds.skycube);

    free(ds.duck);
    if(ds.sep_planes)
        free(ds.sep_planes);

    dengine_terminate();
    ANativeWindow_release(app->window);
    ANativeActivity_finish(app->activity);
}

static void draw()
{
    //render scene
    denginescene_update(ds.scene);
    int width, height;
    dengine_viewport_get(NULL, NULL, &width, &height);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    double delta = dengineutils_timer_get_delta();
    double delta_s = delta / 1000.0;
    static double elaped = 9999.0;
    static char fpsstr[30];
    static vec4 yellow = {1.0f, 1.0f, 0.0f, 1.0f};
    static vec4 blackrgba = {0.0f, 0.0f, 0.0f, 1.0f};
    elaped += delta_s;
    if(elaped > 1.0)
    {
        snprintf(fpsstr, sizeof(fpsstr), "FPS %.1f (%.2fms)", 1 / delta_s, delta);
        elaped = 0.0;
    }

    //SCENE COLOR OUTPUT
    denginegui_panel(0, 0, (float)width, (float)height, ds.cam.framebuffer.color, NULL, blackrgba);

    //FPS
    denginegui_text(10,10,fpsstr, yellow);

    dengine_update();
}

void android_main(struct android_app* state)
{
    dengine_android_set_app(state);
    dengine_android_set_initfunc(init);
    dengine_android_set_terminatefunc(term);

    if(state->savedState)
    {
        dengineutils_logging_log("Restoring from save state");
    }

    while(1)
    {
        dengine_android_pollevents();

        //Quit and detach
        if(state->destroyRequested != 0)
        {
            dengineutils_logging_log("Destroy Requested");
            dengineutils_logging_log("Goodbye!");
            return;
        }

        if (ds.windowinit)
            draw();
    }
}
