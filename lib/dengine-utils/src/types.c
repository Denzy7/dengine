#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "dengine-utils/types.h"
#include "dengine-utils/macros.h"
#include "dengine_config.h"

typedef struct
{
    DengineType type;
    const char* formatstr;
    const size_t size;
}TypeMap;

#ifdef DENGINE_WIN32

#ifdef _WIN64
#define WORDSZ 64
#else
#define WORDSZ 32
#endif

#else
#define WORDSZ __WORDSIZE
#endif

#define UINT32_FMTSTR "%u"
#if WORDSZ == 64 
#define UINT64_FMTSTR "%lu"
#else
#define UINT64_FMTSTR "%llu"
#endif


static const TypeMap dengineutils_types_map[DENGINEUTILS_TYPE_COUNT] = 
{
    {DENGINEUTILS_TYPE_NONE, NULL, sizeof(void) },

#ifdef DENGINE_WIN32
    {DENGINEUTILS_TYPE_POINTER, "0x%p", sizeof(void*) },
#else
    {DENGINEUTILS_TYPE_POINTER, "%p", sizeof(void*) },
#endif
    {DENGINEUTILS_TYPE_UINT8, "%hhu", sizeof(uint8_t) },
    {DENGINEUTILS_TYPE_UINT16, "%hu", sizeof(uint16_t) },
    {DENGINEUTILS_TYPE_UINT32, UINT32_FMTSTR, sizeof(uint32_t) },
    {DENGINEUTILS_TYPE_UINT64, UINT64_FMTSTR, sizeof(uint64_t) },


    {DENGINEUTILS_TYPE_INT8, "%hhd", sizeof(int8_t) },
    {DENGINEUTILS_TYPE_INT16, "%hd", sizeof(int16_t) },
    {DENGINEUTILS_TYPE_INT32, "%d", sizeof(int32_t) },
#if WORDSZ == 64 
    {DENGINEUTILS_TYPE_INT64, "%ld", sizeof(int64_t) },
#else
    {DENGINEUTILS_TYPE_INT64, "%lld", sizeof(int64_t) },
#endif
    {DENGINEUTILS_TYPE_CHAR, "%c", sizeof(char)},

    {DENGINEUTILS_TYPE_FLOAT, "%f", sizeof(float) },
    {DENGINEUTILS_TYPE_DOUBLE, "%lf", sizeof(double) },
#ifdef DENGINE_WIN32
    #if WORDSZ == 64
    {DENGINEUTILS_TYPE_SIZE_T, UINT64_FMTSTR, sizeof(size_t) },
#else
    {DENGINEUTILS_TYPE_SIZE_T, UINT32_FMTSTR, sizeof(size_t) },
#endif
#else   
    {DENGINEUTILS_TYPE_SIZE_T, "%zu", sizeof(size_t) },
#endif
};

const char* dengineutils_types_get_formatstring(DengineType type)
{
    for(size_t i = 0; i < DENGINE_ARY_SZ(dengineutils_types_map); i++)
    {
        if(dengineutils_types_map[i].type == type)
            return dengineutils_types_map[i].formatstr;
    }
    return NULL;
}
size_t dengineutils_types_get_size(DengineType type)
{
    for(size_t i = 0; i < DENGINE_ARY_SZ(dengineutils_types_map); i++)
    {
        if(dengineutils_types_map[i].type == type)
            return dengineutils_types_map[i].size;
    }
    return 0;
}

int dengineutils_types_tostr(DengineType type, char* str, size_t len, const void* src)
{
    const char* fmtstr = dengineutils_types_get_formatstring(type); 

    if(type == DENGINEUTILS_TYPE_UINT8)
        return snprintf(str, len, fmtstr, *(uint8_t*)src);
    else if(type == DENGINEUTILS_TYPE_UINT16)
        return snprintf(str, len, fmtstr, *(uint16_t*)src);
    else if(type == DENGINEUTILS_TYPE_UINT32)
        return snprintf(str, len,fmtstr, *(uint32_t*)src);
    else if(type == DENGINEUTILS_TYPE_UINT64)
        return snprintf(str, len, fmtstr, *(uint64_t*)src);

    else if(type == DENGINEUTILS_TYPE_INT8)
        return snprintf(str, len, fmtstr, *(int8_t*)src);
    else if(type == DENGINEUTILS_TYPE_INT16)
        return snprintf(str, len, fmtstr, *(int16_t*)src);
    else if(type == DENGINEUTILS_TYPE_INT32)
        return snprintf(str, len,fmtstr, *(int32_t*)src);
    else if(type == DENGINEUTILS_TYPE_INT64)
        return snprintf(str, len, fmtstr, *(int64_t*)src);

    else if(type == DENGINEUTILS_TYPE_CHAR)
        return snprintf(str, len, fmtstr, *(char*)src);

    else if(type == DENGINEUTILS_TYPE_FLOAT)
        return snprintf(str, len, fmtstr, *(float*)src);
    else if(type == DENGINEUTILS_TYPE_DOUBLE)
        return snprintf(str, len, fmtstr, *(double*)src);
    else if(type == DENGINEUTILS_TYPE_SIZE_T)
        return snprintf(str, len, fmtstr, *(size_t*)src);
    
    return 0;
}

int dengineutils_types_parse(DengineType type, const char* str, void* dest)
{
    const char* fmtstr = dengineutils_types_get_formatstring(type); 
    if(type == DENGINEUTILS_TYPE_UINT8)
        return sscanf(str, fmtstr, (uint8_t*)dest);
    else if(type == DENGINEUTILS_TYPE_UINT16)
        return sscanf(str, fmtstr, (uint16_t*)dest);
    else if(type == DENGINEUTILS_TYPE_UINT32)
        return sscanf(str,fmtstr, (uint32_t*)dest);
    else if(type == DENGINEUTILS_TYPE_UINT64)
        return sscanf(str, fmtstr, (uint64_t*)dest);

    else if(type == DENGINEUTILS_TYPE_INT8)
        return sscanf(str, fmtstr, (int8_t*)dest);
    else if(type == DENGINEUTILS_TYPE_INT16)
        return sscanf(str, fmtstr, (int16_t*)dest);
    else if(type == DENGINEUTILS_TYPE_INT32)
        return sscanf(str,fmtstr, (int32_t*)dest);
    else if(type == DENGINEUTILS_TYPE_INT64)
        return sscanf(str, fmtstr, (int64_t*)dest);

    else if(type == DENGINEUTILS_TYPE_CHAR)
        return sscanf(str, fmtstr, (char*)dest);

    else if(type == DENGINEUTILS_TYPE_FLOAT)
        return sscanf(str, fmtstr, (float*)dest);
    else if(type == DENGINEUTILS_TYPE_DOUBLE)
        return sscanf(str, fmtstr, (double*)dest);
    else if(type == DENGINEUTILS_TYPE_SIZE_T)
        return sscanf(str, fmtstr, (size_t*)dest);

    return 0;
}
