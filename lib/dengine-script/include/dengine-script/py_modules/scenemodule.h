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
PyObject* denginescript_pymod_scene_entity_new();

// pull data from ecs
void denginescript_pymod_scene_entity_pull(PyObject* object, Entity* entity);

//push/commit changes to PyObject to ecs
void denginescript_pymod_scene_entity_push(PyObject* object, Entity* entity);

#endif // SCENEMODULE_H
