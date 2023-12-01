#include "denginebulletcommon.h"
#include <vector> //bruh
typedef struct
{
    const char* str;
    float* val;
}ShadowProp;

int car_update(Entity* entity);

extern unsigned int rover_obj_ln;
extern unsigned char rover_obj[];
const size_t prtbf_sz = 2048;
char* prtbf;
Scene* scene;
std::vector<Entity*> cubes;
DirLight* dl_ent_dl;
Entity* chassis;
Entity* cam_ent;
Primitive* car_meshes;
Shader stdshdr, shadow2d;
Primitive cube, plane;
Material mat_car, mat_wheels;
Material plane_mat, cube_mat;
Entity* pl_ent;
Texture pl_ent_gizmo;
float black_f[] =  {0.0f, 0.0f, 0.0f, 1.0f};
#ifdef DENGINE_ANDROID
#define SWBTNS
#endif

btDynamicsWorld* refworld;
const btVector3 wheel_dir(0, -1, 0);
const btVector3 wheel_axle(-1, 0, 0);
const btScalar wheel_rest(0.6);
btDefaultVehicleRaycaster* caster;
btRaycastVehicle* vehicle;
btRaycastVehicle::btVehicleTuning tuning;
btCollisionShape* carbox;
static int added = 0;
Entity* wheels[4];
static int dir_engine, dir_steer, dir_brake;
static int resetonce = 0;

int car_setup_wheel(Entity* entity)
{
    btVector3 connection(entity->transform.position[0],
                         entity->transform.position[1] - 4.0,
                         entity->transform.position[2]);
    btWheelInfo& info = vehicle->addWheel(connection, wheel_dir, wheel_axle, wheel_rest, 1.5, tuning, added < 2);
    info.m_suspensionStiffness = 20.0f;
    info.m_wheelsDampingRelaxation = 2.3f;
    info.m_wheelsDampingCompression = 4.4f;
    info.m_frictionSlip = 10.f;
    info.m_rollInfluence = 0.02f;
    wheels[added] = entity;
    added++;
    return 1;
}

int car_setup_chassis(Entity* entity)
{
    btRigidBody* carbody;

    carbox = new btBoxShape(btVector3(2.0, 0.5, 4.0));

    btTransform local;
    const float carmass = 800.0;
    local.setIdentity();
    local.setOrigin(btVector3(entity->transform.position[0],
                              entity->transform.position[1],
                              entity->transform.position[2]));

    btDefaultMotionState* state = new btDefaultMotionState(local);
    btVector3 inertia(0, 0, 0);
    carbox->calculateLocalInertia(carmass, inertia);

    btRigidBody::btRigidBodyConstructionInfo info(carmass, state, carbox, inertia);
    carbody = new btRigidBody(info);
    refworld->addRigidBody(carbody);

    caster = new btDefaultVehicleRaycaster(refworld);
    vehicle = new btRaycastVehicle(tuning, carbody, caster);

    carbody->setActivationState(DISABLE_DEACTIVATION);
    refworld->addVehicle(vehicle);
    vehicle->setCoordinateSystem(0, 1, 2);

    return 1;
}
void tickcbcar(btDynamicsWorld* mWorld, btScalar tick)
{
    car_update(chassis);
}

void drawcubes()
{
    const Material* mat = &cube_mat;
    const Camera* cam = cam_ent->camera_component->camera;
    const Primitive* primitive = &cube;
    const Shader* shader = &stdshdr;

    dengine_material_use(mat);
    dengine_camera_use(cam);
    dengine_camera_apply(&mat->shader_color, cam);
    dengine_lighting_apply_dirlight(dl_ent_dl, shader);
    dengine_lighting_apply_pointlight((PointLight*)pl_ent->light_component->light, shader);
    dengine_draw_sequence_start(primitive, shader);
    for(size_t i = 0; i < cubes.size(); i++)
    {
        Entity* ent = cubes.at(i);
        denginescene_ecs_transform_entity(ent);
        dengine_shader_set_mat4(&mat->shader_color,
                "model",
                ent->transform.world_model[0]
                );
        dengine_draw_sequence_draw();
    }
    dengine_draw_sequence_end();
    dengine_camera_use(NULL);
}

