#include "dengine-utils/strpatch.h"

#include <stdlib.h> //malloc, free
#include <string.h> //strchr, strncmp

char* dengineutils_strpatch(const char* str, const char* key, const char* val)
{
    if(key[0] != key[strlen(key) - 1])
    {
        //printf("use the same delim as start (%c)\n", key[0]);
        return NULL;
    }

    const char* findstr = strchr(str, key[0]);
    if(findstr && !strncmp(findstr, key, strlen(key)))
    {
        //get end, skip key[0]
        const char* endstr = strchr(findstr + 1, key[0]);
        if(endstr)
        {
            size_t new_str_trim = strlen(str) - strlen(findstr);
            size_t new_str_val = strlen(val);
            //skip end[0]
            size_t new_str_end = strlen(endstr + 1);
            //add all + \0
            size_t new_str_len = new_str_trim + new_str_val + new_str_end + 1;

            char* new_str = malloc(new_str_len);
            memset(new_str, 0, new_str_len);

            //copy trim
            strncpy(new_str, str, new_str_trim);
            //cat val
            strncat(new_str, val, strlen(val));
            //cat end
            strncat(new_str, endstr + 1, strlen(endstr) - 1);
            return new_str;
        }else
        {
            //printf("can't find delim '%c'\n", key[0]);
            return NULL;
        }
    }else
    {
        //printf("key : %s not found!\n", key);
        return NULL;
    }
}
