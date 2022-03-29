#include "timermodule.h"

#include "dengine-utils/timer.h"

static PyObject* timer_get_current(PyObject* self, PyObject* args)
{
    return PyFloat_FromDouble(dengineutils_timer_get_current());
}

static PyObject* timer_get_delta(PyObject* self, PyObject* args)
{
    return PyFloat_FromDouble(dengineutils_timer_get_delta());
}

static PyMethodDef DengineInptModuleMethods[]=
{
    {"get_delta", timer_get_delta, METH_VARARGS, "Get delta time in ms"},
    {"get_current", timer_get_current, METH_VARARGS, "Get current time in ms"},
    {NULL, NULL, 0, NULL},
};

static struct PyModuleDef denginetimer =
{
    PyModuleDef_HEAD_INIT,
    "timer",
    "Dengine timer utils",
    -1,
    DengineInptModuleMethods
};

PyMODINIT_FUNC PyInit_timer()
{
    return PyModule_Create(&denginetimer);
}
