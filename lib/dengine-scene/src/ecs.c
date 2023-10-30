#include <stdlib.h> //malloc
#include <string.h> //memset
#include "dengine-scene/ecs.h"
#include "dengine-utils/debug.h"
#include "dengine_config.h"// DENGINE_ECS_MAXCHILDREN

uint32_t entity_count = 0;

void _denginescene_ecs_destroy_entity_children(Entity* root);

void _denginescene_ecs_new_entity_setup(Entity* ent);

void _denginescene_ecs_destroy_entity_components(Entity* root);

void _denginescene_ecs_destroy_entity_components(Entity* root)
{
    if(root->mesh_component)
    {
        free(root->mesh_component->material);
        free(root->mesh_component->mesh);
        free(root->mesh_component);
        root->mesh_component = NULL;
    }

    if(root->camera_component)
    {
        free(root->camera_component->camera);
        free(root->camera_component);
        root->camera_component = NULL;
    }

    if(root->light_component)
    {
        free(root->light_component->light);
        free(root->light_component);
    }

    vtor_free(&root->scripts);
}

void _denginescene_ecs_new_entity_setup(Entity* ent)
{
    entity_count++;
    ent->entity_id = entity_count;
    //dengineutils_logging_log("new ent : %u, %p", entity_count, ent);
    ent->active = 1;
    ent->transform.scale[0]=1.0f,ent->transform.scale[1]=1.0f,ent->transform.scale[2]=1.0f;
    vtor_create(&ent->children, sizeof(EntityChild));
    ent->name =  malloc(DENGINE_ECS_MAXNAME);
    snprintf(ent->name, DENGINE_ECS_MAXNAME, "Entity %u",ent->entity_id);

    vtor_create(&ent->scripts, sizeof(Script));
}

Entity* denginescene_ecs_new_entity()
{
    DENGINE_DEBUG_ENTER;

    Entity* ent = malloc(sizeof(struct _Entity));

    if(!ent)
        return NULL;

    memset(ent, 0, sizeof (Entity));

    _denginescene_ecs_new_entity_setup(ent);

    return ent;
}

void _denginescene_ecs_destroy_entity_children(Entity* root)
{
    size_t children_count = root->children.count;
    EntityChild* ec = root->children.data;
    for (size_t i = 0; i < children_count; i++) {
        Entity* child = ec[i].child;
        _denginescene_ecs_destroy_entity_components(child);
        _denginescene_ecs_destroy_entity_children(child);
        //dengineutils_logging_log("destroy child %u. parent %u", child->entity_id, child->parent->entity_id);
        free(child->name);
        free(child);
    }
    vtor_free(&root->children);
}

void denginescene_ecs_destroy_entity(Entity* root)
{
    DENGINE_DEBUG_ENTER;

    _denginescene_ecs_destroy_entity_children(root);
    _denginescene_ecs_destroy_entity_components(root);
    //dengineutils_logging_log("destroy root %u", root->entity_id);
    free(root->name);
    free(root);
}

void denginescene_ecs_parent(Entity* parent, Entity* child)
{
    DENGINE_DEBUG_ENTER;

    child->parent = parent;

    EntityChild ec = { child };
    vtor_pushback(&parent->children, &ec);

    //dengineutils_logging_log("parent %u (%p) to %u", child->entity_id, child, parent->entity_id);
}

void denginescene_ecs_get_model_local(Entity* entity,mat4 mat4x4)
{
    DENGINE_DEBUG_ENTER;

    glm_mat4_identity(mat4x4);
    glm_translate(mat4x4,entity->transform.position);

    versor x,y,z;

    glm_quat(x, glm_rad(entity->transform.rotation[0]),1.0f,0.0f,0.0f);
    glm_quat(y, glm_rad(entity->transform.rotation[1]),0.0f,1.0f,0.0f);
    glm_quat(z, glm_rad(entity->transform.rotation[2]),0.0f,0.0f,1.0f);

    glm_quat_rotate(mat4x4,x,mat4x4);
    glm_quat_rotate(mat4x4,y,mat4x4);
    glm_quat_rotate(mat4x4,z,mat4x4);

    glm_scale(mat4x4,entity->transform.scale);
}

