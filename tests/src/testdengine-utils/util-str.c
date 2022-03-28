#include <dengine-utils/logging.h>
#include <dengine-utils/str.h>

#include <stdlib.h> //free

int main(int argc, char** argv)
{
    const char* key = "@NAME@";
    const char* str = "Hello There, @NAME@! The time is @TIME@";
    const char* val = "Dennis";
    const char* val2 = "13:34 PM";

    char* patched = dengineutils_str_pat(str, key, val);
    dengineutils_logging_log("key : %s", key);
    dengineutils_logging_log("str : %s", str);
    dengineutils_logging_log("val : %s", val);
    if(patched)
    {
        dengineutils_logging_log("INFO::patched to : %s", patched);
        char* patched2 = dengineutils_str_pat(patched, "@TIME@", val2);
        if(patched2)
        {
            dengineutils_logging_log("INFO::patched2 to : %s", patched2);
            free(patched2);
        }
        free(patched);
    }
}
