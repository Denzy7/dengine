#include <dengine-utils/logging.h>
#include <dengine-utils/strpatch.h>

#include <stdlib.h> //free
int main(int argc, char** argv)
{
    const char* key = "@NAME@";
    const char* str = "Hello There, @NAME@";
    const char* val = "Dennis";
    char* patched = dengineutils_strpatch(str, key, val);
    dengineutils_logging_log("key : %s", key);
    dengineutils_logging_log("str : %s", str);
    dengineutils_logging_log("val : %s", val);
    if(patched)
    {
        dengineutils_logging_log("INFO::patched to : %s", patched);
        free(patched);
    }
}
