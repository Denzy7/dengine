#ifndef ECS_H
#define ECS_H

#include <stdint.h> //uint32_t
#include <cglm/cglm.h>

#include "dengine/primitive.h"
#include "dengine/material.h"
#include "dengine/lighting.h"
#include "dengine/camera.h"

typedef struct
{
    vec3 position;
    vec3 rotation;
    vec3 scale;
}TransformComponent;

typedef struct
{
    int draw;
    Primitive* mesh;
    Material* material;
}MeshComponent;

typedef struct
{
    DirLight* dLight;
    PointLight* pLight;
    SpotLight* sLight;
}LightComponent;

typedef struct
{
    Camera* camera;
}CameraComponent;

typedef struct _Entity
{
    uint32_t entity_id;
    int active;
    TransformComponent transform;
    char* name;

    /* PARENT-CHILD R/SHIPS */

    struct _Entity* parent;
    struct _Entity** children;
    uint32_t children_count;

    /* MORE COMPONENTS HERE */

    MeshComponent* mesh_component;
    LightComponent* light_component;
    CameraComponent* camera_component;

}Entity;

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

void denginescene_ecs_get_model(Entity* entity,mat4 mat4x4);

MeshComponent* denginescene_ecs_new_meshcomponent(const Primitive* mesh, const Material* material);

CameraComponent* denginescene_ecs_new_cameracomponent(const Camera* camera);

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif
