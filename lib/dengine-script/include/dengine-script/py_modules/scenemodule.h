#ifndef SCENEMODULE_H
#define SCENEMODULE_H

#include "commonmodule.h"
#include "dengine-scene/scene.h"

typedef struct
{
    PyObject_HEAD
    Vec3Object* position;
    Vec3Object* rotation;
    Vec3Object* scale;
}TransformObject;
extern PyTypeObject TransformObject_Type;

typedef struct
{
    PyObject_HEAD
    TransformObject* transform;
    PyObject* name;
}EntityObject;
extern PyTypeObject EntityObject_Type;

// new Python Object as an entity
PyAPI_FUNC(PyObject *) denginescript_pymod_scene_entity_new();

// pull data from ecs
PyAPI_FUNC(void) denginescript_pymod_scene_entity_pull(PyObject*, Entity*);

//push/commit changes to PyObject to ecs
PyAPI_FUNC(void) denginescript_pymod_scene_entity_push(PyObject*, Entity*);

#endif // SCENEMODULE_H
