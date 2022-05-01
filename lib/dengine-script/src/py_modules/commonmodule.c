#include "commonmodule.h"

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
