#ifndef ECS_H
#define ECS_H

#include <stdint.h> //uint32_t
#include <cglm/cglm.h>

#include "dengine/primitive.h"
#include "dengine/material.h"
#include "dengine/lighting.h"
#include "dengine/camera.h"

#include "dengine-utils/vtor.h"

typedef struct
{
    vec3 position;
    vec3 rotation;
    vec3 scale;

    mat4 world_model;
}TransformComponent;

typedef struct
{
    int draw;
    Primitive* mesh;
    Material* material;
}MeshComponent;

typedef struct
{
    LightType type;
    Light light;
}LightComponent;

typedef struct
{
    Camera* camera;
    int last_cam;
}CameraComponent;

typedef struct _Entity
{
    uint32_t entity_id;
    int active;
    TransformComponent transform;
    char* name;

    /* PARENT-CHILD R/SHIPS */

    struct _Entity* parent;
    vtor children;

    /* MORE COMPONENTS HERE */

    MeshComponent* mesh_component;
    LightComponent* light_component;
    CameraComponent* camera_component;

}Entity;

typedef struct
{
    Entity* child;
}EntityChild;

/*!
 * \brief Create a new empty entity
 * \return An empty active entity with no components
 */
Entity* denginescene_ecs_new_entity();

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

void denginescene_ecs_get_front(Entity* entity, vec3 front);

void denginescene_ecs_get_right(Entity* entity, vec3 right);

void denginescene_ecs_get_up(Entity* entity, vec3 up);

void denginescene_ecs_transform_entity(Entity* entity);

MeshComponent* denginescene_ecs_new_meshcomponent(const Primitive* mesh, const Material* material);

CameraComponent* denginescene_ecs_new_cameracomponent(const Camera* camera);

LightComponent* denginescene_ecs_new_lightcomponent(LightType type, Light light);

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif
