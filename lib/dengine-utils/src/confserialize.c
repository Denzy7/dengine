#include "dengine-utils/confserialize.h"
#include <stdio.h>  //fopen
#include <string.h> //strdup
#include <stdlib.h> //malloc
#include "dengine-utils/logging.h" //log

#define MAX_KV_SIZE 100

int dengineutils_confserialize_new(Conf* conf)
{
    //TODO : No need to limit KV_SIZE. Implement a vector of sorts!
    //NOT TO MENTION THIS USES MEMORY INEFFICIENTLY!
    conf->keys = malloc(sizeof(char*) * MAX_KV_SIZE);
    conf->values = malloc(sizeof(char*) * MAX_KV_SIZE);
    conf->keys_count = 0;
    return conf->keys && conf->values;
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

        //TODO : No need to limit KV_SIZE. Implement a vector of sorts!
        //NOT TO MENTION THIS USES MEMORY INEFFICIENTLY!
        conf->keys = malloc(sizeof(char*) * MAX_KV_SIZE);
        conf->values = malloc(sizeof(char*) * MAX_KV_SIZE);
        conf->keys_count = 0;

        while(!feof(f_conf))
        {
            memset(line, 0, line_sz);
            fgets(line, line_sz, f_conf);

            //skip #
            if(line[0] == '#')
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


            if(conf->keys_count == MAX_KV_SIZE)
            {
                dengineutils_logging_log("ERROR::DENGINE_UTILS::CONFSERIALIZE::Reached Max Size! [%d]", MAX_KV_SIZE);
                break;
            }
            conf->keys[conf->keys_count] = malloc((line_size - value_size) + 1);
            memset(conf->keys[conf->keys_count], 0, (line_size - value_size) + 1);
            memcpy(conf->keys[conf->keys_count], line, line_size - value_size);

            conf->values[conf->keys_count] = malloc((value_size - offset) + 1);
            memset(conf->values[conf->keys_count], 0, (value_size - offset) + 1);
            memcpy(conf->values[conf->keys_count], pos + 1, value_size - offset);

            char* value = conf->values[conf->keys_count];
            if(remove_new_line && value[strlen(value) - 1] == '\n')
                value[strlen(value) - 1] = 0;

            conf->keys_count++;
        }

        fclose(f_conf);

        free(line);

        return 1;
    }
}

void dengineutils_confserialize_put(const char* key, const char* value, Conf* conf)
{
    if(conf->keys_count == MAX_KV_SIZE)
    {
        dengineutils_logging_log("ERROR::DENGINE_UTILS::CONFSERIALIZE::Reached Max Size! [%d]", MAX_KV_SIZE);
        return;
    }
    conf->keys[conf->keys_count] = strdup(key);
    conf->values[conf->keys_count] = strdup(value);
    conf->keys_count++;
}

char* dengineutils_confserialize_get(const char* key, Conf* conf)
{
    for(size_t i = 0; i < conf->keys_count; i++)
    {
        if(!strcmp(key, conf->keys[i]))
        {
            return conf->values[i];
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
        for(size_t i = 0; i < conf->keys_count; i++)
        {
            //write space as separator if comment
            char separator = conf->keys[i][0] == '#' ? ' ' : conf->separator;
            write += fprintf(f_conf, "%s%c%s\n", conf->keys[i], separator, conf->values[i]);
        }
    }
    fclose(f_conf);

    return write;
}

void dengineutils_confserialize_free(Conf* conf)
{
    for(size_t i = 0; i < conf->keys_count; i++)
    {
        if(conf->keys[i])
            free(conf->keys[i]);

        if(conf->values[i])
            free(conf->values[i]);
    }

    if(conf->keys)
        free(conf->keys);

    if(conf->values)
        free(conf->values);
}
