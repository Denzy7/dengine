#include <dengine/dengine.h>
int main(int argc, char *argv[])
{
    DengineInitOpts* opts = dengine_init_get_opts();
    opts->window_title = "testdengine-script-nslbullet";
    dengine_init();

    NSL nsl = denginescript_nsl_load("nsl-bulletphysics.nsl");
    if(!nsl)
        return -1;

    const size_t prtbf_sz = 2048;
    char* prtbf = malloc(prtbf_sz);

    Script basic;
    denginescript_nsl_get_script("basic", &basic, nsl);

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

    Material cube_mat;
    dengine_material_setup(&cube_mat);
    dengine_material_set_shader_color(stdshdr, &cube_mat);

    MeshComponent* plane_mesh = denginescene_ecs_new_meshcomponent(&plane, &cube_mat);
    Entity* plane_ent = denginescene_ecs_new_entity();
    plane_ent->transform.position[0] = 0.0;
    plane_ent->transform.position[1] = 0.0;
    plane_ent->transform.position[2] = 0.0;
    plane_ent->transform.scale[0] = 15.0;
    plane_ent->transform.scale[1] = 0.25;
    plane_ent->transform.scale[2] = 15.0;
    plane_ent->mesh_component = plane_mesh;
    denginescene_add_entity(scene, plane_ent);

    MeshComponent* cube_mesh = denginescene_ecs_new_meshcomponent(&cube, &cube_mat);
    Entity* cube_ent = denginescene_ecs_new_entity();
    cube_ent->transform.position[0] = 0.0;
    cube_ent->transform.position[1] = 10.0;
    cube_ent->transform.position[2] = 0.0;
    cube_ent->transform.rotation[2] = 45.0f;
    cube_ent->transform.rotation[1] = -45.0f;
    cube_ent->transform.rotation[0] = -45.0f;
    cube_ent->mesh_component = cube_mesh;
    denginescene_add_entity(scene, cube_ent);
    denginescene_ecs_add_script(cube_ent, &basic);

    PointLight pl;
    memset(&pl, 0, sizeof(PointLight));
    dengine_lighting_light_setup(DENGINE_LIGHT_POINT, &pl);
    Entity* pl_ent = denginescene_ecs_new_entity();
    LightComponent* dl_ent_lightcomp = denginescene_ecs_new_lightcomponent(
                DENGINE_LIGHT_POINT, &pl);
    pl_ent->light_component = dl_ent_lightcomp;
    pl_ent->transform.position[0] = 0.0f;
    pl_ent->transform.position[1] = 5.0f;
    pl_ent->transform.position[2] = 0.0f;
    denginescene_add_entity(scene, pl_ent);

    denginescene_ecs_do_script_scene(scene, DENGINE_SCRIPT_FUNC_START);

    while (dengine_window_isrunning()) {
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

        denginegui_text(0, 0, (char*)glGetString(GL_VERSION), NULL);

        dengine_update();
    }
    denginescene_ecs_do_script_scene(scene, DENGINE_SCRIPT_FUNC_TERMINATE);

    denginescene_destroy(scene);
    free(prtbf);
    free(stdshdr);
    dengine_material_destroy(&cube_mat);
    denginescript_nsl_free(nsl);

    dengine_terminate();
    return 0;
}
