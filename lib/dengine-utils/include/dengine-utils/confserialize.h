#ifndef CONFSERIALIZE_H
#define CONFSERIALIZE_H

#include <stddef.h> //size_t
#include "vtor.h" //vtor for kv's
/*
 * Serialize ASCII text,
 * key<separator>pairs into a file
 * e.g.
 * date=08-01-2022
 * time=19:22
 * # <separator> is '='
 * # this line starts with a '#' and is ignored
 */

typedef struct
{
    char* key;
    char* value;
}ConfKV;

typedef struct
{
    char* file;
    char separator;

    vtor keys_values;
}Conf;

#ifdef __cplusplus
extern "C" {
#endif

Conf* dengineutils_confserialize_new(const char* destfile, const char seperator);

int dengineutils_confserialize_load(Conf* conf, int remove_new_line);

void dengineutils_confserialize_put(const char* key, const char* value, Conf* conf);

char* dengineutils_confserialize_get(const char* key, Conf* conf);

size_t dengineutils_confserialize_write(Conf* conf);

void dengineutils_confserialize_free(Conf* conf);

#ifdef __cplusplus
}
#endif

#endif // CONFSERIALIZE_H
