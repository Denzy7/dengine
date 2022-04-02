#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <dengine-scene/ecs.h>
#include <dengine-scene/scene.h>

#include <dengine-utils/logging.h>
#include <dengine-utils/timer.h>
#include <dengine-utils/filesys.h>

#include <dengine/input.h>
#include <dengine/window.h>
#include <dengine/draw.h>
#include <dengine/camera.h>
#include <dengine/loadgl.h>

#include <dengine-gui/gui.h>

#include <dengine-model/model.h>

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

    dengineutils_filesys_init();

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
    denginescene_ecs_parent(ent1, ent2);

    denginescene_ecs_parent(ent10, ent11);
    denginescene_ecs_parent(ent10, ent12);

    Camera cam;
    dengine_camera_setup(&cam);
    dengine_camera_set_rendermode(DENGINE_CAMERA_RENDER_FOWARD,&cam);

    CameraComponent* camc = denginescene_ecs_new_cameracomponent(&cam);
    ent13->camera_component=camc;

    Shader* stdshdr = dengine_shader_new_shader_standard(DENGINE_SHADER_STANDARD);
    Shader* dftshdr = dengine_shader_new_shader_standard(DENGINE_SHADER_DEFAULT);

    Material cube_mat,duck_mat,dft_mat;

    dengine_material_setup(&cube_mat);
    dengine_material_setup(&duck_mat);
    dengine_material_setup(&dft_mat);

    dengine_material_set_shader_color(stdshdr,&cube_mat);
    dengine_material_set_shader_color(stdshdr,&duck_mat);
    dengine_material_set_shader_color(dftshdr,&dft_mat);

    Primitive cube,plane,grid;
    dengine_primitive_gen_cube(&cube,stdshdr);
    dengine_primitive_gen_plane(&plane,stdshdr);
    dengine_primitive_gen_grid(10,&grid,dftshdr);

    const int prtbf_sz=2048;
    char* prtbf=malloc(prtbf_sz);

    snprintf(prtbf,prtbf_sz,"%s/models/duck.obj",dengineutils_filesys_get_assetsdir());
    Primitive* duck = denginemodel_load_file(DENGINE_MODEL_FORMAT_OBJ,prtbf,NULL,stdshdr);

    //load separated planes
    snprintf(prtbf,prtbf_sz,"%s/models/sperated-planes.obj",dengineutils_filesys_get_assetsdir());
    size_t n_planes = 0;
    Primitive* sep_planes = denginemodel_load_file(DENGINE_MODEL_FORMAT_OBJ,prtbf,&n_planes,stdshdr);

    //Entity** child_sep = NULL;
    Entity* ent15 = denginescene_ecs_new_entity();

    Material sep_planes_mat;
    dengine_material_setup(&sep_planes_mat);
    dengine_material_set_shader_color(stdshdr,&sep_planes_mat);

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

    DirLight dLight;
    memset(&dLight,0,sizeof (DirLight));
    dengine_lighting_setup_dirlight(&dLight);
    dLight.light.strength=0.55f;

    PointLight pLight;
    memset(&pLight,0,sizeof (PointLight));
    dengine_lighting_setup_pointlight(&pLight);
    pLight.position[1] = 5.5;
    pLight.light.diffuse[2] = 0;
    pLight.light.specular[2] = 0;
    pLight.light.strength=0.85f;

    dengine_lighting_apply_dirlight(&dLight,stdshdr);
    dengine_lighting_apply_pointlight(&pLight,stdshdr);

    MeshComponent* cube_mesh, * cube_mesh2,* plane_mesh,* duck_mesh,* grid_mesh;
    cube_mesh = denginescene_ecs_new_meshcomponent(&cube, &cube_mat);
    plane_mesh = denginescene_ecs_new_meshcomponent(&plane, &cube_mat);
    duck_mesh = denginescene_ecs_new_meshcomponent(duck, &duck_mat);
    grid_mesh = denginescene_ecs_new_meshcomponent(&grid, &dft_mat);
    cube_mesh2 = denginescene_ecs_new_meshcomponent(&cube, &cube_mat);

    ent1->mesh_component=plane_mesh;
    ent6->mesh_component=cube_mesh;
    ent7->mesh_component=cube_mesh2;
    ent3->mesh_component=duck_mesh;
    ent14->mesh_component=grid_mesh;

    Texture duck_tex;
    memset(&duck_tex,0,sizeof (Texture));
    duck_tex.auto_dataonload=1;
    duck_tex.interface=DENGINE_TEXTURE_INTERFACE_8_BIT;

    snprintf(prtbf,prtbf_sz,"%s/textures/2d/duck.png",dengineutils_filesys_get_assetsdir());
    dengine_texture_load_file(prtbf,1,&duck_tex);

    dengine_material_set_texture(&duck_tex,"diffuseTex",&duck_mat);

    /*
     *            SCENE -- 13(camera),14=grid
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
    vec3 p={4.1f,5.18f,3.4f};
    memcpy(ent13->transform.position,p,sizeof (vec3));

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
    denginescene_add_entity(scene, ent10);
    denginescene_add_entity(scene, ent13);
    denginescene_add_entity(scene, ent14);
    denginescene_add_entity(scene, ent15);

    denginegui_init();
    static float fontsz=18.0f;
    denginegui_set_font(NULL,fontsz,512);
    char fpsstr[30];

    glEnable(GL_DEPTH_TEST);

    float viewportcol[4]={.0,.0,.0,1.};

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

        denginegui_panel(0,0,854,480,&cam.framebuffer.color[0],NULL,viewportcol);
        denginegui_text(10,10,fpsstr,NULL);

        dengine_window_swapbuffers();
    }

    denginescene_destroy(scene);

    dengine_material_destroy(&cube_mat);
    dengine_material_destroy(&duck_mat);
    dengine_material_destroy(&dft_mat);
    dengine_material_destroy(&sep_planes_mat);

    dengine_shader_destroy(stdshdr);
    dengine_shader_destroy(dftshdr);

    free(stdshdr);
    free(dftshdr);

    free(prtbf);
    free(duck);

//    if(child_sep)
//        free(child_sep);

    if(sep_planes)
        free(sep_planes);

    denginegui_terminate();
    dengineutils_filesys_terminate();
    dengine_window_terminate();

    return 0;
}
