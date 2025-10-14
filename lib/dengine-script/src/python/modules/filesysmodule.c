#include "dengine-utils/filesys.h"
#include <Python.h>

static PyObject* filesys_get_cachedir(PyObject* self, PyObject* args)
{
    return PyUnicode_FromString(dengineutils_filesys_get_cachedir());
}

static PyObject* filesys_get_filesdir(PyObject* self, PyObject* args)
{
    return PyUnicode_FromString(dengineutils_filesys_get_filesdir());
}

static PyMethodDef DengineFileSysModuleMethods[]=
{
    {"get_filesdir", filesys_get_filesdir, METH_NOARGS, "Get the accessible directory for perisitent files"},
    {"get_cachedir", filesys_get_cachedir, METH_NOARGS, "Get the accessible directory for cached files"},
    {NULL, NULL, 0, NULL},
};

static PyModuleDef denginefilesys =
{
    PyModuleDef_HEAD_INIT,
    "filesys",
    "Dengine file system util",
    -1,
    DengineFileSysModuleMethods
};

PyMODINIT_FUNC PyInit_filesys()
{
    return PyModule_Create(&denginefilesys);
}
