#include "denginebulletcommon.h"
#include <vector> //bruh
typedef struct
{
    const char* str;
    float* val;
}ShadowProp;

btDynamicsWorld* worldref = NULL;
btRigidBody* cube_bodyref;
Scene* scene;
Material plane_mat, cube2_mat;
Entity* cube_ent;
DirLight* dl_ent_dl;
Entity* pl_ent;
Texture pl_ent_gizmo;
const size_t prtbf_sz = 2048;
char* prtbf;
Entity* cam_ent;
float black_f[4] = {0.0f, 0.0f, 0.0f, 1.0f};
std::vector<ECSPhysicsBody> cubes;

void basic_update()
{
    for(size_t i = 0; i < cubes.size(); i++)
    {
        ECSPhysicsBody epb = cubes.at(i);
        phy2ent(epb.body->getWorldTransform(), epb.ent);
    }
    
    phy2ent(cube_bodyref->getWorldTransform(), cube_ent);

    static const float force = 5.0f;
    if(dengine_input_get_key('F'))
        cube_bodyref->applyCentralImpulse(btVector3(0, force, 0));
    if(dengine_input_get_key('G'))
        cube_bodyref->applyCentralImpulse(btVector3(0, -force, 0));

    if(dengine_input_get_key('W'))
        cube_bodyref->applyCentralImpulse(btVector3(0, 0, -force));
    if(dengine_input_get_key('S'))
        cube_bodyref->applyCentralImpulse(btVector3(0, 0, force));
    if(dengine_input_get_key('A'))
        cube_bodyref->applyCentralImpulse(btVector3(-force, 0, 0));
    if(dengine_input_get_key('D'))
        cube_bodyref->applyCentralImpulse(btVector3(force, 0, 0));

    if(dengine_input_get_key('Z'))
        cube_bodyref->applyTorqueImpulse( btVector3(0, force, 0));
    if(dengine_input_get_key('X'))
        cube_bodyref->applyTorqueImpulse( btVector3(0, -force, 0));

    if(dengine_input_get_key('E'))
        cube_bodyref->applyTorqueImpulse( btVector3(0, 0, force));
    if(dengine_input_get_key('Q'))
        cube_bodyref->applyTorqueImpulse( btVector3(0, 0, -force));

    if(dengine_input_get_key_once('R'))
    {
        btTransform& transform = cube_bodyref->getWorldTransform();

        transform.setOrigin(btVector3(0, 5, 0));
        transform.setRotation(btQuaternion(0, 0, 0));

        cube_bodyref->getMotionState()->setWorldTransform(transform);

        cube_bodyref->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
        cube_bodyref->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
        cube_bodyref->clearForces();
    }

}
extern "C" int basic_world_start(void*)
{
    initworld(&worldref);

    prtbf = new char [prtbf_sz];

    /* create visuals */
    scene = denginescene_new();

    Camera cam;
    dengine_camera_setup(&cam);
    dengine_camera_set_rendermode(DENGINE_CAMERA_RENDER_FOWARD, &cam);
    cam.clearcolor[0] = 0.1f;
    cam.clearcolor[1] = 0.1f;
    cam.clearcolor[2] = 0.1f;
    cam.clearcolor[3] = 0.1f;

    CameraComponent* cam_c = denginescene_ecs_new_cameracomponent(&cam);
    cam_ent = denginescene_ecs_new_entity();
    cam_ent->camera_component = cam_c;
    cam_ent->transform.position[0] = 15.0f;
    cam_ent->transform.position[1] = 15.0f;
    cam_ent->transform.position[2] = 15.0f;
    cam_ent->transform.rotation[0] = -35.0f;
    cam_ent->transform.rotation[1] = 225.0f;
    denginescene_add_entity(scene, cam_ent);

    Shader stdshdr, shadow2d;
    dengine_shader_make_standard(DENGINE_SHADER_STANDARD, &stdshdr);
    dengine_shader_make_standard(DENGINE_SHADER_SHADOW2D, &shadow2d);

    Primitive cube, plane;
    dengine_primitive_gen_cube(&cube, &stdshdr);
    dengine_primitive_gen_plane(&plane, &stdshdr);

    dengine_material_setup(&plane_mat);
    dengine_material_set_shader_color(&stdshdr, &plane_mat);
    dengine_material_set_shader_shadow(&shadow2d, &plane_mat);
    static const char* texs_plane[][2]=
    {
        {"textures/2d/plane_diff.png", "diffuseTex"},
        {"textures/2d/plane_spec.png", "specularTex"}
    };
    Texture tex_plane[2];
    memset(&tex_plane, 0, sizeof(tex_plane));
    for(size_t i = 0; i < DENGINE_ARY_SZ(texs_plane); i++)
    {
        void* texmem;
        size_t texmem_ln;
        tex_plane[i].auto_dataonload = 1;
        tex_plane[i].interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
        dengine_load_asset(texs_plane[i][0], &texmem, &texmem_ln);
        dengine_texture_load_mem(texmem, texmem_ln, 1, &tex_plane[i]);
        free(texmem);
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
    create_rb(plane_ent, DENGINE_ECS_PHYSICS_COLSHAPE_BOX, NULL, 0.0, NULL);
    denginescene_add_entity(scene, plane_ent);

    dengine_material_setup(&cube2_mat);
    dengine_material_set_shader_color(&stdshdr, &cube2_mat);
    dengine_material_set_shader_shadow(&shadow2d, &cube2_mat);

    static const char* texs_cube[][2]=
    {
        {"textures/2d/brickwall.jpg", "diffuseTex"},
        {"textures/2d/brickwall_normal.jpg", "normalTex"}
    };
    Texture tex_cube[2];
    memset(&tex_cube, 0, sizeof(tex_cube));
    for(size_t i = 0; i < DENGINE_ARY_SZ(texs_cube); i++)
    {
        void* texmem;
        size_t texmem_ln;
        tex_cube[i].interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
        tex_cube[i].auto_dataonload = 1;
        dengine_load_asset(texs_cube[i][0], &texmem, &texmem_ln);
        dengine_texture_load_mem(texmem, texmem_ln, 1, &tex_cube[i]);
        free(texmem);
        dengine_material_set_texture(&tex_cube[i], texs_cube[i][1], &cube2_mat);
    }

    MeshComponent* cube_mesh = denginescene_ecs_new_meshcomponent(&cube, &cube2_mat);
    cube_ent = denginescene_ecs_new_entity();
    cube_ent->transform.position[0] = 0.0;
    cube_ent->transform.position[1] = 10.0;
    cube_ent->transform.position[2] = 0.0;
    cube_ent->transform.rotation[2] = 45.0f;
    cube_ent->transform.rotation[1] = -45.0f;
    cube_ent->transform.rotation[0] = -45.0f;
    cube_ent->mesh_component = cube_mesh;
    create_rb(cube_ent, DENGINE_ECS_PHYSICS_COLSHAPE_BOX, NULL, 10.0, &cube_bodyref);
    denginescene_add_entity(scene, cube_ent);
    dengineutils_rng_set_seedwithtime();

    //feel free to increase pool
    Texture cube_tex_pool[3];
    for(size_t i = 0; i < DENGINE_ARY_SZ(cube_tex_pool); i++)
    {
        Texture* cube_tex = &cube_tex_pool[i];
        memset(cube_tex, 0, sizeof(Texture));
        float col[] = { 
            dengineutils_rng_int(255) / 255.0f,
            dengineutils_rng_int(255) / 255.0f,
            dengineutils_rng_int(255) / 255.0f};
        //simple color
        //dengine_texture_make_color(8, 8, col, 3, cube_tex);

        //fancy checkerboard
        float bg[] = {1.0f - col[2], 1.0f - col[1], 1.0f - col[0]};
        dengine_texture_make_checkerboard(8, 8, 2, col, bg, 0, 3, cube_tex);
    }

    const int coef = 4;
    const float spread = 1.24;;
    for(int i = -coef; i < coef; i++)
    {
        for(int j = -coef; j < coef; j++)
        {
            Material cube_mat;
            dengine_material_setup(&cube_mat);
            dengine_material_set_shader_color(&stdshdr, &cube_mat);
            dengine_material_set_shader_shadow(&shadow2d, &cube_mat);

            int pick = dengineutils_rng_int(DENGINE_ARY_SZ(cube_tex_pool));
            Texture* coltex = &cube_tex_pool[pick];
            dengine_material_set_texture(coltex, "diffuseTex", &cube_mat);

            Entity* tiny = denginescene_ecs_new_entity();
            tiny->transform.position[0] = (float)i * coef * spread;
            tiny->transform.position[1] = 10.4 * (float)dengineutils_rng_int(3);
            tiny->transform.position[2] = (float)j * coef * spread;
            tiny->transform.rotation[0] = (float)dengineutils_rng_int(45);
            tiny->transform.rotation[1] = (float)dengineutils_rng_int(45);
            tiny->transform.rotation[2] = (float)dengineutils_rng_int(135);
            tiny->mesh_component = denginescene_ecs_new_meshcomponent(&cube, &cube_mat);
            btRigidBody* b;
            create_rb(tiny, DENGINE_ECS_PHYSICS_COLSHAPE_BOX, NULL, 1.1, &b);
            ECSPhysicsBody epb;
            epb.body = b;
            epb.ent = tiny;
            cubes.push_back(epb);
            denginescene_add_entity(scene, tiny);
        }
    }

    PointLight pl;
    memset(&pl, 0, sizeof(PointLight));
    dengine_lighting_light_setup(DENGINE_LIGHT_POINT, &pl);
    pl.light.strength = 3.0f;
    pl.quadratic = 0.010;
    pl.light.diffuse[0] = 0;
    pl_ent = denginescene_ecs_new_entity();
    LightComponent* dl_ent_lightcomp = denginescene_ecs_new_lightcomponent(
                DENGINE_LIGHT_POINT, &pl);
    pl_ent->light_component = dl_ent_lightcomp;
    pl_ent->transform.position[0] = 0.0f;
    pl_ent->transform.position[1] = 10.0f;
    pl_ent->transform.position[2] = 0.0f;
    denginescene_add_entity(scene, pl_ent);
    dengine_texture_make_color(8, 8, pl.light.diffuse, 3, &pl_ent_gizmo);

    DirLight dl;
    memset(&dl, 0, sizeof(DirLight));
    dl.shadow.enable = 1;
    dl.shadow.shadow_map_size = 512;
    dengine_lighting_light_setup(DENGINE_LIGHT_DIR, &dl);
    dl.shadow.far_shadow = 30.0f;
    dl.shadow_ortho = 22.5f;
    Entity* dl_ent = denginescene_ecs_new_entity();
    dl_ent->transform.position[0] = -6.0f;
    dl_ent->transform.position[1] = 12.0f;
    dl_ent->transform.position[2] = 6.0f;
    dl_ent->light_component = denginescene_ecs_new_lightcomponent(DENGINE_LIGHT_DIR, &dl);
    denginescene_add_entity(scene, dl_ent);
    dl_ent_dl = (DirLight*)dl_ent->light_component->light;

    startworld();
    return 1;
}

extern "C" int basic_world_update(void*)
{
    stepworld();

    if(dengine_input_get_key('1'))
        dl_ent_dl->shadow.max_bias -= 0.001;
    if(dengine_input_get_key('2'))
        dl_ent_dl->shadow.max_bias += 0.001;

    if(dengine_input_get_key('3'))
        dl_ent_dl->shadow_ortho -= 0.1;
    if(dengine_input_get_key('4'))
        dl_ent_dl->shadow_ortho += 0.1;

    if(dengine_input_get_key('5'))
        dl_ent_dl->shadow.far_shadow -= 0.1;
    if(dengine_input_get_key('6'))
        dl_ent_dl->shadow.far_shadow += 0.1;

    if(dengine_input_get_key_once('V'))
        dl_ent_dl->shadow.enable = !dl_ent_dl->shadow.enable;

    denginescene_update(scene);
    basic_update();

    int w, h;
    dengine_viewport_get(NULL, NULL, &w, &h);

    denginegui_panel(0, 0, w, h, cam_ent->camera_component->camera->framebuffer.color, NULL, black_f);
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
        "",
        "+++ SHADOWS +++",
        "V to toggle shadows",
        "Shadow properties:",
    };

    ShadowProp shadowprops[] =
    {
        {"Max Bias +/-0.001 (use 1 or 2)", &dl_ent_dl->shadow.max_bias},
        {"Orthosize +/-0.1 (use 3 or 4)", &dl_ent_dl->shadow_ortho},
        {"Far +/-0.1 (use 5 or 6)", &dl_ent_dl->shadow.far_shadow},
    };

    for(size_t i = 0; i < DENGINE_ARY_SZ(staticmessageslist); i++)
    {
        denginegui_text(fontsz / 4, h - fontsz - i * fontsz, staticmessageslist[i], NULL);
    }

    static vec4 orange = {1.0f, 0.5f, 0.3f, 1.0f};
    for(size_t i = 0; i < DENGINE_ARY_SZ(shadowprops); i++)
    {
        char msg[1024];
        snprintf(msg, sizeof(msg), "%s : %.3f", shadowprops[i].str, *shadowprops[i].val);
        denginegui_text(fontsz / 4, h - fontsz - i * fontsz - (float)DENGINE_ARY_SZ(staticmessageslist) * fontsz,  msg, orange);
    }
    vec2 pos2d_light;
    dengine_camera_world2screen(cam_ent->camera_component->camera, pl_ent->transform.position, pos2d_light);
    denginegui_panel(pos2d_light[0], pos2d_light[1], 30, 30, &pl_ent_gizmo, NULL, black_f);

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

    denginegui_panel(1024, 464, 256, 256, &dl_ent_dl->shadow.shadow_map.depth, NULL, black_f);
    return 1;
}
extern "C" int basic_world_terminate(void* args)
{
    delete[] prtbf;
    destroyworld();
    denginescene_destroy(scene);
    dengine_material_destroy(&plane_mat);
    dengine_material_destroy(&cube2_mat);
    return 1;
}

