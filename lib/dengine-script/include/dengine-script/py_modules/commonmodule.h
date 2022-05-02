#ifndef COMMONMODULE_H
#define COMMONMODULE_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    PyObject_HEAD
    double x;
    double y;
    double z;
}Vec3Object;

extern PyTypeObject Vec3Object_Type;

#ifdef __cplusplus
}
#endif

#endif // COMMONMODULE_H
