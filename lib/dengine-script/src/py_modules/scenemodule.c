#include "dengine-script/script.h"
#include "commonmodule.h"

typedef struct
{
    PyObject_HEAD
    Vec3Object position;
    Vec3Object rotation;
    Vec3Object scale;
}TransformObject;

static PyMemberDef TransformObject_Members[]=
{

};

static PyTypeObject TransformObject_Type=
{
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "scene.Transform",
    .tp_doc = "Tranform type",
    .tp_basicsize = sizeof (TransformObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
};

typedef struct
{
    PyObject_HEAD
    TransformObject transform;
}EntityObject;

static PyTypeObject EntityObject_Type=
{
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "scene.Entity",
    .tp_doc = "Entity type",
    .tp_basicsize = sizeof (EntityObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
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
    PyImport_AppendInittab("common", &PyInit_common);

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

//    Py_INCREF(&EntityObject_Type);
//    if(PyModule_AddObject(scenemod, "Entity", (PyObject*) &EntityObject_Type) < 0)
//    {
//        Py_DECREF(&EntityObject_Type);
//        Py_DECREF(scenemod);
//        return NULL;
//    }
    return scenemod;
}
