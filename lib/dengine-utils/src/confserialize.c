#include "dengine-utils/confserialize.h"
#include <stdio.h>  //fopen
#include <string.h> //strdup
#include <stdlib.h> //malloc
#include "dengine-utils/logging.h" //log

Conf* dengineutils_confserialize_new(const char* destfile, const char seperator)
{
    Conf* conf = malloc(sizeof(Conf));
    memset(conf, 0, sizeof(Conf));
    conf->file = strdup(destfile);
    conf->separator = seperator;

    if(conf)
    {
        vtor_create(&conf->keys_values, sizeof(ConfKV));
    }
    return conf;
}


int dengineutils_confserialize_load(Conf* conf, int remove_new_line)
{
    FILE* f_conf = fopen(conf->file, "r");
    if(!f_conf)
    {
        dengineutils_logging_log("ERROR::DENGINE_UTILS::CONFSERIALIZE::FILE_%s_NOTFOUND!", conf->file);
        return 0;
    }else
    {
        if(!conf->separator)
        {
            dengineutils_logging_log("ERROR::DENGINE_UTILS::CONFSERIALIZE::No key-value Separator Set");
            return 0;
        }

        size_t line_sz = 1024;
        char* line = malloc(line_sz);

        while(!feof(f_conf))
        {
            memset(line, 0, line_sz);
            fgets(line, line_sz, f_conf);

            //skip #, [
            if(line[0] == '#' || line[0] == '[')
                continue;

            char* pos = strchr(line, conf->separator);

            //skip if no sep
            if(!pos)
                continue;

            size_t value_size = strlen(pos);
            size_t line_size = strlen(line);

            size_t offset = 1;
            //offset if new line
            if(line[strlen(line)] == '\n')
                offset = 2;

            ConfKV kv;
            memset(&kv, 0, sizeof(ConfKV));

            kv.key = calloc((line_size - value_size) + 1, 1);
            memcpy(kv.key, line, line_size - value_size);

            kv.value = calloc((value_size - offset) + 1, 1);
            memcpy(kv.value, pos + 1, value_size - offset);

            char* value = kv.value;
            if(remove_new_line && value[strlen(value) - 1] == '\n')
                kv.value[strlen(value) - 1] = 0;

            vtor_pushback(&conf->keys_values, &kv);
        }

        fclose(f_conf);

        free(line);

        return 1;
    }
}

void dengineutils_confserialize_put(const char* key, const char* value, Conf* conf)
{
    ConfKV kv;
    memset(&kv, 0, sizeof(ConfKV));
    kv.key = strdup(key);
    kv.value = strdup(value);
    vtor_pushback(&conf->keys_values, &kv);
}

char* dengineutils_confserialize_get(const char* key, Conf* conf)
{
    ConfKV* kv = conf->keys_values.data;
    for(size_t i = 0; i < conf->keys_values.count; i++)
    {
        if(!strcmp(key, kv[i].key))
        {
            return kv[i].value;
        }
    }
    return NULL;
}

size_t dengineutils_confserialize_write(Conf* conf)
{
    size_t write = 0;
    FILE* f_conf = fopen(conf->file, "w");
    if(!f_conf)
    {
        dengineutils_logging_log("ERROR::DENGINE_UTILS::CONFSERIALIZE::Cannot write to %s", conf->file);
    }else
    {
        ConfKV* kv = conf->keys_values.data;
        for(size_t i = 0; i < conf->keys_values.count; i++)
        {
            //write space as separator if comment
            char separator = kv[i].key[0] == '#' ? ' ' : conf->separator;
            write += fprintf(f_conf, "%s%c%s\n", kv[i].key, separator, kv[i].value);
        }
    }
    fclose(f_conf);

    return write;
}

void dengineutils_confserialize_free(Conf* conf)
{
    ConfKV* kv = conf->keys_values.data;
    for(size_t i = 0; i < conf->keys_values.count; i++)
    {
        if(kv[i].key)
            free(kv[i].key);

        if(kv[i].value)
            free(kv[i].value);
    }

    vtor_free(&conf->keys_values);

    free(conf->file);
    free(conf);
}
