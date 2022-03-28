#include "dengine-utils/str.h"
#include "dengine-utils/logging.h"

#include <stdlib.h> //malloc, free
#include <string.h> //strchr, strncmp
#include <stdio.h>

char* dengineutils_str_pat(const char* str, const char* key, const char* val)
{
    if(key[0] != key[strlen(key) - 1])
    {
        dengineutils_logging_log("ERROR::STR_PAT::%s",
                                 "use same key char at start and end : @KEY@");
        return NULL;
    }

    const char* find = strstr(str, key);
    if(!find)
    {
        dengineutils_logging_log("ERROR::STR_PAT::%s",
                                 "key not found");
        return NULL;
    }

    const char* post_str = find + strlen(key);

    size_t pre_str_len = strlen(str) - strlen(find);
    char* pre_str = calloc(pre_str_len, 1);
    strncpy(pre_str, str, pre_str_len);

    size_t new_str_len = strlen(str) + strlen(val);
    char* new_str = calloc(new_str_len, 1);

    snprintf(new_str, new_str_len, "%s%s%s", pre_str,val,post_str);
    free(pre_str);

    return new_str;
}
