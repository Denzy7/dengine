#ifndef FILESYS_H
#define FILESYS_H

#include "dengine_config.h" //DENGINE_WIN, LINUX
#if defined (DENGINE_LINUX)
#define DENGINE_PATH_SEP '/'
#elif defined(DENGINE_WIN32)
#define DENGINE_PATH_SEP '\\'
#endif

#include <stddef.h> //size_t

//TODO : Implement chunking to load bigger files
typedef struct File2Mem
{
    const char* file;
    void* mem;
    size_t size;
}File2Mem;

#ifdef __cplusplus
extern "C" {
#endif

int dengineutils_filesys_init();

int dengineutils_filesys_isinit();

void dengineutils_filesys_terminate();

int dengineutils_filesys_file2mem_load(File2Mem* file2mem);

void dengineutils_filesys_file2mem_free(File2Mem* file2mem);

const char* dengineutils_filesys_get_srcdir();

const char* dengineutils_filesys_get_assetsdir();

void dengineutils_filesys_set_filesdir(const char* dir);

void dengineutils_filesys_set_cachedir(const char* dir);

const char* dengineutils_filesys_get_filesdir();

const char* dengineutils_filesys_get_filesdir_dengine();

const char* dengineutils_filesys_get_cachedir();

#ifdef __cplusplus
}
#endif

#endif // FILE2MEM_H
