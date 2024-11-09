#ifndef _DENGINESCENE_QUEUE_H
#define _DENGINESCENE_QUEUE_H

/*THIS HEADER SHOULD ONLY HAVE DEFINES. IT'S INCLUDED 
 * IN UPSTREAM LIBRARIES
 */

/* the concept of a queue is actually simple
 * each scene has queues which have pointers to 
 * entities. queues are organized in order of execution i.e
 * scripts are executed and if there are any  entities in the camera 
 * queue trigger the rendering process i.e transform lights, render
 * skybox, opaques then transparents, then UI
 *
 * any shadows will be first rendered before the camera queue is executed
 * so shaders can receive up to date shadow maps. this also means shadows 
 * will still be rendered despite there being no active cameras
 * 
 * each queue uses some memory to store entities in a quickly accessible vector
 * so we only recurse to transform entities. accessing entites recursively to render stuff
 * is very expensive and must be avoided at all costs, hence the queues must be
 * a vector of entity pointers
 */
typedef enum
{
    /* phantom queues. used to store 
     * auxilliary queue data*/
    DENGINESCENE_QUEUE_SCRIPT,
    DENGINESCENE_QUEUE_MESH,

    /* components. execution for each camera starts here*/
    DENGINESCENE_QUEUE_CAMERA,
    DENGINESCENE_QUEUE_LIGHT_DIR, /* yes i know we only support one dirlight but who knows what the future holds? */
    DENGINESCENE_QUEUE_LIGHT_POINT,
    DENGINESCENE_QUEUE_LIGHT_SPOT,
    DENGINESCENE_QUEUE_MESH_OPAQUE,
    DENGINESCENE_QUEUE_MESH_TRANSPARENT,

    /* scene execution order */
    DENGINESCENE_QUEUE_UI,

    DENGINESCENE_QUEUE_COUNT
}SceneQueueType;

#endif
