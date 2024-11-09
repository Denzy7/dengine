#ifndef DENGINESCRIPT_PYTHON_PYTHON_H
#define DENGINESCRIPT_PYTHON_PYTHON_H

#include "dengine-script/script.h"

#ifdef __cplusplus
extern "C" {
#endif

int denginescript_python_init();

int denginescript_python_isinit();

void denginescript_python_terminate();

DENGINE_EXPORT extern struct _inittab denginescript_python_inittab[];

DENGINE_EXPORT int denginescript_python_compile(const char* src, const char* name, Script* script);

DENGINE_EXPORT int denginescript_python_call(const Script* script, ScriptFunc func, PyObject* args);

#ifdef __cplusplus
}
#endif

#endif

