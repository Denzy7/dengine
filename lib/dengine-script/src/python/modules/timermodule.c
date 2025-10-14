#include "dengine-utils/timer.h"
#include <Python.h>

static PyObject* timer_get_current(PyObject* self, PyObject* args)
{
    return PyFloat_FromDouble(dengineutils_timer_get_current());
}

static PyObject* timer_get_delta(PyObject* self, PyObject* args)
{
    return PyFloat_FromDouble(dengineutils_timer_get_delta());
}
static PyObject* timer_get_delta_s(PyObject* self, PyObject* args)
{
    return PyFloat_FromDouble(dengineutils_timer_get_delta() / 1000.0);
}

static PyMethodDef DengineTimerModuleMethods[]=
{
    {"get_delta", timer_get_delta, METH_VARARGS, "Get delta time in ms"},
    {"get_delta_s", timer_get_delta_s, METH_VARARGS, "Get delta time in secs"},
    {"get_current", timer_get_current, METH_VARARGS, "Get current time in ms"},
    {NULL, NULL, 0, NULL},
};

static PyModuleDef denginetimer =
{
    PyModuleDef_HEAD_INIT,
    "timer",
    "Dengine timer util",
    -1,
    DengineTimerModuleMethods
};

PyMODINIT_FUNC PyInit_timer()
{
    return PyModule_Create(&denginetimer);
}
