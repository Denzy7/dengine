#include <dengine/dengine.h>
int main(int argc, char *argv[])
{
    DengineInitOpts* opts = dengine_init_get_opts();
    opts->window_title = "testdengine-script-nslbullet";
    dengine_init();

    NSL nsl = denginescript_nsl_load("nsl-bulletphysics.nsl");
    if(!nsl)
        return -1;

    nslfunc basic_create_rb = denginescript_nsl_get_func(nsl, "basic_create_rb");
    if(!basic_create_rb)
    {
        dengineutils_logging_log("ERROR::Cannot load nslfunc basic_create_rb");
        return -1;
    }

    const size_t prtbf_sz = 2048;
    char* prtbf = malloc(prtbf_sz);

    Script basic, basic_world;
    denginescript_nsl_get_script("basic", &basic, nsl);
    denginescript_nsl_get_script("basic_world", &basic_world, nsl);

    ECSPhysicsWorld world;
    world.substeps = 10;
    world.timestep = 1.f/40.f;
    world.timestep_fixed = 1.f/ 60.f;

    //manually call start to create world
    denginescript_call(&basic_world, DENGINE_SCRIPT_FUNC_START, &world);

    Script forces;
    denginescript_nsl_get_script("forces", &forces, nsl);

    /*
     * this script(plugin) uses bulletphysics to move an entity object.
     *
     * Notice how NSL enables C and C++ to communicate with the engine
     *
     * Build as shared to avoid link errors
     *
     * NSL can also access anything in the eco-system and is more powerful than regular
     * python
     */

    Scene* scene = denginescene_new();

    Camera cam;
    dengine_camera_setup(&cam);
    dengine_camera_set_rendermode(DENGINE_CAMERA_RENDER_FOWARD, &cam);
    cam.clearcolor[0] = 0.1f;
    cam.clearcolor[1] = 0.1f;
    cam.clearcolor[2] = 0.1f;
    cam.clearcolor[3] = 0.1f;

    CameraComponent* cam_c = denginescene_ecs_new_cameracomponent(&cam);
    Entity* cam_ent = denginescene_ecs_new_entity();
    cam_ent->camera_component = cam_c;
    cam_ent->transform.position[0] = 15.0f;
    cam_ent->transform.position[1] = 15.0f;
    cam_ent->transform.position[2] = 15.0f;
    cam_ent->transform.rotation[0] = -35.0f;
    cam_ent->transform.rotation[1] = 225.0f;
    denginescene_add_entity(scene, cam_ent);

    Shader* stdshdr = dengine_shader_new_shader_standard(DENGINE_SHADER_STANDARD);

    Primitive cube, plane;
    dengine_primitive_gen_cube(&cube, stdshdr);
    dengine_primitive_gen_plane(&plane, stdshdr);

    Material plane_mat;
    dengine_material_setup(&plane_mat);
    dengine_material_set_shader_color(stdshdr, &plane_mat);
    static const char* texs_plane[][2]=
    {
        {"textures/2d/plane_diff.png", "diffuseTex"},
        {"textures/2d/plane_spec.png", "specularTex"}
    };
    Texture tex_plane[2];
    memset(&tex_plane, 0, sizeof(tex_plane));
    for(int i = 0; i < DENGINE_ARY_SZ(texs_plane); i++)
    {
        tex_plane[i].auto_dataonload = 1;
        tex_plane[i].interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
        snprintf(prtbf, prtbf_sz, "%s/%s", dengineutils_filesys_get_assetsdir(), texs_plane[i][0]);
        dengine_texture_load_file(prtbf, 1, &tex_plane[i]);
        dengine_material_set_texture(&tex_plane[i], texs_plane[i][1], &plane_mat);
    }

    MeshComponent* plane_mesh = denginescene_ecs_new_meshcomponent(&plane, &plane_mat);
    Entity* plane_ent = denginescene_ecs_new_entity();
    plane_ent->transform.position[0] = 0.0;
    plane_ent->transform.position[1] = 0.0;
    plane_ent->transform.position[2] = 0.0;
    plane_ent->transform.scale[0] = 20.0;
    plane_ent->transform.scale[1] = 0.25;
    plane_ent->transform.scale[2] = 20.0;
    plane_ent->mesh_component = plane_mesh;
    plane_ent->physics_component = denginescene_ecs_new_physicscomponent(DENGINE_ECS_PHYSICS_COLSHAPE_BOX, NULL, 0.0);
    basic_create_rb(plane_ent);
    denginescene_add_entity(scene, plane_ent);

    Material cube2_mat;
    dengine_material_setup(&cube2_mat);
    dengine_material_set_shader_color(stdshdr, &cube2_mat);

    static const char* texs_cube[][2]=
    {
        {"textures/2d/brickwall.jpg", "diffuseTex"},
        {"textures/2d/brickwall_normal.jpg", "normalTex"}
    };
    Texture tex_cube[2];
    memset(&tex_cube, 0, sizeof(tex_cube));
    for(int i = 0; i < DENGINE_ARY_SZ(texs_cube); i++)
    {
        tex_cube[i].interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
        tex_cube[i].auto_dataonload = 1;
        snprintf(prtbf, prtbf_sz, "%s/%s", dengineutils_filesys_get_assetsdir(), texs_cube[i][0]);
        dengine_texture_load_file(prtbf, 1, &tex_cube[i]);
        dengine_material_set_texture(&tex_cube[i], texs_cube[i][1], &cube2_mat);
    }

    MeshComponent* cube_mesh = denginescene_ecs_new_meshcomponent(&cube, &cube2_mat);
    Entity* cube_ent = denginescene_ecs_new_entity();
    cube_ent->transform.position[0] = 0.0;
    cube_ent->transform.position[1] = 10.0;
    cube_ent->transform.position[2] = 0.0;
    cube_ent->transform.rotation[2] = 45.0f;
    cube_ent->transform.rotation[1] = -45.0f;
    cube_ent->transform.rotation[0] = -45.0f;
    cube_ent->mesh_component = cube_mesh;
    cube_ent->physics_component = denginescene_ecs_new_physicscomponent(DENGINE_ECS_PHYSICS_COLSHAPE_BOX, NULL, 10.0);
    basic_create_rb(cube_ent);
    denginescene_add_entity(scene, cube_ent);

    //add basic since this entity has mass
    denginescene_ecs_add_script(cube_ent, &basic);
    //move with forces too
    denginescene_ecs_add_script(cube_ent, &forces);
    dengineutils_rng_set_seedwithtime();

    Material cube_mat;
    dengine_material_setup(&cube_mat);
    dengine_material_set_shader_color(stdshdr, &cube_mat);

    const int coef = 4;
    const float spread = 1.24;;
    for(int i = -coef; i < coef; i++)
    {
        for(int j = -coef; j < coef; j++)
        {
            Entity* tiny = denginescene_ecs_new_entity();
            tiny->transform.position[0] = (float)i * coef * spread;
            tiny->transform.position[1] = 10.4 * (float)dengineutils_rng_int(3);
            tiny->transform.position[2] = (float)j * coef * spread;
            tiny->transform.rotation[0] = (float)dengineutils_rng_int(45);
            tiny->transform.rotation[1] = (float)dengineutils_rng_int(-45);
            tiny->transform.rotation[2] = (float)dengineutils_rng_int(135);
            tiny->mesh_component = denginescene_ecs_new_meshcomponent(&cube, &cube_mat);
            tiny->physics_component = denginescene_ecs_new_physicscomponent(DENGINE_ECS_PHYSICS_COLSHAPE_BOX, NULL, 1.1);
            basic_create_rb(tiny);
            //add basic since this entity has mass
            denginescene_ecs_add_script(tiny, &basic);

            denginescene_add_entity(scene, tiny);
        }
    }

    PointLight pl;
    memset(&pl, 0, sizeof(PointLight));
    dengine_lighting_light_setup(DENGINE_LIGHT_POINT, &pl);
    pl.light.strength = 3.0f;
    pl.quadratic = 0.010;
    Entity* pl_ent = denginescene_ecs_new_entity();
    LightComponent* dl_ent_lightcomp = denginescene_ecs_new_lightcomponent(
                DENGINE_LIGHT_POINT, &pl);
    pl_ent->light_component = dl_ent_lightcomp;
    pl_ent->transform.position[0] = 0.0f;
    pl_ent->transform.position[1] = 10.0f;
    pl_ent->transform.position[2] = 0.0f;
    denginescene_add_entity(scene, pl_ent);

    while (dengine_window_isrunning()) {
        //stepSimulation
        denginescript_call(&basic_world, DENGINE_SCRIPT_FUNC_UPDATE, &world);

        denginescene_update(scene);

        vec4 black = {0.0f, 0.0f, 0.0f, 1.0f};
        int w, h;
        dengine_viewport_get(NULL, NULL, &w, &h);

        denginegui_panel(0, 0, w, h, cam_ent->camera_component->camera->framebuffer.color, NULL, black);
        float fontsz = denginegui_get_fontsz();
        static const char* staticmessageslist[] =
        {
            "Press WASD to apply movement force",
            "F to apply upthrust",
            "G to apply downthrust",
            "Z to apply -ve upward torque",
            "X to apply +ve upward torque",
            "Q to apply +ve forward torque",
            "E to apply -ve forward torque",
            "R to reset position",
        };

        for(int i = 0; i < DENGINE_ARY_SZ(staticmessageslist); i++)
        {
            denginegui_text(fontsz / 4, h - fontsz - i * fontsz, staticmessageslist[i], NULL);
        }

        static char fpstr[100];
        static vec4 yellow = {1.0, 1.0, 0.0, 1.0};

        double delta = dengineutils_timer_get_delta();
        static double elapsed = 0.0;
        elapsed += delta;
        if(elapsed > 1000.0){
            snprintf(fpstr, sizeof (fpstr), "%s :: FPS : %.1f(%.2fms)", (char*)glGetString(GL_VERSION), 1 / (delta / 1000.0), delta);
            elapsed = 0.0;
        }
        denginegui_text(fontsz, fontsz, fpstr, yellow);

        dengine_update();
    }
    //destroy world
    denginescript_call(&basic_world, DENGINE_SCRIPT_FUNC_TERMINATE, NULL);

    denginescene_destroy(scene);
    free(prtbf);
    free(stdshdr);
    dengine_material_destroy(&cube_mat);
    dengine_material_destroy(&plane_mat);
    dengine_material_destroy(&cube2_mat);
    denginescript_nsl_free(nsl);

    dengine_terminate();
    return 0;
}
