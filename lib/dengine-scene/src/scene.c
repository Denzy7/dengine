#include <stdlib.h> //malloc
#include <string.h> //memset

#include "dengine-scene/scene.h"
#include "dengine-utils/logging.h"
#include "dengine/draw.h"

#include "dengine_config.h"// DENGINE_ECS_MAXCHILDREN

Scene* denginescene_new()
{
    Scene* newscn = malloc(sizeof (Scene));
    memset(newscn, 0, sizeof (Scene));

    newscn->entities = malloc(DENGINE_ECS_MAXCHILDREN * sizeof (Entity*));

    return newscn;
}

void denginescene_destroy(Scene* scene)
{
    for (uint32_t i = 0; i < scene->n_entities; i++) {
        denginescene_ecs_destroy_entity(scene->entities[i]);
    }
    free(scene->entities);
    free(scene);
}

void denginescene_add_entity(Scene* scene, struct _Entity* entity)
{
    if (scene->n_entities >= DENGINE_ECS_MAXCHILDREN) {
        dengineutils_logging_log("WARNING::Scene limit reached! Recompile to increase limit. Free the allocated entity to avoid a memory leak!");
        return;
    }
    scene->entities[scene->n_entities] = entity;
    scene->n_entities++;
}

void denginescene_update(Scene* scene)
{
    for (uint32_t i = 0; i < scene->n_entities; i++) {
        Entity* root=scene->entities[i];
        denginescene_do_camera(root,scene);
    }
}

void _denginescene_ecs_do_camera_draw_mesh(Entity* camera, Entity* mesh)
{
    //dengineutils_logging_log("drw %u",mesh->entity_id);
    dengine_material_use(mesh->mesh_component->material);
    dengine_camera_lookat(NULL, camera->camera_component->camera);
    dengine_camera_apply(&mesh->mesh_component->material->shader_color,camera->camera_component->camera);

    mat4 model,model_parent;
    denginescene_ecs_get_model(mesh,model);
    if(mesh->parent)
    {
        denginescene_ecs_get_model(mesh->parent,model_parent);
        glm_mat4_mul(model_parent,model,model);
    }

    dengine_shader_set_mat4(&mesh->mesh_component->material->shader_color,
                            "model",
                            model[0]
                            );
    dengine_draw_primitive(mesh->mesh_component->mesh,&mesh->mesh_component->material->shader_color);
    dengine_material_use(NULL);
}

void _denginescene_ecs_do_camera_draw_children(Entity* camera,Entity* root)
{
    size_t children_count = root->children_count;
    for (size_t i = 0; i < children_count; i++) {
        Entity* child = root->children[i];
        if(child->mesh_component)
        {
            _denginescene_ecs_do_camera_draw_mesh(camera,child);
        }
        _denginescene_ecs_do_camera_draw_children(camera,child);
    }
}

void _denginescene_ecs_do_camera_draw(Entity* camera,Entity* entity)
{
    if(entity->mesh_component)
    {
        _denginescene_ecs_do_camera_draw_mesh(camera,entity);
    }
    _denginescene_ecs_do_camera_draw_children(camera,entity);
}

void _denginescene_ecs_do_camera_scene(Entity* camera, Scene* scene)
{
    //apply position
    memcpy(camera->camera_component->camera->position,
           camera->transform.position,
           sizeof(camera->camera_component->camera->position));
    //dengineutils_logging_log("dcmp %f %f %f",t[0],t[1],t[2]);
    dengine_camera_use(camera->camera_component->camera);
    //render scene recursive
    for (size_t i = 0; i < scene->n_entities; i++)
    {
        _denginescene_ecs_do_camera_draw(camera,scene->entities[i]);
    }

    dengine_camera_use(NULL);
}
void _denginescene_ecs_do_camera_children(Entity* root, Scene* scene)
{
    size_t children_count = root->children_count;
    for (size_t i = 0; i < children_count; i++) {
        Entity* child = root->children[i];
        if(child->camera_component)
        {
            _denginescene_ecs_do_camera_scene(child,scene);
        }
        _denginescene_ecs_do_camera_children(child,scene);
    }
}
void denginescene_do_camera(Entity* root, Scene* scene)
{
    //check camera comp
    if(root->camera_component)
    {
        _denginescene_ecs_do_camera_scene(root,scene);
    }

    _denginescene_ecs_do_camera_children(root, scene);
}
