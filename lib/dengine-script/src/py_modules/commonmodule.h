#ifndef COMMONMODULE_H
#define COMMONMODULE_H

#include "dengine-script/script.h"
#include <structmember.h>

#ifdef __cplusplus
extern "C" {
#endif

PyMODINIT_FUNC PyInit_common();

#ifdef __cplusplus
}
#endif

typedef struct
{
    PyObject_HEAD
    double x;
    double y;
    double z;
}Vec3Object;

static PyMemberDef Vec3Object_Members[]=
{
    {"x", T_DOUBLE, offsetof(Vec3Object, x), 0, "X component"},
    {"y", T_DOUBLE, offsetof(Vec3Object, y), 0, "Y component"},
    {"z", T_DOUBLE, offsetof(Vec3Object, z), 0, "Z component"},
    {NULL}
};

//static PyMethodDef Vec3Object_Methods[] =
//{

//};

static PyTypeObject Vec3Object_Type=
{
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "common.Vec3",
    .tp_doc = "Vector with 3 components (X, Y and Z)",
    .tp_basicsize = sizeof(Vec3Object),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_members = Vec3Object_Members
};

static PyModuleDef commonmodule=
{
    PyModuleDef_HEAD_INIT,
    "common",
    "Dengine common scripting types and methods",
    -1
};

#endif // COMMONMODULE_H
