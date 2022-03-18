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
    mat4 modelmtx;
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
 * \brief Destroy an allocated entity and its underlying entities. Does not destoy
 * components
 * \param root Entity to destroy
 */
void denginescene_ecs_destroy_entity(Entity* root);

/*!
 * \brief Parent a child to a parent entity
 * \param parent Parent to use
 * \param child Child to use
 */
void denginescene_ecs_parent(struct _Entity* parent, struct _Entity* child);

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif