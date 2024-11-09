#ifndef SCENEMODULE_H
#define SCENEMODULE_H

#include "dengine-script/python/modules/commonmodule.h"

typedef struct
{
    PyObject_HEAD;
    VectorObject *position, *rotation, *scale;
}TransformObject;
DENGINE_EXPORT PyAPI_DATA(PyTypeObject) TransformObject_Type;

typedef struct
{
    PyObject_HEAD;
    TransformObject* transform;
    PyObject* name;
}EntityObject;
DENGINE_EXPORT PyAPI_DATA(PyTypeObject) EntityObject_Type;

/* NOTE: we only declare the related EntityObject types here. No functions depending on libscene!
 * As that would be depending on a downstream library when we are upstream.
 * remember libscene depends libscript thus libscript should not depend on libscene
 *
 * the actual synchronization between the Entity object and python code is actully done inside 
 * libscene. Eventually the object is pushed to python script, update is executes and we get the
 * changed Entity object then set Entity* values 
 */

#ifdef __cplusplus
extern "C" {
#endif

DENGINE_EXPORT int denginescript_python_module_scenemodule_new_entityobject(EntityObject** o);

#ifdef __cplusplus
}
#endif
#endif // SCENEMODULE_H
