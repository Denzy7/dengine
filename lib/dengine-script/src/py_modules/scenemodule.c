#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include "commonmodule.h" //Vec3Object

typedef struct
{
    PyObject_HEAD
    Vec3Object* position;
    Vec3Object* rotation;
    Vec3Object* scale;
}TransformObject;

static PyMemberDef TransformObject_Members[]=
{
    {"position", T_OBJECT_EX, offsetof(TransformObject, position), 0, "XYZ world position"},
    {"rotation", T_OBJECT_EX, offsetof(TransformObject, rotation), 0, "XYZ euler rotation in degrees"},
    {"scale", T_OBJECT_EX, offsetof(TransformObject, scale), 0, "XYZ world scale"},
    {NULL}
};

PyObject* TransformObject_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    TransformObject* self;
    self = (TransformObject*) type->tp_alloc(type, 0);
    self->position = (Vec3Object*) Vec3Object_Type.tp_new(&Vec3Object_Type, NULL, NULL);
    self->rotation= (Vec3Object*) Vec3Object_Type.tp_new(&Vec3Object_Type, NULL, NULL);
    self->scale = (Vec3Object*) Vec3Object_Type.tp_new(&Vec3Object_Type, NULL, NULL);
    return (PyObject*)self;
}

static PyTypeObject TransformObject_Type=
{
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "scene.Transform",
    .tp_doc = "Transform component",
    .tp_basicsize = sizeof (TransformObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_members = TransformObject_Members,

    .tp_new = TransformObject_new,
    .tp_alloc = PyType_GenericAlloc,
};

typedef struct
{
    PyObject_HEAD
    TransformObject* transform;
}EntityObject;

static
PyObject* EntityObject_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    EntityObject* self;
    self = (EntityObject*) type->tp_alloc(type, 0);
    self->transform = (TransformObject*)TransformObject_Type.tp_new(&TransformObject_Type, NULL, NULL);
    //printf("alloc entity\n");
    return (PyObject*) self;
}

static
PyMemberDef EntityObject_Members[]=
{
    {"transform", T_OBJECT_EX, offsetof(EntityObject, transform), 0, "Transform component"},
    {NULL}
};

static PyTypeObject EntityObject_Type=
{
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "scene.Entity",
    .tp_doc = "Entity type with set of components",
    .tp_basicsize = sizeof (EntityObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_members = EntityObject_Members,

    .tp_new = EntityObject_new,
};

static PyModuleDef scenemodule =
{
    PyModuleDef_HEAD_INIT,
    "scene",
    "Dengine scene scripting and types",
    -1
};

PyMODINIT_FUNC PyInit_scene()
{
    if(PyType_Ready(&TransformObject_Type) < 0)
        return NULL;

    if(PyType_Ready(&EntityObject_Type) < 0)
        return NULL;

    PyObject* scenemod = PyModule_Create(&scenemodule);
    if(!scenemod)
        return NULL;

    Py_INCREF(&TransformObject_Type);
    if(PyModule_AddObject(scenemod, "Transform", (PyObject*) &TransformObject_Type) < 0)
    {
        Py_DECREF(&TransformObject_Type);
        Py_DECREF(scenemod);
        return NULL;
    }

    Py_INCREF(&EntityObject_Type);
    if(PyModule_AddObject(scenemod, "Entity", (PyObject*) &EntityObject_Type) < 0)
    {
        Py_DECREF(&EntityObject_Type);
        Py_DECREF(scenemod);
        return NULL;
    }
    return scenemod;
}
