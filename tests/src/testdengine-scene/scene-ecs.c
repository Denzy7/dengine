#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <dengine-scene/ecs.h>
#include <dengine-scene/scene.h>

#include <dengine-utils/logging.h>
#include <dengine-utils/timer.h>

#include <dengine/input.h>
#include <dengine/window.h>
#include <dengine/draw.h>
#include <dengine/camera.h>

#include <dengine-gui/gui.h>

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

    Window window;
    dengine_window_init();
    dengine_window_create(1280,720,"testdengine-scene-ecs",&window);
    dengine_window_makecurrent(&window);
    dengine_window_loadgl();

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

    denginescene_ecs_parent(ent6, ent7);
    denginescene_ecs_parent(ent6, ent8);
    denginescene_ecs_parent(ent6, ent9);
    denginescene_ecs_parent(ent4, ent5);
    denginescene_ecs_parent(ent4, ent6);
    denginescene_ecs_parent(ent2, ent3);
    denginescene_ecs_parent(ent2, ent4);
    denginescene_ecs_parent(ent1, ent2);

    denginescene_ecs_parent(ent10, ent11);
    denginescene_ecs_parent(ent10, ent12);

    Camera cam;
    dengine_camera_setup(&cam);
    dengine_camera_set_rendermode(DENGINE_CAMERA_RENDER_FOWARD,&cam);
    CameraComponent camc;
    camc.camera=&cam;
    ent13->camera_component=&camc;

    Shader* stdshdr = dengine_shader_new_shader_standard();

    Material cube_mat;
    dengine_material_setup(&cube_mat);
    dengine_material_set_shader_color(stdshdr,&cube_mat);

    Primitive cube,plane;
    dengine_primitive_gen_cube(&cube,stdshdr);
    dengine_primitive_gen_plane(&plane,stdshdr);

    DirLight dLight;
    memset(&dLight,0,sizeof (DirLight));
    dengine_lighting_setup_dirlight(&dLight);

    PointLight pLight;
    memset(&pLight,0,sizeof (PointLight));
    dengine_lighting_setup_pointlight(&pLight);
    pLight.position[1] = 5.5;
    pLight.light.diffuse[2] = 0;
    pLight.light.specular[2] = 0;

    dengine_lighting_apply_dirlight(&dLight,stdshdr);
    dengine_lighting_apply_pointlight(&pLight,stdshdr);

    MeshComponent cube_mesh,plane_mesh;

    cube_mesh.material=&cube_mat;
    plane_mesh.material=&cube_mat;

    plane_mesh.mesh=&plane;
    cube_mesh.mesh=&cube;

    ent1->mesh_component=&plane_mesh;
    ent6->mesh_component=&cube_mesh;
    ent7->mesh_component=&cube_mesh;

    /*
     *            SCENE -- 13(camera)
     *            |    |
     *            1    10
     *            |    / \
     *            2   11 12
     *           / \
     *          3   4
     *            / \
     *           5   6
     *             / | \
     *            7  8 9
     */

//    prt(ent1);
    vec3 p={6.1f,7.18f,3.4f};
    memcpy(ent13->transform.position,p,sizeof (vec3));

    p[0]=1.3f,p[1]=2.9f,p[2]=1.0f;
    memcpy(ent6->transform.position,p,sizeof (vec3));

    p[0]=2.0f,p[1]=-2.0f,p[2]=1.0f;
    memcpy(ent7->transform.position,p,sizeof (vec3));

    p[0]=5.0f,p[1]=5.0f,p[2]=5.0f;
    memcpy(ent1->transform.scale,p,sizeof (vec3));

    Scene* scene = denginescene_new();
    denginescene_add_entity(scene, ent1);
    denginescene_add_entity(scene, ent10);
    denginescene_add_entity(scene, ent13);

    glEnable(GL_DEPTH_TEST);

    denginegui_init();
    static float fontsz=18.0f;
    denginegui_set_font(NULL,fontsz,512);
    char fpsstr[30];

    while (dengine_window_isrunning()) {
        dengine_input_pollevents();

        static double elapsed=9999.0;

        dengineutils_timer_update();
        double delta=dengineutils_timer_get_delta();

        elapsed+=delta;
        if(elapsed>1000.0)
        {
            snprintf(fpsstr,sizeof (fpsstr),"FPS : %.1f (%.2fms)",1/delta*1000,delta);
            elapsed=0.0;
        }

        denginescene_update(scene);

        glClearColor(0.1,0.1,0.1,1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        denginegui_panel(0,0,640,360,&cam.framebuffer.color[0],NULL,NULL);
        denginegui_text(10,10,fpsstr,NULL);

        dengine_window_swapbuffers();
    }

    denginescene_destroy(scene);

    dengine_material_destroy(&cube_mat);
    dengine_shader_destroy(stdshdr);
    free(stdshdr);

    dengine_window_terminate();

    return 0;
}
