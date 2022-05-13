#ifndef SCRIPT_H
#define SCRIPT_H

#include "dengine_config.h" //DENGINE_SCRIPTING_PYTHON

#ifdef DENGINE_SCRIPTING_PYTHON

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#endif

typedef struct
{
#ifdef DENGINE_SCRIPTING_PYTHON

    PyObject* bytecode;
    PyObject* module;

    PyObject* fn_start;
    PyObject* fn_update;

#endif
}Script;

typedef enum
{
    DENGINE_SCRIPT_FUNC_START,
    DENGINE_SCRIPT_FUNC_UPDATE,
}ScriptFunc;

typedef enum
{
    DENGINE_SCRIPT_TYPE_PYTHON,
}ScriptType;

#ifdef __cplusplus
extern "C" {
#endif

int denginescript_init();

void denginescript_terminate();

int denginescript_compile(const char* src, const char* name, ScriptType type, Script* script);

int denginescript_call(ScriptType type, const Script* script, ScriptFunc func, const void* args);

int denginescript_isavailable(ScriptType type);

#ifdef DENGINE_SCRIPTING_PYTHON
int denginescript_python_compile(const char* src, const char* name, Script* script);

int denginescript_python_call(const Script* script, ScriptFunc func, const PyObject* args);
#endif

#ifdef __cplusplus
}
#endif

#endif // SCRIPT_H
