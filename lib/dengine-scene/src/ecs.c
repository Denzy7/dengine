#include <stdlib.h> //malloc
#include <string.h> //memset
#include "dengine-scene/ecs.h"
#include "dengine-utils/debug.h"
#include "dengine-utils/logging.h"
#include "dengine_config.h"// DENGINE_ECS_MAXCHILDREN
#ifdef DENGINE_SCRIPTING_PYTHON
#include "dengine-script/python/python.h"
#include "dengine-scene/scriptingsync/python/entity.h"
#endif

uint32_t entity_count = 0;


void _denginescene_ecs_new_entity_setup(Entity* ent);

void _denginescene_ecs_new_entity_setup(Entity* ent)
{
    entity_count++;
    ent->entity_id = entity_count;
    //dengineutils_logging_log("new ent : %u, %p", entity_count, ent);
    ent->active = 1;
    glm_vec3_fill(ent->transform.scale, 1);
    vtor_create_ptrs(&ent->children);
    ent->name =  malloc(DENGINE_ECS_MAXNAME);
    snprintf(ent->name, DENGINE_ECS_MAXNAME, "Entity %u", ent->entity_id);

    vtor_create(&ent->scripts, sizeof(Script));
}

int denginescene_ecs_new_entity(Entity** entity)
{
    DENGINE_DEBUG_ENTER;

    if((*entity = calloc(1, sizeof(Entity))) == NULL)
    {
        perror("calloc");
        return 0;
    };

    _denginescene_ecs_new_entity_setup(*entity);

    return 1;
}

void denginescene_ecs_destroy_entity(Entity* root)
{
    DENGINE_DEBUG_ENTER;

    for(size_t i = 0; i < root->children.count; i++)
    {
        Entity** e = root->children.data;
        denginescene_ecs_destroy_entity(e[i]); /* dfs since bfs may free parent before children
                                                  which is bad since that reading a null pointer :)
                                                  */
    }

    if(root->mesh_component)
        free(root->mesh_component);

    if(root->camera_component)
        free(root->camera_component);

    if(root->light_component)
    {
        free(root->light_component->light);
        free(root->light_component);
    }

    vtor_free(&root->scripts);
    vtor_free(&root->children);

    free(root->name);
    free(root);
}

void denginescene_ecs_parent(Entity* parent, Entity* child)
{
    DENGINE_DEBUG_ENTER;
    child->parent = parent;
    /* TODO:
     * vtor probably not best for parenting
     * and unparenting */
    vtor_pushback(&parent->children, child);
    //dengineutils_logging_log("parent %u (%p) to %u", child->entity_id, child, parent->entity_id);
}

void denginescene_ecs_get_model_local(Entity* entity,mat4 mat4x4)
{
    DENGINE_DEBUG_ENTER;

    glm_mat4_identity(mat4x4);
    glm_translate(mat4x4,entity->transform.position);
    glm_rotate_x(mat4x4, glm_rad(entity->transform.rotation[0]), mat4x4);
    glm_rotate_y(mat4x4, glm_rad(entity->transform.rotation[1]), mat4x4);
    glm_rotate_z(mat4x4, glm_rad(entity->transform.rotation[2]), mat4x4);
    glm_scale(mat4x4,entity->transform.scale);
}

void denginescene_ecs_set_entity_name(Entity* entity, const char* name)
{
    DENGINE_DEBUG_ENTER;

    snprintf(entity->name, DENGINE_ECS_MAXNAME, "%s", name);
}

int denginescene_ecs_new_meshcomponent(const Primitive* mesh, const Material* material, MeshComponent** component)
{
    DENGINE_DEBUG_ENTER;
    if((*component = calloc(1, sizeof(MeshComponent))) == NULL)
    {
        perror("calloc");
        return 0;
    };

    memcpy(&(*component)->mesh, mesh, sizeof(Primitive));
    memcpy(&(*component)->material, material, sizeof(Material));
    (*component)->drawmode = DENGINESCENE_ECS_MESHCOMPONENT_DRAWMODE_ENABLED;

    return 1;
}

int denginescene_ecs_new_cameracomponent(const Camera* camera, CameraComponent** component)
{
    DENGINE_DEBUG_ENTER;
    if((*component = calloc(1, sizeof(CameraComponent))) == NULL)
    {
        perror("calloc");
        return 0;
    }

    memcpy(&(*component)->camera, camera, sizeof(Camera));

    return 1;
}

int denginescene_ecs_new_lightcomponent(LightType type, const Light* light, LightComponent** component)
{
    DENGINE_DEBUG_ENTER;
    size_t ln = 0;

    if(type == DENGINE_LIGHT_DIR){
        ln = sizeof(DirLight);
    }
    else if(type == DENGINE_LIGHT_POINT){
        ln = sizeof(PointLight);
    }
    else if(type == DENGINE_LIGHT_SPOT){
        ln = sizeof(SpotLight);
    }
    else {
        dengineutils_logging_log("ERROR::unknown light type!\n");
        return 0;
    }

    if((*component = calloc(1, sizeof(LightComponent))) == NULL)
    {
        perror("calloc");
        return 0;
    }
    if(((*component)->light = calloc(1, ln)) == NULL)
    {
        perror("calloc");
        return 0;
    }

    memcpy((*component)->light, light, ln);

    (*component)->type = type;

    return 1;
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

    for(size_t i = 0; i < entity->children.count; i++)
    {
        Entity* child = ((Entity**)entity->children.data)[i];
        denginescene_ecs_get_model_local(child,
                                         child->transform.world_model);

        if(child->parent)
        {
            glm_mat4_mul(
                    child->parent->transform.world_model,
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

void _advancelevel(int level)
{
    for(int i = 0; i < level; i++)
    {
        printf("    ");
    }
    printf("*");
}
void _dumpentity(Entity* e, FILE* stdiof, int* l)
{
    int level = *l;
    _advancelevel(level);
    fprintf(stdiof,"Name: %s ,", e->name);
    fprintf(stdiof,"Entity id: %u,", e->entity_id);
    fprintf(stdiof,"Active: %d\n", e->active);

    _advancelevel(level + 1);
    fprintf(stdiof,"Transform:\n");

    const char* tgtstr[3] = {"position", "rotation", "scale"};
    const char* tgtfmt[3] = {"x", "y", "z"};
    float* tgtflt[3] = { e->transform.position, e->transform.rotation, e->transform.scale};
    for(int i = 0; i < 3; i++)
    {
        _advancelevel(level + 2);
        fprintf(stdiof,"%s: ",tgtstr[i]);
        for(int j = 0; j < 3; j++)
        {   
            fprintf(stdiof,"%s: %f",tgtfmt[j], tgtflt[i][j]);
            if(j != 2)
                fprintf(stdiof, ", ");
        }
        fprintf(stdiof,"\n");
    }

    if(e->camera_component)
    {
        Camera* c = &e->camera_component->camera;
        _advancelevel(level + 1);
        fprintf(stdiof,"Camera:\n");
        _advancelevel(level + 2);
        fprintf(stdiof,"near: %f, far: %f\n", c->near, c->far);
        _advancelevel(level + 2);
        fprintf(stdiof,"width: %u, height: %u\n", c->render_width, c->render_height);
    }

    if(e->mesh_component)
    {
        _advancelevel(level + 1);
        fprintf(stdiof,"Mesh:\n");
        _advancelevel(level + 2);
        fprintf(stdiof,"drawmode: %d\n", e->mesh_component->drawmode);
        _advancelevel(level + 2);
        fprintf(stdiof,"materialid: %d\n", e->mesh_component->material.id);
        _advancelevel(level + 2);
        fprintf(stdiof,"array_buffer id: %u, index_buffer id: %u\n",
                e->mesh_component->mesh.array.buffer_id,
                e->mesh_component->mesh.index.buffer_id);
    }

    if(e->light_component)
    {
        _advancelevel(level + 1);
        fprintf(stdiof,"Light:\n");
        _advancelevel(level + 2);
        if(e->light_component->type == DENGINE_LIGHT_DIR)
        {
            fprintf(stdiof,"Type: Dir\n");
        }else if(e->light_component->type == DENGINE_LIGHT_POINT)
        {
            fprintf(stdiof,"Type: Point\n");
        }else if(e->light_component->type == DENGINE_LIGHT_SPOT)
        {
            fprintf(stdiof,"Type: Spot\n");
        }

    }

    printf("\n");
    for(size_t i = 0; i < e->children.count; i++)
    {
        Entity* ch = ((Entity**)e->children.data)[i];
        (*l)++;
        _dumpentity(ch, stdiof, l);
        (*l)--;
    }
}
void denginescene_ecs_dumphierachy_stdio(Entity* entity, FILE* stdiof)
{
    int level = 0;
    _dumpentity(entity, stdiof, &level);
}


void denginescene_ecs_scripts_run(Entity* entity, ScriptFunc func, void* args)
{
    Entity* e = entity;
    for(size_t j = 0; j < e->scripts.count; j++)
    {
        Script* script =  &((Script*)e->scripts.data)[j];
        if(script->type == DENGINE_SCRIPT_TYPE_PYTHON)
        {
#ifdef DENGINE_SCRIPTING_PYTHON
            denginescene_scriptingsync_python_push(DENGINESCENE_SCRIPTSYNC_ALL, e, args);
            denginescript_call(script, func, args);
            denginescene_scriptingsync_python_pull(DENGINESCENE_SCRIPTSYNC_ALL, e, args);
#endif
        }else if(script->type == DENGINE_SCRIPT_TYPE_NSL)
        {
            denginescript_call(script, func, e);
        }
    }
}
