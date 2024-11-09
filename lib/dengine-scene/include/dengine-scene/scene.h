#ifndef SCENE_H
#define SCENE_H

#include "dengine-scene/ecs.h"
#include "dengine-scene/queue.h"
#ifdef DENGINE_SCRIPTING_PYTHON
#include "dengine-script/python/modules/scenemodule.h"
#endif
typedef struct
{
    Material material;
    Primitive cube;
}Skybox;
typedef struct{
    SceneQueueType type;
    /* this is QueueFunc but can't be defined earlier coz
     * QueueFunc depends on Scene to be defined which is 
     * not yet defined */
    void* queue_func;
    vtor entities;
}SceneQueue;
typedef enum
{
    /*never runs */
    DENGINESCENE_QUEUER_STATE_PAUSED,

    /* runs when something changed in scene  ie. something got added. setting it to running automatically queues */
    DENGINESCENE_QUEUER_STATE_RUNNING,

    /* run every frame. very dangerously slow. no need to pause or resume queuer manually when adding multiple entities */
    DENGINESCENE_QUEUER_STATE_ALWAYSRUNNING,
}SceneQueuerState;

typedef struct
{
    SceneQueuerState queuerstate;

    /*
     * processed FIFO. we want transparency
     * , gui etc to be last
     */
    SceneQueue queues[DENGINESCENE_QUEUE_COUNT];

    /* material batch info for opaque objects */
    vtor batchinfo;

    /* entity info for transparent entities for sorting */
    vtor transparenyinfo;

    /* pointers to entities.
     * dont dare pushback a Entity here
     * its Entity*
     */
    vtor entities;
    Entity* last_cam;

    Skybox* skybox;


    /* PRIVATE FIELDS. DON'T CHANGE ANYTHING FROM 
     * DOWN HERE!
     */
    int __axisdraw;
    Primitive __axis;
    int __griddraw;
    Primitive __grid_a;
    Primitive __grid_b;
    Shader __dftshdr;

#ifdef DENGINE_SCRIPTING_PYTHON
    EntityObject* entityobj; /* we only need since it fully pushed 
                                before calling function. unless special 
                                cases it not? saves a bit of memory 
                                */
#endif
}Scene;

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Create an empty scene. Any resources added to scene 
 * are automatically freed so beware of double free
 *
 * \param scene input scene
 * \return 1 if ok, else error
 */
int denginescene_new(Scene** scene);

int denginescene_new_skybox(const Primitive* cube, const Material* material, Skybox** skybox);

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

/* pause the queuer when 
 * adding many scene items to avoid 
 * lag spike. it has to realloc arrays which 
 * is done for each item*/
void denginescene_queuer_state(Scene* scene, SceneQueuerState state);

/* convert the scene tree into an easy to digest
 * arrays for fast operations. need only to run 
 * when scene structure changes:
 * 1. reparenting
 * 2. adding entity to scene
 * etc
 */
void denginescene_queuer_queue(Scene* scene);

/* update the scene,
 * set the camera, lights, render
 * run scripts etc
 */
void denginescene_update(Scene* scene);

/* for debugging scene hierachy 
 * isssue or just general 
 * visualization
 */
void denginescene_dumphierachy_stdio(Scene* scene, FILE* stdiof);

/* dump queue statistics
 */
void denginescene_queuer_dumpstats_stdio(Scene* scene, FILE* stdiof);

/* run all scene scripts 
 * useful for starting the scene*/
void denginescene_scripts_run(Scene* scene, ScriptFunc func);

/* draw a debug axis or grid for all entities into last cam 
 * framebuffer. uses the axis primitive and default shader
 */
void denginescene_set_debugaxis(Scene* scene, int state);
void denginescene_set_debuggrid(Scene* scene, int state);

/* render the scene with this camera entity */
void denginescene_ecs_do_camera_scene(Entity* camera, Scene* scene);

#ifdef __cplusplus
}
#endif

#endif // SCENE_H
