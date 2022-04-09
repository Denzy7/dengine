#ifndef SCENE_H
#define SCENE_H

#include "dengine-scene/ecs.h"
#include <stdint.h> //uint32_t

typedef struct _Scene
{
    Entity** entities;
    uint32_t n_entities;
}Scene;

/*!
 * \brief Create an empty scene
 * \return An empty scene
 */
Scene* denginescene_new();

/*!
 * \brief Destroy a scene. Free's all entities and their components
 * \param scene Scene to destroy
 */
void denginescene_destroy(Scene* scene);

/*!
 * \brief Add an entity to a scene
 * \param scene Scene to use
 * \param entity Entity to add
 */
void denginescene_add_entity(Scene* scene, Entity* entity);

void denginescene_update(Scene* scene);

void denginescene_ecs_do_camera_scene(Entity* camera, Scene* scene);

#endif // SCENE_H
