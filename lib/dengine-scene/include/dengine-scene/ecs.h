#ifndef ECS_H
#define ECS_H

#include <cglm/cglm.h>

#include "dengine/primitive.h"
#include "dengine/material.h"
#include "dengine/lighting.h"
#include "dengine/camera.h"

#include "dengine-utils/vtor.h"

#include "dengine-script/script.h"

typedef struct
{
    vec3 position;
    vec3 rotation;
    vec3 scale;

    /* ecs wont write to world_model with this 
     * set to 1. used for physics simulations etc.
     */
    int manualtransform;

    mat4 world_model;
}TransformComponent;

typedef enum 
{
    DENGINESCENE_ECS_MESHCOMPONENT_DRAWMODE_DISABLED,
    DENGINESCENE_ECS_MESHCOMPONENT_DRAWMODE_ENABLED,
    DENGINESCENE_ECS_MESHCOMPONENT_DRAWMODE_SHADOWONLY,
}MeshDrawMode;
typedef struct
{
    MeshDrawMode drawmode;
    Primitive mesh;
    Material material;
}MeshComponent;

typedef struct
{
    LightType type;
    Light* light;
}LightComponent;

typedef struct
{
    /* we store entry stuff here. we'll change them 
     * throught out rendering so we set once and set back
     * once here
     */
    int entry_x, entry_y, entry_w, entry_h;;
    int entry_depthfunc, entry_cullmode;
    int entry_srcalpha, entry_dstalpha;

    int entry_depthon, entry_cullon, entry_blendon;

    Camera camera;
}CameraComponent;

typedef struct Entity
{
    uint32_t entity_id;
    int active;
    TransformComponent transform;
    char* name;

    /* PARENT-CHILD R/SHIPS */
    /* its a recusive structure so it maintains struct keyword */
    struct Entity* parent;
    vtor children;

    /* SCRIPTS */
    vtor scripts;

    /* MORE COMPONENTS HERE */
    /* additional components are kept as pointers
     * so we can discern if it added
     * by checking NULL instad of each var
     *
     * also they'll be stored on the heap that way it wont overflow if we have many entities
     */
    MeshComponent* mesh_component;
    CameraComponent* camera_component;
    LightComponent* light_component;
}Entity;

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Create a new empty entity
 * \return An empty active entity with no components
 */
int denginescene_ecs_new_entity(Entity** entity);

/*!
 * \brief Destroy an allocated entity, its underlying entities and components
 * \param root Entity to destroy
 */
void denginescene_ecs_destroy_entity(Entity* root);

/*!
 * \brief Set the name of an entity
 * \param entity Entity to use
 * \param name Name to use
 */
void denginescene_ecs_set_entity_name(Entity* entity, const char* name);

/*!
 * \brief Parent a child to a parent entity
 * \param parent Parent to use
 * \param child Child to use
 */
void denginescene_ecs_parent(Entity* parent, Entity* child);

void denginescene_ecs_get_model_local(Entity* entity,mat4 mat4x4);

/* note this is not homogenous and contains
 * the position the entity forward would be.
 * to get the homogenous, subtract the current position
 */
void denginescene_ecs_get_front(Entity* entity, vec3 front);
/* note this is not homogenous and contains
 * the position the entity forward would be.
 * to get the homogenous, subtract the current position
 */
void denginescene_ecs_get_right(Entity* entity, vec3 right);
/* note this is not homogenous and contains
 * the position the entity forward would be.
 * to get the homogenous, subtract the current position
 */
void denginescene_ecs_get_up(Entity* entity, vec3 up);

/*!
 * \brief Transform an entity position, rotation and scale to model matrix with parent entity then transform children entities
 * \param entity entity to transform. can be a root entity or child entity 
 */
void denginescene_ecs_transform_entity(Entity* entity);

int denginescene_ecs_new_meshcomponent(const Primitive* mesh, const Material* material, MeshComponent** component);

int denginescene_ecs_new_cameracomponent(const Camera* camera, CameraComponent** component);

int denginescene_ecs_new_lightcomponent(LightType type, const Light* light, LightComponent** component);

//PhysicsComponent* denginescene_ecs_new_physicscomponent(ECSPhysicsColShape type, const void* colshapeconfig, const float mass);

void denginescene_ecs_add_script(Entity* entity, const Script* script);

void denginescene_ecs_scripts_run(Entity* entity, ScriptFunc func, void* args);

void denginescene_ecs_dumphierachy_stdio(Entity* entity, FILE* stdiof);

#ifdef __cplusplus
}
#endif

#endif
