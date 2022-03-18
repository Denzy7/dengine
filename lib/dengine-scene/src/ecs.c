#include <stdlib.h> //malloc
#include <string.h> //memset
#include "dengine/draw.h"
#include "dengine-scene/ecs.h"

#include "dengine-utils/logging.h"
#include "dengine_config.h"// DENGINE_ECS_MAXCHILDREN

uint32_t entity_count = 0;

void _denginescene_ecs_destroy_entity_children(Entity* root);

Entity* denginescene_ecs_new_entity()
{
    entity_count++;
    Entity* ent = malloc(sizeof (Entity));
    memset(ent, 0, sizeof (Entity));
    ent->entity_id = entity_count;
    //dengineutils_logging_log("new ent : %u, %p", entity_count, ent);
    ent->active = 1;
    ent->transform.scale[0]=1.0f,ent->transform.scale[1]=1.0f,ent->transform.scale[2]=1.0f;
    ent->children = malloc(DENGINE_ECS_MAXCHILDREN * sizeof (Entity*));
    return ent;
}

void _denginescene_ecs_destroy_entity_children(Entity* root)
{
    size_t children_count = root->children_count;
    for (size_t i = 0; i < children_count; i++) {
        Entity* child = root->children[i];
        _denginescene_ecs_destroy_entity_children(child);
        //dengineutils_logging_log("destroy child %u. parent %u", child->entity_id, child->parent->entity_id);
        free(child);
    }
    free(root->children);
}

void denginescene_ecs_destroy_entity(Entity* root)
{
    _denginescene_ecs_destroy_entity_children(root);

    //dengineutils_logging_log("destroy root %u", root->entity_id);
    free(root);
}

void denginescene_ecs_parent(struct _Entity* parent, struct _Entity* child)
{
    if (parent->children_count >= DENGINE_ECS_MAXCHILDREN) {
        dengineutils_logging_log("WARNING::Parenting limit reached! Recompile to increase limit. Free the allocated entity to avoid a memory leak!");
        return;
    }
    child->parent = parent;

    parent->children[parent->children_count] = child;
    parent->children_count++;

    dengineutils_logging_log("parent %u (%p) to %u", child->entity_id, child, parent->entity_id);
}

void denginescene_ecs_get_model(Entity* entity,mat4 mat4x4)
{
    glm_mat4_identity(mat4x4);
    glm_translate(mat4x4,entity->transform.position);
    //rotate here
    glm_scale(mat4x4,entity->transform.scale);
}
