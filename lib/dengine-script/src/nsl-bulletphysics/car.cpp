#include "denginebulletcommon.h"
#include <vector> //bruh
typedef struct
{
    const char* str;
    float* val;
}ShadowProp;

void car_tickcb(btDynamicsWorld* world, float ts);
int car_update(Entity* entity);

extern unsigned int rover_obj_ln;
extern unsigned char rover_obj[];
extern unsigned int wheelmap_jpg_ln;
extern unsigned char wheelmap_jpg[];

static const size_t prtbf_sz = 2048;
char* prtbf;
Scene* scene;
int cubes_draw = 1;
int physics_toggle = 1;
int debugui = 1;
std::vector<Entity*> cubes;
std::vector<ECSPhysicsBody> stacks;
DirLight* dl_ent_dl;
Entity* chassis;
Entity* cam_ent;
Primitive* car_meshes;
Shader stdshdr, shadow2d,sky2d;
Primitive cube, plane;
Material mat_car, mat_wheels, skymat;
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
static float dir_engine, dir_steer, dir_brake;
static int resetonce = 0;

int car_setup_wheel(Entity* entity)
{
    btVector3 connection(entity->transform.position[0],
                         entity->transform.position[1] - 4.0,
                         entity->transform.position[2]);
    btWheelInfo& info = vehicle->addWheel(connection, wheel_dir, wheel_axle, wheel_rest, 1.5, tuning, added < 2);
    info.m_suspensionStiffness = 15.0f;
    info.m_wheelsDampingRelaxation = 0.85f;
    info.m_wheelsDampingCompression = 0.55f;
    if(added < 2 )
        info.m_frictionSlip = 2.0f;
    else
        info.m_frictionSlip = 1.5f;
    info.m_rollInfluence = 0.1f;
    wheels[added] = entity;
    added++;
    return 1;
}

int car_setup_chassis(Entity* entity)
{
    btRigidBody* carbody;

    carbox = new btBoxShape(btVector3(3.0, 0.5, 4.0));

    btTransform local;
    const float carmass = 750.0;
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
        dengine_shader_current_set_mat4("model", ent->transform.world_model[0]);
        dengine_draw_sequence_draw();
    }
    dengine_draw_sequence_end();
    dengine_camera_use(NULL);
}