extern "C" int car_world_start(void* arg)
{    
    initworld(&refworld);
    refworld->setInternalTickCallback(tickcbcar);

    prtbf = new char[prtbf_sz];
    scene = denginescene_new();

    Camera cam;
    dengine_camera_setup(&cam);
    cam.far = 200.0;
    cam.clearonuse = 0;
    dengine_camera_set_rendermode(DENGINE_CAMERA_RENDER_FOWARD, &cam);
    cam.clearcolor[0] = 0.1f;
    cam.clearcolor[1] = 0.1f;
    cam.clearcolor[2] = 0.1f;
    cam.clearcolor[3] = 1.0f;

    cam_ent = denginescene_ecs_new_entity();
    cam_ent->camera_component = denginescene_ecs_new_cameracomponent(&cam);
    cam_ent->transform.rotation[0] = -35.0f;
    cam_ent->transform.rotation[1] = 225.0f;

    denginescene_add_entity(scene, cam_ent);

    dengine_shader_make_standard(DENGINE_SHADER_STANDARD, &stdshdr);
    dengine_shader_make_standard(DENGINE_SHADER_SHADOW2D, &shadow2d);

    dengine_primitive_gen_cube(&cube, &stdshdr);
    dengine_primitive_gen_plane(&plane, &stdshdr);

    dengine_material_setup(&plane_mat);
    dengine_material_set_shader_color(&stdshdr, &plane_mat);
    dengine_material_set_shader_shadow(&shadow2d, &plane_mat);

    dengine_material_setup(&cube_mat);
    dengine_material_set_shader_color(&stdshdr, &cube_mat);

    static const char* texs_plane[][2]=
    {
        {"textures/2d/plane_diff.png", "diffuseTex"},
        {"textures/2d/plane_spec.png", "specularTex"}
    };
    Texture tex_plane[2];
    memset(&tex_plane, 0, sizeof(tex_plane));
    for(int i = 0; i < DENGINE_ARY_SZ(texs_plane); i++)
    {
        void* texmem;
        size_t texmem_len;
        tex_plane[i].auto_dataonload = 1;
        tex_plane[i].interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
        dengine_load_asset(texs_plane[i][0], &texmem, &texmem_len);
        dengine_texture_load_mem(texmem, texmem_len, 1, &tex_plane[i]);
        free(texmem);
        dengine_material_set_texture(&tex_plane[i], texs_plane[i][1], &plane_mat);
    }

    MeshComponent* plane_mesh = denginescene_ecs_new_meshcomponent(&plane, &plane_mat);
    Entity* plane_ent = denginescene_ecs_new_entity();
    plane_ent->transform.position[0] = 0.0;
    plane_ent->transform.position[1] = 0.0;
    plane_ent->transform.position[2] = 0.0;
    plane_ent->transform.scale[0] = 75.0;
    plane_ent->transform.scale[1] = 0.25;
    plane_ent->transform.scale[2] = 75.0;
    plane_ent->mesh_component = plane_mesh;
    create_rb(plane_ent, DENGINE_ECS_PHYSICS_COLSHAPE_BOX, NULL, 0.0, NULL);
    denginescene_add_entity(scene, plane_ent);

    /* we'll have to render these in a single pass, adding them to scene will degrade performance
     * since each will have its own mesh and material
     */
    int gencoef = 5;
    for(int i = -gencoef; i < gencoef; i++)
    {
        for(int j = -gencoef; j < gencoef; j++)
        {
            Entity* cube_ent = denginescene_ecs_new_entity();
            cube_ent->transform.scale[0] = 3.0;
            cube_ent->transform.scale[1] = 0.4;
            cube_ent->transform.scale[2] = 3.0;
            cube_ent->transform.position[0] = i + (i * 15);
            cube_ent->transform.position[1] = abs((i * j)) / (float)gencoef;
            cube_ent->transform.position[2] = j + (j * 10);
            create_rb(cube_ent, DENGINE_ECS_PHYSICS_COLSHAPE_BOX, NULL, 0.0, NULL);
            cubes.push_back(cube_ent);
        }
    }
    /* car */
    Texture tex_black, tex_random;
    dengine_texture_make_color(2, 2, black_f, 3, &tex_black);
    static float randcol[3];

    for(int i = 0; i < 3; i++)
    {
        randcol[i] = dengineutils_rng_int(256) / 256.0;
    }
    dengine_texture_make_color(2, 2, randcol, 3, &tex_random);

    /* mesh */
    size_t car_meshes_n;

    car_meshes = denginemodel_load_mem(DENGINE_MODEL_FORMAT_OBJ, rover_obj, rover_obj_ln,
            &car_meshes_n,&stdshdr);

    /* material */

    dengine_material_setup(&mat_car);
    dengine_material_set_shader_color(&stdshdr, &mat_car);
    dengine_material_set_shader_shadow(&shadow2d, &mat_car);

    dengine_material_set_texture(&tex_random, "diffuseTex", &mat_car);

    dengine_material_setup(&mat_wheels);
    dengine_material_set_shader_color(&stdshdr, &mat_wheels);
    dengine_material_set_shader_shadow(&shadow2d, &mat_wheels);

    dengine_material_set_texture(&tex_black, "diffuseTex", &mat_wheels);

//    for(size_t i = 0; i < 4; i++)
//    {
//        Entity* e = denginescene_ecs_new_entity();
//        e->transform.position[1] = 0.0;
//        e->mesh_component = denginescene_ecs_new_meshcomponent(&car_meshes[i], &mat_car);
//        denginescene_add_entity(scene, e);
//    }
    chassis = denginescene_ecs_new_entity();
    //denginescene_ecs_add_script(chassis, &car);

    chassis->transform.manualtransform = 1;
    chassis->transform.position[1] = 4.0;
    chassis->mesh_component = denginescene_ecs_new_meshcomponent(&car_meshes[2], &mat_car);

    car_setup_chassis(chassis);
    denginescene_add_entity(scene, chassis);

    vec3 wheel_poses[] =
    {
      {3.0, 4.0, 3.0},
      {-3.0, 4.0, 3.0},
      {3.0, 4.0, -3.0},
      {-3.0, 4.0, -3.0}
    };

    Primitive* wheel_meshes[]=
    {
        &car_meshes[3],
        &car_meshes[0],
        &car_meshes[4],
        &car_meshes[1],
    };

    for(int i = 0; i < 4; i++)
    {
        Entity* wheel = denginescene_ecs_new_entity();
        wheel->transform.manualtransform = 1;
        memcpy(wheel->transform.position, wheel_poses[i], sizeof(vec3));
        wheel->mesh_component = denginescene_ecs_new_meshcomponent(wheel_meshes[i], &mat_wheels);
        car_setup_wheel(wheel);
        denginescene_add_entity(scene, wheel);
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
    dl.shadow.shadow_map_size = 1024;
    dengine_lighting_light_setup(DENGINE_LIGHT_DIR, &dl);
//    dl.shadow.pcf = 1;
    dl.light.strength*= 2.f;
    dl.shadow.far_shadow = 175.0f;
    dl.shadow_ortho = 125.0f;
    Entity* dl_ent = denginescene_ecs_new_entity();
    dl_ent->transform.position[0] = -50.0f;
    dl_ent->transform.position[1] = 50.0f;
    dl_ent->transform.position[2] = 50.0f;
    dl_ent->light_component = denginescene_ecs_new_lightcomponent(DENGINE_LIGHT_DIR, &dl);
    denginescene_add_entity(scene, dl_ent);
    dl_ent_dl = (DirLight*)dl_ent->light_component->light;

    startworld();

    return 1;
}

int car_update(Entity* entity)
{
#ifndef SWBTNS
    if(dengine_input_get_key('W'))
        dir_engine = 1;
    else if(dengine_input_get_key('S'))
        dir_engine = -1;
    else
        dir_engine = 0;

    if(dengine_input_get_key('D'))
        dir_steer = -1;
    else if(dengine_input_get_key('A'))
        dir_steer = 1;
    else
        dir_steer = 0;

    if(dengine_input_get_key('X'))
        dir_brake = 1;
    else
        dir_brake = 0;

#endif

    phy2ent(vehicle->getChassisWorldTransform(), entity);

    if(dengine_input_get_key_once('R'))
    {
        resetonce = 1;
    }

    if(resetonce)
    {
        resetonce = 0;
        btTransform transform;
        transform.setIdentity();

        transform.setOrigin(btVector3(0, 5, 0));
        transform.setRotation(btQuaternion(0, 0, 0));

        vehicle->getRigidBody()->setWorldTransform(transform);
        vehicle->getRigidBody()->getMotionState()->setWorldTransform(transform);

        vehicle->getRigidBody()->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
        vehicle->getRigidBody()->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
        vehicle->getRigidBody()->clearForces();
    }
    for(int i = 0; i < 4; i++)
    {
        if(i == 0 || i == 1){
            vehicle->setSteeringValue(0.5 * dir_steer, i);
        }else
        {
            vehicle->setBrake(200.0 * dir_brake, i);
        }

        vehicle->applyEngineForce(1750.0 * dir_engine, i);

        phy2ent(vehicle->getWheelTransformWS(i), wheels[i]);
    }

    return 1;
}

extern "C" int car_world_update(void* arg)
{
    stepworld();

    double delta = dengineutils_timer_get_delta();

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
    drawcubes();

    static float camdist = 40.0f;
    if(dengine_input_get_key('E'))
        camdist += 10.0 * (delta / 1000.0);
    if(dengine_input_get_key('C'))
        camdist -= 10.0 * (delta / 1000.0);

    memcpy(cam_ent->transform.position, chassis->transform.position, sizeof(vec3));
    glm_vec3_adds(cam_ent->transform.position, camdist, cam_ent->transform.position);

    int w, h;
    dengine_viewport_get(NULL, NULL, &w, &h);

    denginegui_panel(0, 0, w, h, cam_ent->camera_component->camera->framebuffer.color, NULL, black_f);
    float fontsz = denginegui_get_fontsz();
    static const char* staticmessageslist[] =
    {
        "Press W/S to accelarate/brake",
        "A/D to steer left/right",
        "X to brake",
        "R to reset position",
        "E/C increase/decrease camera distance",
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

    for(int i = 0; i < DENGINE_ARY_SZ(staticmessageslist); i++)
    {
        denginegui_text(fontsz / 4, h - fontsz - i * fontsz, staticmessageslist[i], NULL);
    }

    static vec4 orange = {1.0f, 0.5f, 0.3f, 1.0f};
    for(int i = 0; i < DENGINE_ARY_SZ(shadowprops); i++)
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

    static double elapsed = 0.0;
    elapsed += delta;
    if(elapsed > 1000.0){
        snprintf(fpstr, sizeof (fpstr), "%s :: FPS : %.1f(%.2fms)", (char*)glGetString(GL_VERSION), 1 / (delta / 1000.0), delta);
        elapsed = 0.0;
    }

    int shadowdgbsz = 128;
    denginegui_panel(w - shadowdgbsz - fontsz, h - shadowdgbsz - fontsz,
            shadowdgbsz, shadowdgbsz, &dl_ent_dl->shadow.shadow_map.depth, NULL, black_f);
    denginegui_text(fontsz, fontsz, fpstr, yellow);

    char speed[128];
    snprintf(speed, sizeof(speed), "Speed: %.1f Km/h", vehicle->getCurrentSpeedKmHour());

    denginegui_text(fontsz, fontsz * 2, speed, NULL);

#ifdef SWBTNS
    float btnoffset = 30;
    float btnwidth = 200;
    float btnheight = 200;
    float btnspace = 10;
    denginegui_set_button_repeatable(1);
    dir_steer = 0;
    if(denginegui_button(btnoffset, (h / 2.0) - (btnheight / 2.0), btnwidth, btnheight, "A", NULL))
    {
        dir_steer = 1;
    }
    if(denginegui_button(w - btnoffset - btnwidth, (h / 2.0) - (btnheight / 2.0), btnwidth, btnheight, "D", NULL))
    {
        dir_steer = -1;
    }

    dir_engine = 0;
    if(denginegui_button(w - btnoffset - btnwidth, btnoffset, btnwidth, btnheight, "W", NULL))
    {
        dir_engine = 1;
    }
    if(denginegui_button(w - btnoffset - (2.0 * btnwidth) - (2.0 * btnspace), btnoffset, btnwidth, btnheight, "S", NULL))
    {
        dir_engine = -1;
    }

    if(denginegui_button(w - btnoffset - (3.0 * btnwidth) - (3.0 * btnspace), btnoffset, btnwidth, btnheight, "X", NULL))
    {
        dir_brake = 1;
    }else{
        dir_brake = 0;
    } 

    btnwidth /= 2.0f;
    btnoffset /= 2.0f;
    denginegui_set_button_repeatable(0);
    if(denginegui_button(w - fontsz - shadowdgbsz - btnoffset - btnwidth
                , h - fontsz - (btnheight / 2.0f), btnwidth, btnheight / 2.0f, "R", NULL))
        resetonce = 1;

    if(denginegui_button(w - fontsz - shadowdgbsz - btnoffset - btnwidth
                , h - fontsz - btnoffset - btnheight, btnwidth, btnheight / 2.0f, "V", NULL)) 
        dl_ent_dl->shadow.enable = !dl_ent_dl->shadow.enable;
    denginegui_set_button_repeatable(1);
    if(denginegui_button(w - fontsz - shadowdgbsz - (2.0f * btnoffset) - (2.0f * btnwidth)
                , h - fontsz - (btnheight / 2.0f), btnwidth, btnheight / 2.0f, "E", NULL))  
        camdist += 10.0 * (delta / 1000.0);

    if(denginegui_button(w - fontsz - shadowdgbsz - (2.0f * btnoffset) - (2.0f * btnwidth)
                , h - fontsz - btnoffset - btnheight, btnwidth, btnheight / 2.0f, "C", NULL)) 
        camdist -= 10.0 * (delta / 1000.0);


    denginegui_set_button_repeatable(0);

    #endif

    return 1;
}

extern "C" int car_world_terminate(void* args)
{
    denginescene_destroy(scene);
    free(prtbf);
    free(car_meshes);
    dengine_material_destroy(&plane_mat);
    dengine_material_destroy(&mat_car);
    dengine_material_destroy(&cube_mat);
    dengine_material_destroy(&mat_wheels);
    delete carbox;
    delete vehicle;
    delete caster;
    destroyworld();

    return 1;
}

