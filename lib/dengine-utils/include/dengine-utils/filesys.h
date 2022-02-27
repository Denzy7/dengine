#ifndef FILESYS_H
#define FILESYS_H

#include <stddef.h> //size_t

//TODO : Implement chunking to load bigger files
typedef struct File2Mem
{
    char* file;
    void* mem;
    size_t size;
}File2Mem;


#ifdef __cplusplus
extern "C" {
#endif


int dengineutils_filesys_file2mem_load(File2Mem* file2mem);

void dengineutils_filesys_file2mem_free(File2Mem* file2mem);

const char* dengineutils_filesys_compiledir();

#ifdef __cplusplus
}
#endif

#endif // FILE2MEM_H
