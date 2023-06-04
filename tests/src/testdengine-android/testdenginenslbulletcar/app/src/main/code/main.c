#include "dengine-utils/logging.h"
#include "dengine-utils/stream.h"
#include "dengine/camera.h"
#include "dengine/texture.h"
#include <dengine/dengine.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    const char* str;
    float* val;
}ShadowProp;

extern unsigned int rover_obj_ln;
extern unsigned char rover_obj[];

Script car, car_world;
Scene* scene;
const size_t prtbf_sz = 2048;
char* prtbf;
Primitive* car_meshes;
Material mat_car, mat_wheels, plane_mat, cube_mat;;
NSL nsl;
DirLight* dl_ent_dl;
Entity* cam_ent,* chassis,* pl_ent;
Texture pl_ent_gizmo;
DirLight dl;
nslfunc car_speed;
ECSPhysicsWorld world;
static void init(struct android_app* app)
{
    if(dengine_init())
    {
        nsl = denginescript_nsl_load("libnsl_bulletcar_android.so");

        nslfunc car_create_rb = denginescript_nsl_get_func(nsl, "car_create_rb");
        nslfunc car_setup_chassis = denginescript_nsl_get_func(nsl, "car_setup_chassis");
        nslfunc car_setup_wheel = denginescript_nsl_get_func(nsl, "car_setup_wheel");
        car_speed = denginescript_nsl_get_func(nsl, "car_speed");
        
        prtbf = malloc(prtbf_sz);

        denginescript_nsl_get_script("car", &car, nsl);
        denginescript_nsl_get_script("car_world", &car_world, nsl);

        
        world.substeps = 1;
        world.timestep_fixed = 1.f/ 60.f;

        //manually call start to create world
        denginescript_call(&car_world, DENGINE_SCRIPT_FUNC_START, &world);

        scene = denginescene_new();

        Camera cam;
        dengine_camera_setup(&cam);
        // const float camscalerandroid = 0.4;
        // cam.render_width *= camscalerandroid;
        // cam.render_height *= camscalerandroid;
        // dengineutils_logging_log("%d %d", cam.render_width, cam.render_height);
        // dengine_camera_resize(&cam, cam.render_width,cam.render_height);
        cam.far = 200.0;
        dengine_camera_set_rendermode(DENGINE_CAMERA_RENDER_FOWARD, &cam);
        cam.clearcolor[0] = 0.1f;
        cam.clearcolor[1] = 0.1f;
        cam.clearcolor[2] = 0.1f;
        cam.clearcolor[3] = 0.1f;

        CameraComponent* cam_c = denginescene_ecs_new_cameracomponent(&cam);
        cam_ent = denginescene_ecs_new_entity();
        cam_ent->camera_component = cam_c;
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
            tex_plane[i].auto_dataonload = 1;
            tex_plane[i].interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
            Stream* stream = dengineutils_stream_new(texs_plane[i][0],DENGINEUTILS_STREAM_TYPE_ANDROIDASSET, DENGINEUTILS_STREAM_MODE_BUFFER);
            dengine_texture_load_mem(stream->buffer, stream->size, 1, &tex_plane[i]);
            dengine_material_set_texture(&tex_plane[i], texs_plane[i][1], &plane_mat);
            dengineutils_stream_destroy(stream);
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
        plane_ent->physics_component = denginescene_ecs_new_physicscomponent(DENGINE_ECS_PHYSICS_COLSHAPE_BOX, NULL, 0.0);
        car_create_rb(plane_ent);
        denginescene_add_entity(scene, plane_ent);

        int gencoef = 3;
        for(int i = -gencoef; i < gencoef; i++)
        {
            for(int j = -gencoef; j < gencoef; j++)
            {
                Entity* cube_ent = denginescene_ecs_new_entity();
                cube_ent->mesh_component = denginescene_ecs_new_meshcomponent(&cube, &cube_mat);
                cube_ent->transform.scale[0] = 3.0;
                cube_ent->transform.scale[1] = 0.4;
                cube_ent->transform.scale[2] = 3.0;
                cube_ent->transform.position[0] = i + (i * 15);
                cube_ent->transform.position[1] = abs((i * j)) / (float)gencoef;
                cube_ent->transform.position[2] = j + (j * 10);
                cube_ent->physics_component = denginescene_ecs_new_physicscomponent(DENGINE_ECS_PHYSICS_COLSHAPE_CAPSULE, NULL, 0.0);
                car_create_rb(cube_ent);
                denginescene_add_entity(scene, cube_ent);
            }
        }

        /* car */
        Texture tex_black, tex_random;
        dengine_texture_make_color(2, 2, GLM_VEC4_BLACK, 3, &tex_black);
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
        denginescene_ecs_add_script(chassis, &car);

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

        
        memset(&dl, 0, sizeof(DirLight));
        //uncomment for laggy android shadows
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
        dl_ent_dl= dl_ent->light_component->light;
    }
}


static void term(struct  android_app* app)
{
    //destroy world
    denginescript_call(&car_world, DENGINE_SCRIPT_FUNC_TERMINATE, NULL);

    denginescene_destroy(scene);
    free(prtbf);
    free(car_meshes);
    dengine_material_destroy(&plane_mat);
    dengine_material_destroy(&mat_car);
    dengine_material_destroy(&cube_mat);
    denginescript_nsl_free(nsl);

    dengine_terminate();
}

static void draw()
{
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

    //stepSimulation
    denginescript_call(&car_world, DENGINE_SCRIPT_FUNC_UPDATE, &world);

    denginescene_update(scene);

    static float camdist = 40.0f;
    if(dengine_input_get_key('E'))
        camdist += 10.0 * (delta / 1000.0);
    if(dengine_input_get_key('C'))
        camdist -= 10.0 * (delta / 1000.0);

    memcpy(cam_ent->transform.position, chassis->transform.position, sizeof(vec3));
    glm_vec3_adds(cam_ent->transform.position, camdist, cam_ent->transform.position);

    int w, h;
    dengine_viewport_get(NULL, NULL, &w, &h);

    denginegui_panel(0, 0, w, h, cam_ent->camera_component->camera->framebuffer.color, NULL, GLM_VEC4_BLACK);
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
        denginegui_text(fontsz / 4, h - fontsz - i * fontsz - DENGINE_ARY_SZ(staticmessageslist) * fontsz,  msg, orange);
    }
    vec2 pos2d_light;
    dengine_camera_world2screen(cam_ent->camera_component->camera, pl_ent->transform.position, pos2d_light);
    denginegui_panel(pos2d_light[0], pos2d_light[1], 30, 30, &pl_ent_gizmo, NULL, GLM_VEC4_BLACK);

    static char fpstr[100];
    static vec4 yellow = {1.0, 1.0, 0.0, 1.0};

    static double elapsed = 0.0;
    elapsed += delta;
    if(elapsed > 1000.0){
        snprintf(fpstr, sizeof (fpstr), "%s :: FPS : %.1f(%.2fms)", (char*)glGetString(GL_VERSION), 1 / (delta / 1000.0), delta);
        elapsed = 0.0;
    }

    denginegui_text(fontsz, fontsz, fpstr, yellow);

    car_speed(NULL);

    // denginegui_panel(1024, 464, 256, 256, &dl.shadow.shadow_map.depth, NULL, GLM_VEC4_BLACK);
    dengine_update();
}

void android_main(struct android_app* state)
{
    //Set app callbacks
    dengineutils_android_set_app(state);
    dengineutils_android_set_initfunc(init);
    dengineutils_android_set_terminatefunc(term);

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

        if(DENGINE_WINDOW_CURRENT)
        	draw();
    }
}