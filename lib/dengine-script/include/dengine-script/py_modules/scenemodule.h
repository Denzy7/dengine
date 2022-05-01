#ifndef SCENEMODULE_H
#define SCENEMODULE_H

#include "commonmodule.h"

typedef struct
{
    PyObject_HEAD
    Vec3Object* position;
    Vec3Object* rotation;
    Vec3Object* scale;
}TransformObject;
PyAPI_DATA(PyTypeObject) TransformObject_Type;

typedef struct
{
    PyObject_HEAD
    TransformObject* transform;
}EntityObject;
PyAPI_DATA(PyTypeObject) EntityObject_Type;

#endif // SCENEMODULE_H
