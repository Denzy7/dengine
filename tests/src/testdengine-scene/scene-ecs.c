#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dengine/dengine.h>

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

int testdengine_scece_ecs_main(int argc, char **argv)
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

    if(!denginescript_isinit())
        usensl = 1;

    NSL nsl = NULL;
    nsl = denginescript_nsl_load("nsl-test.nsl");
    if(!nsl)
    {
        dengineutils_logging_log("ERROR::nsl-test.nsl not found. place it next to executable");
        return -1;
    }
    dengineutils_logging_log("INFO::NSL loaded!");

    if(!usensl)
        dengineutils_logging_log("you can pass -usensl to use native scripting library alternative");

    Entity* ent1 ; denginescene_ecs_new_entity(&ent1);
    Entity* ent2 ; denginescene_ecs_new_entity(&ent2);
    Entity* ent3 ; denginescene_ecs_new_entity(&ent3);
    Entity* ent4 ; denginescene_ecs_new_entity(&ent4);
    Entity* ent5 ; denginescene_ecs_new_entity(&ent5);
    Entity* ent6 ; denginescene_ecs_new_entity(&ent6);
    Entity* ent7 ; denginescene_ecs_new_entity(&ent7);
    Entity* ent8 ; denginescene_ecs_new_entity(&ent8);
    Entity* ent9 ; denginescene_ecs_new_entity(&ent9);
    Entity* ent10 ; denginescene_ecs_new_entity(&ent10);
    Entity* ent11 ; denginescene_ecs_new_entity(&ent11);
    Entity* ent12 ; denginescene_ecs_new_entity(&ent12);
    Entity* ent13 ; denginescene_ecs_new_entity(&ent13);

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

    cam.clearcolor[0] = 1.0f;
    cam.clearcolor[1] = 0.2f;
    cam.clearcolor[2] = 0.2f;

    denginescene_ecs_new_cameracomponent(&cam, &ent13->camera_component);
    Script scenecamera;
    /* this demonstrates a nice mix of nsl and python script. NOT BECAUSE 
     * I TOTALLY DONT WANT TO WRITE ONE!
     */
    denginescript_nsl_get_script("scenecamera", &scenecamera, nsl);
    denginescene_ecs_add_script(ent13, &scenecamera);
   
    Shader stdshdr;
    Shader dftshdr;
    Shader shadow2d;
    Shader shadow3d;
    Shader skycube;

    dengine_shader_make_standard(DENGINE_SHADER_STANDARD, &stdshdr);
    dengine_shader_make_standard(DENGINE_SHADER_DEFAULT, &dftshdr);
    dengine_shader_make_standard(DENGINE_SHADER_SHADOW2D, &shadow2d);
    dengine_shader_make_standard(DENGINE_SHADER_SHADOW3D, &shadow3d);
    dengine_shader_make_standard(DENGINE_SHADER_SKYBOXCUBE, &skycube);

    Material cube_mat,duck_mat,dft_mat, skymat;

    dengine_material_setup(&cube_mat);
    cube_mat.properties.specular_power = 128.0f;
    dengine_material_setup(&duck_mat);
    duck_mat.properties.specular_power = 256.0f;
    dengine_material_setup(&dft_mat);
    dengine_material_setup(&skymat);

    dengine_material_set_shader_color(&stdshdr,&cube_mat);
    dengine_material_set_shader_color(&stdshdr,&duck_mat);
    dengine_material_set_shader_color(&dftshdr,&dft_mat);
    dengine_material_set_shader_color(&skycube,&skymat);

    dengine_material_set_shader_shadow(&shadow2d, &cube_mat);
    dengine_material_set_shader_shadow(&shadow2d, &duck_mat);
    dengine_material_set_shader_shadow3d(&shadow3d, &cube_mat);
    dengine_material_set_shader_shadow3d(&shadow3d, &duck_mat);

    Primitive cube,plane;
    dengine_primitive_gen_cube(&cube,&stdshdr);
    dengine_primitive_gen_plane(&plane,&stdshdr);

    const int prtbf_sz=2048;
    char* prtbf=malloc(prtbf_sz);
    void* mem; size_t memsz;

    if(!dengine_load_asset("models/duck.obj", &mem, &memsz)){
        dengineutils_logging_log("ERROR::cant load models/duck.obj");
        return 1;
    }
    Primitive* duck = denginemodel_load_mem(DENGINE_MODEL_FORMAT_OBJ,mem, memsz, NULL,&stdshdr);
    free(mem);

    Script duckscript;
    if(usensl)
    {
        denginescript_nsl_get_script("moveduck", &duckscript, nsl);
    }else
    {
#ifdef DENGINE_SCRIPTING_PYTHON
    const char* duckscriptfile = "scripts/moveduck.py";
    if(!dengine_load_asset(duckscriptfile, &mem, &memsz)){
        dengineutils_logging_log("ERROR::cant load %s", duckscriptfile);
        return 1;
    }
    denginescript_python_compile(mem, duckscriptfile, &duckscript);
    free(mem);
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
        if(!dengine_load_asset(pingpongscalefile, &mem, &memsz)){
            dengineutils_logging_log("ERROR::cant load %s", pingpongscalefile);
            return 1;
        }
        denginescript_python_compile(mem, pingpongscalefile, &pingpongscale);
        free(mem);
    #endif
    }
    denginescene_ecs_add_script(ent7, &pingpongscale);

    //load separated planes
    if(!dengine_load_asset("models/sperated-planes.obj", &mem, &memsz)){
        dengineutils_logging_log("ERROR::cant load models/sperated-planes.obj");
        return 1;
    }
    size_t n_planes = 0;
    Primitive* sep_planes = denginemodel_load_mem(DENGINE_MODEL_FORMAT_OBJ, mem, memsz,&n_planes,&stdshdr);
    free(mem);

    //Entity** child_sep = NULL;
    Entity* ent15 ; denginescene_ecs_new_entity(&ent15);

    Material sep_planes_mat;
    dengine_material_setup(&sep_planes_mat);

    dengine_material_set_shader_color(&stdshdr,&sep_planes_mat);
    dengine_material_set_shader_shadow(&shadow2d, &sep_planes_mat);
    sep_planes_mat.shader_color.hint= DENGINE_SHADER_SCENEQUEUER_SHADERHINT_TRANSPARENT;
    sep_planes_mat.properties.alpha = 0.75f;

    Texture sep_plane_tex;
    memset(&sep_plane_tex,0,sizeof (Texture));
    sep_plane_tex.auto_dataonload=1;
    sep_plane_tex.interface=DENGINE_TEXTURE_INTERFACE_8_BIT;

    char* texinfos[2][2] = 
    {
        {"diffuseTex", "cube_diff.png"},
        {"specularTex", "cube_spec.png"},
    };
    for(int i = 0; i < 2; i++)
    {
        snprintf(prtbf,prtbf_sz,"textures/2d/%s",
                texinfos[i][1]);
        if(!dengine_load_asset(prtbf, &mem, &memsz))
        {
            dengineutils_logging_log("ERROR::cant load %s", prtbf);
            return 1;
        }
        dengine_texture_load_mem(mem, memsz,1,&sep_plane_tex);
        dengine_material_set_texture(&sep_plane_tex,texinfos[i][0],&sep_planes_mat);
        free(mem);
    }

    if(n_planes)
    {
        //child_sep = denginescene_ecs_new_entity_n(n_planes);
        for(size_t i = 0; i < n_planes; i++)
        {
            Entity* ent ; denginescene_ecs_new_entity(&ent);
            if(ent)
            {
                ent->transform.position[1] = (float)i / (float)n_planes;
                ent->transform.rotation[1] = i * 10.0f;

                denginescene_ecs_new_meshcomponent(&sep_planes[i], &sep_planes_mat, &ent->mesh_component);
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
        if(!dengine_load_asset(contantrotationfile, &mem, &memsz))
        {
            dengineutils_logging_log("ERROR::cant load %s", contantrotationfile);
            return 1;
        }
        denginescript_python_compile(mem, contantrotationfile, &constantrotation);
        free(mem);
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
    pLight.shadow.enable = 1;
    pLight.shadow.shadow_map_size = 512;
    dengine_lighting_setup_pointlight(&pLight);
    pLight.light.diffuse[2] = 0;
    pLight.light.specular[2] = 0;
    pLight.light.strength=1.95f;

    Entity* ent_plight ; denginescene_ecs_new_entity(&ent_plight);
    Entity* ent_dlight ; denginescene_ecs_new_entity(&ent_dlight);
    denginescene_ecs_new_lightcomponent(DENGINE_LIGHT_DIR,
            &dLight,
            &ent_dlight->light_component);

    denginescene_ecs_new_lightcomponent(DENGINE_LIGHT_POINT,
            &pLight,
            &ent_plight->light_component);

    ent_plight->transform.position[0] = 5.5f;
    ent_plight->transform.position[1] = 5.5f;

    glm_vec3_fill(ent_dlight->transform.position, -10.0f);
    ent_dlight->transform.position[1] = 10.0f;

    ent_dlight->transform.rotation[0] = 45.0;

    Entity* ent_sLight;
    SpotLight sLight;
    memset(&sLight, 0, sizeof(sLight));
    dengine_lighting_light_setup(DENGINE_LIGHT_SPOT, &sLight);
    sLight.pointLight.light.diffuse[0] = 1.0f;
    denginescene_ecs_new_entity(&ent_sLight);
    denginescene_ecs_new_lightcomponent(DENGINE_LIGHT_SPOT, &sLight, &ent_sLight->light_component);
    ent_sLight->transform.rotation[0] = -45.0f;
    ent_sLight->transform.rotation[1] = 135.0f;
    glm_vec3_fill(ent_sLight->transform.position, 3.0f);

    Script pingpongposition;
    if(usensl)
    {
        denginescript_nsl_get_script("pingpongposition", &pingpongposition, nsl);
    }else
    {
        
#ifdef DENGINE_SCRIPTING_PYTHON
         char* pingpongpositionfile = "scripts/pingpongposition.py";
         if(!dengine_load_asset(pingpongpositionfile, &mem, &memsz))
         {
             dengineutils_logging_log("ERROR::cant load %s", pingpongpositionfile);
             return 1;
         }
        denginescript_python_compile(mem, pingpongpositionfile, &pingpongposition);
        free(mem);
#endif
    }
    denginescene_ecs_add_script(ent_plight, &pingpongposition);

    Texture duck_tex;
    memset(&duck_tex,0,sizeof (Texture));
    duck_tex.auto_dataonload=1;
    duck_tex.interface=DENGINE_TEXTURE_INTERFACE_8_BIT;

    if(!dengine_load_asset("textures/2d/duck.png", &mem, &memsz))
    {
        dengineutils_logging_log("ERROR::cant load textures/2d/duck.png");
        return 1;
    }
    dengine_texture_load_mem(mem, memsz,1,&duck_tex);
    free(mem);

    dengine_material_set_texture(&duck_tex,"diffuseTex",&duck_mat);
    denginescene_ecs_new_meshcomponent(duck, &duck_mat, &ent3->mesh_component);

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
        float bg[] = {1. - col[2], 1. - col[1], 1. - col[0]};
        dengine_texture_make_checkerboard(8, 8, 2, col, bg, 0, 3, cube_tex);
    }
    denginescene_ecs_new_meshcomponent(&plane, &cube_mat, &ent1->mesh_component);
    dengine_material_set_texture(&cube_tex_pool[dengineutils_rng_int(DENGINE_ARY_SZ(cube_tex_pool))],"diffuseTex", &ent1->mesh_component->material);
    denginescene_ecs_new_meshcomponent(&cube, &cube_mat, &ent6->mesh_component);
    dengine_material_set_texture(&cube_tex_pool[dengineutils_rng_int(DENGINE_ARY_SZ(cube_tex_pool))],"diffuseTex", &ent6->mesh_component->material);
    denginescene_ecs_new_meshcomponent(&cube, &cube_mat, &ent7->mesh_component);
    dengine_material_set_texture(&cube_tex_pool[dengineutils_rng_int(DENGINE_ARY_SZ(cube_tex_pool))],"diffuseTex", &ent7->mesh_component->material);
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
    vec3 p={6.f,6.f,6.f};
    memcpy(ent13->transform.position,p,sizeof (vec3));
    p[0] = -45.0f, p[1] = 225.0f, p[2] = 0.0f;
    memcpy(ent13->transform.rotation,p,sizeof (vec3));

    p[0]=0.3f,p[1]=2.9f,p[2]=1.0f;
    memcpy(ent6->transform.position,p,sizeof (vec3));

    p[0]=2.0f,p[1]=-2.0f,p[2]=1.0f;
    memcpy(ent7->transform.position,p,sizeof (vec3));

    p[0]=5.0f,p[1]=5.0f,p[2]=5.0f;
    memcpy(ent1->transform.scale,p,sizeof (vec3));

    p[0]=4.0f,p[1]=2.0f,p[2]=1.0f;
    memcpy(ent3->transform.position,p,sizeof (vec3));

    p[0]=3.0f,p[1]=3.0f,p[2]=3.0f;
    memcpy(ent15->transform.position,p,sizeof (vec3));

    ent3->transform.rotation[1]=45.f;

    ent6->transform.rotation[1]=5.f;

    Scene* scene ; denginescene_new(&scene);
    denginescene_queuer_state(scene, DENGINESCENE_QUEUER_STATE_PAUSED);

    denginescene_add_entity(scene, ent1);
    denginescene_add_entity(scene, ent2);
    denginescene_add_entity(scene, ent10);
    denginescene_add_entity(scene, ent13);
    denginescene_add_entity(scene, ent15);

    denginescene_add_entity(scene, ent_plight);
    denginescene_add_entity(scene, ent_dlight);
    denginescene_add_entity(scene, ent_sLight);

    Entity* multicam[3];
    vec3 multicam_transf[] = 
    {
        /* pos,     rot */
        { 0, 10.0f, 0 }, {-89.9f /* 90.0 breaks idk y on new cglm*/, 0, 0.0f},
        { 0, 5.0f, -5.0f }, {-45.0f, 90.0f, 0},
        { -5.0f, 5.0f, 0 }, {-45.0f, 0, 0},
    };
    for(int i = 0; i < 3; i++)
    {
        Camera c;
        memset(&c, 0, sizeof(c));
        dengine_camera_setup(&c);
        dengine_camera_set_rendermode(DENGINE_CAMERA_RENDER_FOWARD, &c);
        if(i == 0)
            c.fov = 90.0f;
        dengine_camera_resize(&c, c.render_width / 2, c.render_height / 2);
        denginescene_ecs_new_entity(&multicam[i]);
        memcpy(multicam[i]->transform.position, multicam_transf[i * 2], sizeof(vec3));
        memcpy(multicam[i]->transform.rotation, multicam_transf[i * 2 + 1], sizeof(vec3));
        denginescene_ecs_new_cameracomponent(&c, &multicam[i]->camera_component);
        denginescene_add_entity(scene, multicam[i]);
    }

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
        snprintf(prtbf, prtbf_sz, "textures/cubemaps/sea/sea%d.jpg",
                 i + 1);
        if(!dengine_load_asset(prtbf, &mem, &memsz))
        {
            dengineutils_logging_log("ERROR::cant load %s", prtbf);
            return 1;
        }
        dengine_texture_load_mem(mem, memsz, 0, &cubemap);
        cubemap.format = cubemap.channels == 3 ? GL_RGB : GL_RGBA;
        cubemap.internal_format = cubemap.format;
        dengine_texture_data(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, &cubemap);
        dengine_texture_free_data(&cubemap);
        free(mem);
    }
    dengine_texture_set_params(GL_TEXTURE_CUBE_MAP, &cubemap);
    dengine_texture_bind(GL_TEXTURE_CUBE_MAP, NULL);

    dengine_material_set_texture(&cubemap, "cubemap", &skymat);

    denginescene_new_skybox(&cube, &skymat, &scene->skybox);

    int poly = 1;

    denginescene_queuer_state(scene, DENGINESCENE_QUEUER_STATE_RUNNING);

    denginescene_scripts_run(scene, DENGINE_SCRIPT_FUNC_START);

    denginescene_dumphierachy_stdio(scene, stdout);
    denginescene_set_debugaxis(scene, 1);
    denginescene_set_debuggrid(scene, 1);

    while (dengine_update()) {
        /*glPolygonMode(GL_FRONT_AND_BACK, GL_LINE + poly);*/

        static double elapsed=9999.0;

        double delta = dengineutils_timer_get_delta();
        double delta_s = delta / 1000.0;

        if(dengine_input_get_key('H'))
            ent_dlight->transform.rotation[0] += delta_s * 45.0f;
        if(dengine_input_get_key('J'))
            ent_dlight->transform.rotation[0] -= delta_s * 45.0f;

        if(dengine_input_get_key('P'))
            ent_sLight->transform.rotation[1] += delta_s * 45.0f;

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

        /*glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);*/

        denginegui_panel(0,0,854,480,&cam.framebuffer.color[0],NULL,viewportcol);
        for(int i = 0; i < 3; i++)
        {
            Entity* e = multicam[i];
            CameraComponent* c = multicam[i]->camera_component;
            int w = 1280 - 854; 
            int h = 720 - 480; 
            denginegui_panel(854, i * h, w, h,&c->camera.framebuffer.color[0],NULL,viewportcol);
            char caminfo[1024];
            snprintf(caminfo, sizeof(caminfo), "Camera %d (fov:%.1f) ", i, c->camera.fov);
            denginegui_text(854, i * h + ( fontsz * 2), caminfo, NULL);
            snprintf(caminfo, sizeof(caminfo), "pos: %f %f %f",
                    e->transform.position[0], e->transform.position[1], e->transform.position[2]);
            denginegui_text(854, i * h + ( fontsz * 1), caminfo, NULL);
            snprintf(caminfo, sizeof(caminfo), "rot: %f %f %f",
                    e->transform.rotation[0], e->transform.rotation[1], e->transform.rotation[2]);
            denginegui_text(854, i * h , caminfo, NULL);
        }

        denginegui_text(10,10,fpsstr, yellow);

        denginegui_text(0, 480 - fontsz, "USE WASD, EC TO MOVE DUCKAROO!", NULL);

        denginegui_text(0, 480 - 2 * fontsz, "USE ZX TO ROTATE DUCKAROO!", NULL);

        snprintf(prtbf, prtbf_sz, "PRESS F TO SWITCH POLYGON MODE : %d", poly);
        denginegui_text(0, 480 - 3 * fontsz, prtbf, NULL);

        snprintf(prtbf, prtbf_sz, "PRESS G TO SWITCH FACE CULLING (Note FPS change) : %d", (int)glIsEnabled(GL_CULL_FACE));
        denginegui_text(0, 480 - 4 * fontsz, prtbf, NULL);

        denginegui_text(0, 480 - 5 * fontsz, "PRESS T TO TAKE A \"SCREENSHOT\"", NULL);

        denginegui_text(0, 480 - 6 * fontsz, "PRESS H/J TO ROTATE THE SUN", NULL);

        denginegui_text(0, 480 - 7 * fontsz, "HOLD RMB AND MOVE MOUSE TO MOVE MAIN CAMERA. SCROLL TO ZOOM", NULL);

        denginegui_panel(0, 480, 720 - 480, 720 - 480, &dLight.shadow.shadow_map.depth, NULL, GLM_VEC4_ONE);
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



    return 0;
}

#ifndef DENGINE_ANDROID
int main(int argc, char *argv[])
{

    DengineInitOpts* opts = dengine_init_get_opts();
    opts->window_title = "testdengine-scene-ecs"; 
    /* for GUI calculation */
    opts->window_width = 1280;
    opts->window_height = 720;
    dengine_init();
    testdengine_scece_ecs_main(argc, argv);
    dengine_terminate();
}
#endif
