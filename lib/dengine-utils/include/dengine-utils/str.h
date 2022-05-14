#ifndef STR_H
#define STR_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif
char* dengineutils_str_pat(const char* str, const char* key, const char* val);

char* dengineutils_str_ndup(const char* str, const size_t n);
#ifdef __cplusplus
}
#endif

#endif // STRPATCH_H
