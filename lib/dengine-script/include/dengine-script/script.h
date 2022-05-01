#ifndef SCRIPT_H
#define SCRIPT_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

typedef struct
{
    PyObject* bytecode;
    PyObject* module;

    PyObject* fn_start;
    PyObject* fn_update;
}PyScript;

typedef enum
{
    DENGINE_SCRIPT_FUNC_START,
    DENGINE_SCRIPT_FUNC_UPDATE,
}PyScriptFunc;

#ifdef __cplusplus
extern "C" {
#endif

int denginescript_init();

void denginescript_terminate();

PyScript* denginescript_python_new(const char* src, const char* name);

void denginescript_python_call(const PyScript* script, PyScriptFunc func, PyObject* args);

#ifdef __cplusplus
}
#endif

#endif // SCRIPT_H