extern "C" int car_world_start(void* arg)
{    
    initworld(&refworld);
    add_tickcb(car_tickcb);
    set_timescale(1.125);

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
    dengine_shader_make_standard(DENGINE_SHADER_SKYBOX2D, &sky2d);

    dengine_primitive_gen_cube(&cube, &stdshdr);
    dengine_primitive_gen_plane(&plane, &stdshdr);

    dengine_material_setup(&plane_mat);
    dengine_material_set_shader_color(&stdshdr, &plane_mat);
    dengine_material_set_shader_shadow(&shadow2d, &plane_mat);

    dengine_material_setup(&cube_mat);
    dengine_material_set_shader_color(&stdshdr, &cube_mat);

    dengine_material_setup(&skymat);
    dengine_material_set_shader_color(&sky2d, &skymat);

    void* skytexmem;
    size_t skytexmem_ln;
    Texture skytex;
    memset(&skytex, 0, sizeof(skytex));
    skytex.auto_dataonload = 1;
    if(dengine_texture_issupprorted(GL_TEXTURE_2D, GL_FLOAT, GL_RGB, GL_RGB))
    {
        skytex.interface = DENGINE_TEXTURE_INTERFACE_FLOAT;
        skytex.type = GL_FLOAT;
        /* OES_texture_float.txt wants nearest */
        skytex.filter_min = GL_NEAREST;
        skytex.filter_mag = GL_NEAREST;
    }else
    {
        dengineutils_logging_log("WARNING::skybox has been converted to lower precision 8 bit!");
        skytex.interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
        skytex.type = GL_UNSIGNED_BYTE;
    }
    dengine_load_asset("textures/hdri/sunset.hdr", &skytexmem, &skytexmem_ln);
    dengine_texture_load_mem(skytexmem, skytexmem_ln, 1, &skytex);
    free(skytexmem);
    dengine_material_set_texture(&skytex, "eqireqMap", &skymat);
    scene->skybox = denginescene_new_skybox(&cube, &skymat);


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
    plane_ent->transform.scale[0] = 80.0;
    plane_ent->transform.scale[1] = 0.25;
    plane_ent->transform.scale[2] = 80.0;
    plane_ent->mesh_component = plane_mesh;
    create_rb(plane_ent, DENGINE_ECS_PHYSICS_COLSHAPE_BOX, NULL, 0.0, NULL);
    denginescene_add_entity(scene, plane_ent);

    /* we'll have to render these in a single pass, adding them to scene will degrade performance
     * since each will have its own mesh and material
     */
    int gencoef = 3;
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
    /* walls **/
    float wallheight = 2.5f;
    float walllength = plane_ent->transform.scale[0];
    float wallthicl = 0.5f;
    static const float wallpos[][2] = 
    {
        {-1.0f,  0.0f},
        { 1.0f,  0.0f},
        { 0.0f, -1.0f},
        { 0.0f,  1.0f}
    };
    for(int i = 0; i < 4; i++)
    {
        Entity* wall = denginescene_ecs_new_entity();
        wall->transform.scale[0] = wallthicl; 
        wall->transform.scale[1] = wallheight;
        wall->transform.scale[2] = walllength;
        wall->transform.position[0] = wallpos[i][0] * walllength;
        wall->transform.position[1] = wallheight;
        wall->transform.position[2] = wallpos[i][1] * walllength;
        if(i > 1)
            wall->transform.rotation[1] += 90.0f;
        create_rb(wall, DENGINE_ECS_PHYSICS_COLSHAPE_BOX, NULL, 0.0, NULL);
        cubes.push_back(wall);
    }

    for(int i = 1; i < 10; i++)
    {
        Entity* cubestack = denginescene_ecs_new_entity();
        btRigidBody* bodyref;
        glm_vec3_scale(cubestack->transform.scale, 2.0, cubestack->transform.scale);
        cubestack->transform.position[1] += i + (i * 2.0);
        cubestack->transform.position[2] = 10.0;
        create_rb(cubestack, DENGINE_ECS_PHYSICS_COLSHAPE_BOX, NULL, 1.0, &bodyref);
        ECSPhysicsBody epb;
        epb.ent = cubestack;
        epb.body = bodyref;
        stacks.push_back(epb);
        cubes.push_back(cubestack);
    }
    /* car */
    Texture tex_black, tex_random, wheelmap;
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

    memset(&wheelmap, 0, sizeof(wheelmap));
    wheelmap.interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
    wheelmap.auto_dataonload = 1;
    dengine_texture_load_mem(wheelmap_jpg, wheelmap_jpg_ln, 1, &wheelmap);

    dengine_material_set_texture(&wheelmap, "diffuseTex", &mat_wheels);

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
    chassis->mesh_component = denginescene_ecs_new_meshcomponent(&car_meshes[0], &mat_car);

    car_setup_chassis(chassis);
    denginescene_add_entity(scene, chassis);
    static const float axlediff = 3.4f;
    static const float suspensionpoint = 4.0f;

    vec3 wheel_poses[] =
    {
      {axlediff, suspensionpoint, axlediff},
      {-axlediff, suspensionpoint, axlediff},
      {axlediff, suspensionpoint, -axlediff},
      {-axlediff, suspensionpoint, -axlediff}
    };

    for(int i = 0; i < 4; i++)
    {
        Entity* wheel = denginescene_ecs_new_entity();
        Entity* wheelroot = denginescene_ecs_new_entity();
        memcpy(wheelroot->transform.position, wheel_poses[i], sizeof(vec3));
        if(i % 2 == 0)
            wheel->transform.rotation[1] = 0;
        else
            wheel->transform.rotation[1] = 180;
        wheel->mesh_component = denginescene_ecs_new_meshcomponent(&car_meshes[1], &mat_wheels);
        car_setup_wheel(wheelroot);
        denginescene_ecs_parent(wheelroot, wheel);
        denginescene_add_entity(scene, wheelroot);
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

    return 1;
}

int car_update(Entity* entity)
{
    phy2ent(vehicle->getChassisWorldTransform(), entity);

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
            /* 4x4 steer */
            vehicle->setSteeringValue(0.5 * -dir_steer * dir_brake, i);
        }
        vehicle->setBrake(100.0 * dir_brake, i);
        vehicle->applyEngineForce(1500.0 * dir_engine, i);

        phy2ent(vehicle->getWheelTransformWS(i), wheels[i]);
    }

    return 1;
}

