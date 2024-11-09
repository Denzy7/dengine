#ifndef SCRIPT_H
#define SCRIPT_H

#include <stddef.h>
#include "dengine_config.h" //DENGINE_SCRIPTING_PYTHON
#include "dengine-utils/dynlib.h"
#include "dengine-utils/macros.h"

#ifdef DENGINE_SCRIPTING_PYTHON
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#endif

typedef struct 
{
    const char* name;
    void* address;
    size_t size;
}NSLExports;

typedef enum
{
    DENGINE_SCRIPT_TYPE_PYTHON = 1,
    DENGINE_SCRIPT_TYPE_NSL
}ScriptType;

//NATIVE SCRIPT LIBRARY types
typedef DynLib NSL;
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

/* sidenote:
 * denginescript_[ call | compile | ... ] all get the appropriate function
 * by checking the script type. you can skip the check and directly call
 * the appropritate func if you know the script type. so for python you would
 * call denginescript_python_[call | compile | ...]. 
 * or denginescript_nsl_[call | compile | ...] if its NSL
 *
 * NSL is always available since it's just a shared library*/

#ifdef __cplusplus
extern "C" {
#endif

DENGINE_EXPORT int denginescript_init();

DENGINE_EXPORT int denginescript_isinit();

DENGINE_EXPORT void denginescript_terminate();

int denginescript_compile(const char* src, const char* name, ScriptType type, Script* script);

DENGINE_EXPORT int denginescript_call(const Script* script, ScriptFunc func, void* args);

int denginescript_isavailable(ScriptType type);

DENGINE_EXPORT NSL denginescript_nsl_load(const char* file);

DENGINE_EXPORT void denginescript_nsl_free(NSL nsl);

DENGINE_EXPORT int denginescript_nsl_get_script(const char* name, Script* script, const NSL nsl);

int denginescript_nsl_call(const Script* script, ScriptFunc func, void* args);

nslfunc denginescript_nsl_get_func(const NSL nsl, const char* name);
#ifdef __cplusplus
}
#endif

#endif // SCRIPT_H
