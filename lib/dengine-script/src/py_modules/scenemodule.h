#ifndef SCENEMODULE_H
#define SCENEMODULE_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

PyObject* EntityObject_new(PyTypeObject* type, PyObject* args, PyObject* kwds);

#endif // SCENEMODULE_H
