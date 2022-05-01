#include "commonmodule.h"
#include <structmember.h>

static PyMemberDef Vec3Object_Members[]=
{
    {"x", T_DOUBLE, offsetof(Vec3Object, x), 0, "X component"},
    {"y", T_DOUBLE, offsetof(Vec3Object, y), 0, "Y component"},
    {"z", T_DOUBLE, offsetof(Vec3Object, z), 0, "Z component"},
    {NULL}
};

PyTypeObject Vec3Object_Type=
{
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "common.Vec3",
    .tp_doc = "Vector with 3 components (X, Y and Z)",
    .tp_basicsize = sizeof(Vec3Object),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_members = Vec3Object_Members,

    .tp_new = PyType_GenericNew,
    .tp_alloc = PyType_GenericAlloc,
    .tp_getattro = PyObject_GenericGetAttr,
    .tp_setattro = PyObject_GenericSetAttr
};

static PyModuleDef commonmodule=
{
    PyModuleDef_HEAD_INIT,
    "common",
    "Dengine common scripting types and methods",
    -1
};

PyMODINIT_FUNC PyInit_common()
{
    if(PyType_Ready(&Vec3Object_Type) < 0)
        return NULL;

    PyObject* m = PyModule_Create(&commonmodule);
    if(!m)
        return NULL;

    Py_INCREF(&Vec3Object_Type);
    if(PyModule_AddObject(m, "Vec3", (PyObject*) &Vec3Object_Type) < 0)
    {
        Py_DECREF(&Vec3Object_Type);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}