extern "C" int car_world_update(void* arg)
{
    if(physics_toggle)
        stepworld();

    double delta = dengineutils_timer_get_delta();
    double delta_s = delta / 1000.0;

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
    if(dengine_input_get_key_once('B'))
        cubes_draw = !cubes_draw;
    if(dengine_input_get_key_once('P'))
        physics_toggle = !physics_toggle;
    if(dengine_input_get_key_once('G'))
        debugui = !debugui;

    static const float lerpspeed = 5.0f;
#ifndef SWBTNS
    if(dengine_input_get_key('W'))
        dir_engine = 1;
    else if(dengine_input_get_key('S'))
        dir_engine = -1;
    else
        dir_engine = 0;

    if(dengine_input_get_key('D'))
        dir_steer = glm_lerp(dir_steer, -1.0f, delta_s * lerpspeed);
    else if(dengine_input_get_key('A'))
        dir_steer = glm_lerp(dir_steer, 1.0f, delta_s * lerpspeed);
    else 
        dir_steer = glm_lerp(dir_steer, 0.0f, delta_s * lerpspeed);

    if(dengine_input_get_key('X'))
        dir_brake = 1;
    else
        dir_brake = 0;

    if(dengine_input_get_key_once('R'))
    {
        resetonce = 1;
    }

#endif

    denginescene_update(scene);
    if(cubes_draw)
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
        "X to brake or 4x4 steer",
        "R to reset position",
        "E/C increase/decrease camera distance",
        "B toggle visual cubes",
        "P toggle physics",
        "G toggle debug UI",
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

    int shadowdgbsz = 128;
    static vec4 yellow = {1.0, 1.0, 0.0, 1.0};
    if(debugui){

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


    denginegui_panel(w - shadowdgbsz - fontsz, h - shadowdgbsz - fontsz,
            shadowdgbsz, shadowdgbsz, &dl_ent_dl->shadow.shadow_map.depth, NULL, black_f);

    }

    static char fpstr[100];
    static double elapsed = 0.0;
    static uint32_t frames = 0;
    elapsed += delta;
    frames++;
    if(elapsed > 1000.0){
        snprintf(fpstr, sizeof (fpstr), "%s :: FPS : %d(%.2fms)", (char*)glGetString(GL_VERSION), frames, delta);
        elapsed = 0.0;
        frames = 0;
    }
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
    int down = 0;
    if(denginegui_button(btnoffset, (h / 3.75) - (btnheight / 2.0), btnwidth, btnheight, "A", NULL))
    {
        down = 1;
        dir_steer = glm_lerp(dir_steer, 1.0f, delta_s * lerpspeed);
    }
    if(denginegui_button(btnwidth + (2.0 * btnoffset), (h / 3.75) - (btnheight / 2.0), btnwidth, btnheight, "D", NULL))
    {
        down = 1;
        dir_steer = glm_lerp(dir_steer, -1.0f, delta_s * lerpspeed);
    }

    if (!down)
        dir_steer = glm_lerp(dir_steer, 0.0f, delta_s * lerpspeed);

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
    if(denginegui_button(w - fontsz - shadowdgbsz - (3.0f * btnoffset) - (3.0f * btnwidth)
                , h - fontsz - (btnheight / 2.0f), btnwidth, btnheight / 2.0f, "B", NULL))  
        cubes_draw = !cubes_draw;
    if(denginegui_button(w - fontsz - shadowdgbsz - (4.0f * btnoffset) - (4.0f * btnwidth)
                , h - fontsz - (btnheight / 2.0f), btnwidth, btnheight / 2.0f, "P", NULL))  
        physics_toggle = !physics_toggle;
    if(denginegui_button(w - fontsz - shadowdgbsz - (5.0f * btnoffset) - (5.0f * btnwidth)
                , h - fontsz - (btnheight / 2.0f), btnwidth, btnheight / 2.0f, "G", NULL))  
        debugui = !debugui;

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
    for(size_t i = 0; i < cubes.size(); i++)
    {
        Entity* e = cubes.at(i);
        denginescene_ecs_destroy_entity(e);
    }
    delete[] prtbf;
    free(car_meshes);
    dengine_shader_destroy(&stdshdr);
    dengine_shader_destroy(&shadow2d);
    dengine_material_destroy(&plane_mat);
    dengine_material_destroy(&mat_car);
    dengine_material_destroy(&cube_mat);
    dengine_material_destroy(&mat_wheels);
    dengine_material_destroy(&skymat);
    delete carbox;
    delete vehicle;
    delete caster;
    destroyworld();


    return 1;
}

void car_tickcb(btDynamicsWorld* world, float ts)
{
    car_update(chassis);
    for(size_t i = 0; i < stacks.size(); i++)
    {
        ECSPhysicsBody epb = stacks[i];
        phy2ent(epb.body->getWorldTransform(), epb.ent);
    }
}
