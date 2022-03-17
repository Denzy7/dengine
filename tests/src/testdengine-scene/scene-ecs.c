#include <dengine-scene/ecs.h>
#include <dengine-scene/scene.h>

#include <dengine-utils/logging.h>
#include <stdlib.h>
#include <stdio.h>

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
    //dengine_camera_setup(&cam);
    //dengine_camera_lookat(NULL,&cam);
    CameraComponent camc;
    camc.camera=&cam;
    ent13->camera_component=&camc;

    MeshComponent msh;
    ent4->mesh_component=&msh;


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
    vec3 p={12.1f,8.18f,10.4f};
    glm_translate(ent13->transform.modelmtx,p);

    Scene* scene = denginescene_new();
    denginescene_add_entity(scene, ent1);
    denginescene_add_entity(scene, ent10);
    denginescene_add_entity(scene, ent13);

    denginescene_update(scene);

    denginescene_destroy(scene);

    return 0;
}
