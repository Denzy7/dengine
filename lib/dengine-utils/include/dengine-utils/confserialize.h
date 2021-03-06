#ifndef CONFSERIALIZE_H
#define CONFSERIALIZE_H

#include <stddef.h> //size_t

/*
 * Serialize ASCII text to create a sort of configuration file
 * eg.
 *
 * # file.ini (can be really anything, .ini is widely accepted. unix like .conf )
 *
 * # confserialize_put_block()
 * [this is a block for organization and is ignored]
 *
 * # confserialize_put_newline()
 *
 *
 * # confserialize_put()
 * # key<separator>pairs into a file
 * # e.g.
 * date=08-01-2022
 * time=19:22
 *
 * # confserialize_put_comment()
 * # this comment line starts with a '#' and is ignored
 */

typedef struct Conf Conf;

#ifdef __cplusplus
extern "C" {
#endif

Conf* dengineutils_confserialize_new(const char* destfile, const char seperator);

int dengineutils_confserialize_load(Conf* conf, int remove_new_line);

void dengineutils_confserialize_put(const char* key, const char* value, Conf* conf);

void dengineutils_confserialize_put_newline(Conf* conf);

void dengineutils_confserialize_put_comment(const char* comment, Conf* conf);

void dengineutils_confserialize_put_block(const char* name, Conf* conf);

char* dengineutils_confserialize_get_value(const char* key, Conf* conf);

size_t dengineutils_confserialize_get_keycount(const Conf* conf);

size_t dengineutils_confserialize_write(Conf* conf);

void dengineutils_confserialize_free(Conf* conf);

#ifdef __cplusplus
}
#endif

#endif // CONFSERIALIZE_H
