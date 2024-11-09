#ifndef DENGINEUTILS_TYPES_H
#define DENGINEUTILS_TYPES_H

#include <stddef.h>

/* basically aliases to base types (and some base derived types like size_t)
 * and their printf format strings 
 */
typedef enum
{
    DENGINEUTILS_TYPE_NONE, /* void */
    DENGINEUTILS_TYPE_POINTER,

    DENGINEUTILS_TYPE_UINT8,
    DENGINEUTILS_TYPE_UINT16,
    DENGINEUTILS_TYPE_UINT32,
    DENGINEUTILS_TYPE_UINT64,

    DENGINEUTILS_TYPE_INT8,
    DENGINEUTILS_TYPE_INT16,
    DENGINEUTILS_TYPE_INT32,
    DENGINEUTILS_TYPE_INT64,

    DENGINEUTILS_TYPE_CHAR,

    DENGINEUTILS_TYPE_FLOAT,
    DENGINEUTILS_TYPE_DOUBLE,

    DENGINEUTILS_TYPE_SIZE_T,

    DENGINEUTILS_TYPE_COUNT
}DengineType;

#ifdef __cplusplus
extern "C" {
#endif

const char* dengineutils_types_get_formatstring(DengineType type);

size_t dengineutils_types_get_size(DengineType type);

int dengineutils_types_parse(DengineType type, const char* str, void* dest);

int dengineutils_types_tostr(DengineType type, char* str, size_t sz, const void* src);

#ifdef __cplusplus
}
#endif

#endif