void denginescene_ecs_set_entity_name(Entity* entity, const char* name)
{
    DENGINE_DEBUG_ENTER;

    snprintf(entity->name, DENGINE_ECS_MAXNAME, "%s", name);
}

MeshComponent* denginescene_ecs_new_meshcomponent(const Primitive* mesh, const Material* material)
{
    DENGINE_DEBUG_ENTER;

    MeshComponent* mesh_comp = calloc(1, sizeof(MeshComponent));

    Primitive* prim = calloc(1, sizeof(Primitive));
    Material* mat = calloc(1, sizeof(Material));

    memcpy(prim, mesh, sizeof(Primitive));
    memcpy(mat, material, sizeof(Material));

    mesh_comp->draw = 1;
    mesh_comp->material = mat;
    mesh_comp->mesh= prim;

    return mesh_comp;
}

CameraComponent* denginescene_ecs_new_cameracomponent(const Camera* camera)
{
    DENGINE_DEBUG_ENTER;

    CameraComponent* cam_comp = calloc(1, sizeof(CameraComponent));
    Camera* cam = calloc(1, sizeof(Camera));
    memcpy(cam, camera, sizeof(Camera));

    cam_comp->camera = cam;
    cam_comp->last_cam = 1;

    return cam_comp;
}

LightComponent* denginescene_ecs_new_lightcomponent(LightType type, Light light)
{
    DENGINE_DEBUG_ENTER;

    LightComponent* comp = calloc(1, sizeof(LightComponent));

    if(!comp)
        return NULL;

    if(type == DENGINE_LIGHT_DIR)
    {
        comp->light = calloc(1, sizeof(DirLight));
        memcpy(comp->light, light, sizeof(DirLight));
    }else if(type == DENGINE_LIGHT_POINT)
    {
        comp->light = calloc(1, sizeof(PointLight));
        memcpy(comp->light, light, sizeof(PointLight));
    }else if(type == DENGINE_LIGHT_SPOT)
    {
        comp->light = calloc(1, sizeof(SpotLight));
        memcpy(comp->light, light, sizeof(SpotLight));
    }
    comp->type = type;
    return comp;
}

void denginescene_ecs_add_script(Entity* entity, const Script* script)
{
    DENGINE_DEBUG_ENTER;

    vtor_pushback(&entity->scripts, script);
}

void denginescene_ecs_transform_entity(Entity* entity)
{
    DENGINE_DEBUG_ENTER;

    if(entity->transform.manualtransform)
        return;

    if(!entity->parent)
        denginescene_ecs_get_model_local(entity, entity->transform.world_model);

    EntityChild* ec = entity->children.data;
    for(size_t i = 0; i < entity->children.count; i++)
    {
        Entity* child = ec[i].child;

        denginescene_ecs_get_model_local(child,
                                         child->transform.world_model);

        if(child->parent)
        {
            glm_mat4_mul(child->parent->transform.world_model,
                         child->transform.world_model,
                         child->transform.world_model);

            denginescene_ecs_transform_entity(child);
        }
    }
}

void denginescene_ecs_get_front(Entity* entity, vec3 front)
{
    DENGINE_DEBUG_ENTER;

    front[0] = cosf(glm_rad(entity->transform.rotation[1])) * cosf(glm_rad(entity->transform.rotation[0]));
    front[1] = sinf(glm_rad(entity->transform.rotation[0]));
    front[2] = sinf(glm_rad(entity->transform.rotation[1])) * cosf(glm_rad(entity->transform.rotation[0]));
    glm_vec3_add(entity->transform.position, front, front);
}

void denginescene_ecs_get_right(Entity* entity, vec3 right)
{
    DENGINE_DEBUG_ENTER;

    vec3 front;
    vec3 y = {0.0f, 1.0f, 0.0f};
    denginescene_ecs_get_front(entity, front);
    glm_vec3_cross(front, y, right);
}

void denginescene_ecs_get_up(Entity* entity, vec3 up)
{
    DENGINE_DEBUG_ENTER;

    vec3 right, front;
    denginescene_ecs_get_right(entity, right);
    denginescene_ecs_get_front(entity, front);
    glm_vec3_cross(right, front, up);
}

