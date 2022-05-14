#ifndef SCENE_H
#define SCENE_H

#include "dengine-scene/ecs.h"
#include <stdint.h> //uint32_t

typedef struct
{
    Material* material;
    Primitive* cube;
}Skybox;

typedef struct _Scene
{
    vtor entities;
    Camera* last_cam;

    Skybox* skybox;

#ifdef DENGINE_SCRIPTING_PYTHON
    PyObject* dummyentityobj;
#endif
}Scene;

/*!
 * \brief Create an empty scene
 * \return An empty scene
 */
Scene* denginescene_new();

Skybox* denginescene_new_skybox(const Primitive* cube,
                                const Material* material);

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

void denginescene_ecs_do_light_scene(Entity* light, Scene* scene);

void denginescene_ecs_do_skybox_scene(Scene* scene, Camera* camera);

void denginescene_ecs_do_script_entity(Entity* entity, ScriptFunc func, void* args);

void denginescene_ecs_do_script_scene(Scene* scene, ScriptFunc func);

#endif // SCENE_H
