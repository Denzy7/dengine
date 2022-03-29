#include "inptmodule.h"
#include "dengine/input.h"

static PyObject* inpt_getmousepos_x(PyObject* self, PyObject* args)
{
    return PyFloat_FromDouble(dengine_input_get_mousepos_x());
}

static PyObject* inpt_getmousepos_y(PyObject* self, PyObject* args)
{
    return PyFloat_FromDouble(dengine_input_get_mousepos_y());
}

static PyObject* inpt_getkey(PyObject* self, PyObject* args)
{
    char* key = NULL;
    if(!PyArg_ParseTuple(args, "s", &key))
    {
        PyErr_SetString(PyExc_TypeError, "Pass a single letter like printed on your keyboard");
        return NULL;
    }
    char keytoup = toupper(key[0]);
    return PyBool_FromLong(dengine_input_get_key(keytoup));
}

static PyMethodDef DengineInptModuleMethods[]=
{
    {"get_mousex", inpt_getmousepos_x, METH_VARARGS, "Get mouse position X axis"},
    {"get_mousey", inpt_getmousepos_y, METH_VARARGS, "Get mouse position Y axis"},
    {"get_key", inpt_getkey, METH_VARARGS, "Get key using ASCII code equivalent"},
    {NULL, NULL, 0, NULL},
};

static struct PyModuleDef dengineinpt =
{
    PyModuleDef_HEAD_INIT,
    "inpt",
    "Dengine input scripting",
    -1,
    DengineInptModuleMethods
};

PyMODINIT_FUNC PyInit_inpt()
{
    return PyModule_Create(&dengineinpt);
}
