#ifndef DYNLIB_H
#define DYNLIB_H

#include "dengine_config.h"
typedef void* DynLib;

#ifdef __cplusplus
extern "C" {
#endif

DynLib dengineutils_dynlib_open(const char* path);

void dengineutils_dynlib_close(DynLib dynlib);

void* dengineutils_dynlib_get_sym(const DynLib dynlib, const char* name);

#ifdef __cplusplus
}
#endif

#endif // DYNLIB_H
