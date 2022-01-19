#include "filesys.h"

#include <stdio.h>  //fopen, fread
#include <stdlib.h> //malloc
#include <string.h> //str

#include "logging.h"
#include "dengine_config.h" //DENGINE_WIN, LINUX
#define PATH_SEP "/"


int dengineutils_filesys_file2mem_load(File2Mem* file2mem)
{
    file2mem->size = 0;
    FILE* fp = fopen(file2mem->file, "r");
    if(!fp)
    {
        dengineutils_logging_log("ERROR::DENGINE_UTILS_FILE2MEM::CANNOT_READ::%s", file2mem->file);
        return 0;
    }else
    {
        fseek(fp, 0, SEEK_END);
        file2mem->size = ftell(fp);
        rewind(fp);

        // +1 = '\0'
        file2mem->mem = malloc(file2mem->size + 1);
        memset(file2mem->mem, 0, file2mem->size + 1);
        if(file2mem)
        {
            fread(file2mem->mem, 1, file2mem->size, fp);
        }else
        {
            dengineutils_logging_log("ERROR::DENGINE_UTILS_FILE2MEM::malloc failed!");
            return 0;
        }

        fclose(fp);
        return 1;
    }
}

void dengineutils_filesys_file2mem_free(File2Mem* file2mem)
{
    if(file2mem->mem)
        free(file2mem->mem);
    else
        dengineutils_logging_log("ERROR::DENGINE_UTILS_FILE2MEM::can't free invalid memory!");
}

