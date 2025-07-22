#ifndef DENGINE_ASSETS_H
#define DENGINE_ASSETS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


int dengineutils_assets_zip_load(const char* extrapaths);
void dengineutils_assets_zip_free();
int dengineutils_assets_zip_loaded();
int dengineutils_assets_load(const char* path, void** mem, size_t* length);

#ifdef __cplusplus
}
#endif

#endif
