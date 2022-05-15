#ifndef SCRIPT_H
#define SCRIPT_H

#include "dengine_config.h" //DENGINE_SCRIPTING_PYTHON

#ifdef DENGINE_SCRIPTING_PYTHON
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#endif

typedef enum
{
    DENGINE_SCRIPT_TYPE_PYTHON = 1,
    DENGINE_SCRIPT_TYPE_NSL
}ScriptType;

//NATIVE SCRIPT LIBRARY types
typedef void* NSL;
typedef int (*nslfunc)(void*);

typedef struct
{
    ScriptType type;

    nslfunc nsl_start;
    nslfunc nsl_update;
    nslfunc nsl_terminate;

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
    DENGINE_SCRIPT_FUNC_TERMINATE,
}ScriptFunc;


#ifdef __cplusplus
extern "C" {
#endif

int denginescript_init();

int denginescript_isinit();

void denginescript_terminate();

int denginescript_compile(const char* src, const char* name, ScriptType type, Script* script);

int denginescript_call(const Script* script, ScriptFunc func, void* args);

int denginescript_isavailable(ScriptType type);

#ifdef DENGINE_SCRIPTING_PYTHON
int denginescript_python_compile(const char* src, const char* name, Script* script);

int denginescript_python_call(const Script* script, ScriptFunc func, const PyObject* args);
#endif

NSL denginescript_nsl_load(const char* file);

void denginescript_nsl_free(NSL nsl);

int denginescript_nsl_get_script(const char* name, Script* script, const NSL nsl);

int denginescript_nsl_call(const Script* script, ScriptFunc func, void* args);
#ifdef __cplusplus
}
#endif

#endif // SCRIPT_H
