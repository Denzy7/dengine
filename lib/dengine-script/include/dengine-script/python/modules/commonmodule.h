#ifndef COMMONMODULE_H
#define COMMONMODULE_H

#include "dengine-script/script.h"

typedef struct
{
    PyObject_HEAD;
    double _xyzw[4]; /* this is actually not used and is only here to appease the interpretor */
}VectorObject;

 DENGINE_EXPORT PyAPI_DATA(PyTypeObject) VectorObject_Type;

#endif // COMMONMODULE_H
