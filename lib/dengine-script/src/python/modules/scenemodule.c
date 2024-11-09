#include "dengine-script/python/modules/scenemodule.h"
#include "dengine-script/script.h"
#include "dengine-script/python/modules/commonmodule.h" //Vec3Object
#include <structmember.h>                                                        

static PyMemberDef TransformObject_Members[]=
{
    {"position", T_OBJECT_EX, offsetof(TransformObject, position), 0, "XYZ world position"},
    {"rotation", T_OBJECT_EX, offsetof(TransformObject, rotation), 0, "XYZ euler rotation in degrees"},
    {"scale", T_OBJECT_EX, offsetof(TransformObject, scale), 0, "XYZ world scale"},
    {NULL}
};

static
PyObject* TransformObject_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    TransformObject* self;

    self = (TransformObject*) type->tp_alloc(type, 0);
    self->position = (VectorObject*) PyObject_CallObject((PyObject*)&VectorObject_Type, NULL); 
    self->rotation = (VectorObject*) PyObject_CallObject((PyObject*)&VectorObject_Type, NULL);
    self->scale = (VectorObject*) PyObject_CallObject((PyObject*)&VectorObject_Type, NULL);

    //printf("create transform\n");
    return (PyObject*)self;
}

DENGINE_EXPORT PyTypeObject TransformObject_Type =
{
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "scene.Transform",
    .tp_doc = "Transform component",
    .tp_basicsize = sizeof (TransformObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_members = TransformObject_Members,
    .tp_new = TransformObject_new,
    /*.tp_alloc = PyType_GenericAlloc,*/
};

static
PyObject* EntityObject_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    EntityObject* self;

    self = (EntityObject*) type->tp_alloc(type, 0);
    self->transform = (TransformObject*)PyObject_CallObject((PyObject*)&TransformObject_Type, NULL);
    //printf("alloc entity\n");
    return (PyObject*) self;
}

static
PyMemberDef EntityObject_Members[]=
{
    {"transform", T_OBJECT_EX, offsetof(EntityObject, transform), 0, "Transform component"},
    {"name", T_OBJECT_EX, offsetof(EntityObject, name), 0, "Name of the entity"},
    {NULL}
};

DENGINE_EXPORT PyTypeObject EntityObject_Type=
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


DENGINE_EXPORT int denginescript_python_module_scenemodule_new_entityobject(EntityObject** o)
{
    if(PyType_Ready(&EntityObject_Type) < 0)
        return 0;

    PyObject* ret = PyObject_CallObject((PyObject*) &EntityObject_Type, NULL);

    *o =  (EntityObject*)ret;
    return 1;
}
