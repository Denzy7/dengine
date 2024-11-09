#include "dengine-script/script.h"
#include "dengine-utils/logging.h"

static PyObject* logging_log(PyObject* self, PyObject* args)
{
    char* str;
    if(!PyArg_ParseTuple(args, "s", &str))
    {
        PyErr_SetString(PyExc_TypeError, "Pass an already formatted string");
        return NULL;
    }
    dengineutils_logging_log("%s", str);
    return Py_None;
}

static PyMethodDef DengineLoggingModuleMethods[]=
{
    {"log", logging_log, METH_VARARGS, "Log a preformmatted string"},
    {NULL, NULL, 0, NULL},
};

static PyModuleDef denginelogging =
{
    PyModuleDef_HEAD_INIT,
    "logging",
    "Dengine logging util",
    -1,
    DengineLoggingModuleMethods
};

PyMODINIT_FUNC PyInit_logging()
{
    return PyModule_Create(&denginelogging);
}
