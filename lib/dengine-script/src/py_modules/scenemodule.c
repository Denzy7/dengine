#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include "dengine-script/py_modules/scenemodule.h" //Vec3Object

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
    self->position = (Vec3Object*) Vec3Object_Type.tp_new(&Vec3Object_Type, NULL, NULL);
    self->rotation= (Vec3Object*) Vec3Object_Type.tp_new(&Vec3Object_Type, NULL, NULL);
    self->scale = (Vec3Object*) Vec3Object_Type.tp_new(&Vec3Object_Type, NULL, NULL);
    //printf("create transform\n");
    return (PyObject*)self;
}

PyTypeObject TransformObject_Type =
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

static
PyObject* EntityObject_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    EntityObject* self;
    self = (EntityObject*) type->tp_alloc(type, 0);
    self->name = PyUnicode_FromString("");
    self->transform = (TransformObject*)TransformObject_Type.tp_new(&TransformObject_Type, NULL, NULL);
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

PyTypeObject EntityObject_Type=
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

PyObject* denginescript_pymod_scene_entity_new()
{
    return PyObject_CallObject((PyObject*) &EntityObject_Type, NULL);
}

void denginescript_pymod_scene_entity_pull(PyObject* object, Entity* entity)
{
    EntityObject* obj = (EntityObject*)object;

    obj->transform->position->x = entity->transform.position[0];
    obj->transform->position->y = entity->transform.position[1];
    obj->transform->position->z = entity->transform.position[2];

    obj->transform->rotation->x = entity->transform.rotation[0];
    obj->transform->rotation->y = entity->transform.rotation[1];
    obj->transform->rotation->z = entity->transform.rotation[2];

    obj->transform->scale->x = entity->transform.scale[0];
    obj->transform->scale->y = entity->transform.scale[1];
    obj->transform->scale->z = entity->transform.scale[2];

    obj->name = PyUnicode_FromString(entity->name);
}

void  denginescript_pymod_scene_entity_push(PyObject* object, Entity* entity)
{
    EntityObject* obj = (EntityObject*)object;

    entity->transform.position[0] = obj->transform->position->x;
    entity->transform.position[1] = obj->transform->position->y;
    entity->transform.position[2] = obj->transform->position->z;

    entity->transform.rotation[0] = obj->transform->rotation->x;
    entity->transform.rotation[1] = obj->transform->rotation->y;
    entity->transform.rotation[2] = obj->transform->rotation->z;

    entity->transform.scale[0] = obj->transform->scale->x;
    entity->transform.scale[1] = obj->transform->scale->y;
    entity->transform.scale[2] = obj->transform->scale->z;

    //TODO: Push name
}

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
