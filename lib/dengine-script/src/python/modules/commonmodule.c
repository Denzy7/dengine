#include "dengine-script/script.h"
#include "dengine-script/python/modules/commonmodule.h"
#include <structmember.h>

PyMemberDef VecObject_Members[]=
{
    /* NOTE: the first 4 values (VALUES_STR, x, y, z, w} should be ordered as is 
     * as it will break if not as is the oode below loops over it expecting that */
    {"x", T_DOUBLE, offsetof(VectorObject, _xyzw[0]), 0, "x"},
    {"y", T_DOUBLE, offsetof(VectorObject, _xyzw[1]), 0, "y"},
    {"z", T_DOUBLE, offsetof(VectorObject, _xyzw[2]), 0, "z"},
    {"w", T_DOUBLE, offsetof(VectorObject, _xyzw[3]), 0, "w"},

    {NULL}
};

PyObject* VectorObject_str(VectorObject* self)
{
    const size_t chnksz = 64;
    char ret[chnksz * 4];
    char val[chnksz];
    size_t len = 0;
    for(size_t i = 0; i < 4; i++){
        const char* idx = VecObject_Members[i].name;
        snprintf(val, chnksz, "%f", self->_xyzw[i]);
#define basefmtstr "%s: %s"
        const char* fmtstr = basefmtstr;
        if(i != 3)
            fmtstr = basefmtstr", ";

        snprintf(ret + len, chnksz, fmtstr, idx, val);
        len = strlen(ret);
    }

    return PyUnicode_FromString(ret);
}

int VectorObject_setattro(PyObject* o, PyObject* a, PyObject* v)
{
    /*TODO: guard this is a debug flag? might speed up */
#if 1
    if(v == NULL)
    {
        PyErr_Format(PyExc_TypeError, "What are you doing? You should not delete a built-in Vector member!");
        return -1;
    }
#endif

    return PyObject_GenericSetAttr(o, a, v);
}

PyObject* VectorObject_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    VectorObject* self;
    if(type->tp_dict == NULL)
        if(PyType_Ready(type) < 0)
            return NULL;

    self = (VectorObject*) type->tp_alloc(type, 0);
    return (PyObject*)self;
}

DENGINE_EXPORT PyTypeObject VectorObject_Type=
{
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "common.Vector",
    .tp_doc = "Vector with up to 4 components (X, Y, Z, W)",
    .tp_basicsize = sizeof(VectorObject),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_members = VecObject_Members,
    .tp_str = (reprfunc)VectorObject_str,
    .tp_setattro = VectorObject_setattro,
    .tp_new = VectorObject_new
};

PyModuleDef commonmodule=
{
    PyModuleDef_HEAD_INIT,
    "common",
    "Dengine common scripting types and methods",
    -1
};

PyMODINIT_FUNC PyInit_common()
{
    if(PyType_Ready(&VectorObject_Type) < 0)
        return NULL;

    PyObject* m = PyModule_Create(&commonmodule);
    if(!m)
        return NULL;

    Py_INCREF(&VectorObject_Type);
    if(PyModule_AddObject(m, "Vector", (PyObject*) &VectorObject_Type) < 0)
    {
        Py_DECREF(&VectorObject_Type);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}
