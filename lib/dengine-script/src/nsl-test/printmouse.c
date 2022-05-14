#include <dengine/dengine.h>

int printmouse_start(void* args)
{
    dengineutils_logging_log("INFO::started");
    const char* cd = dengineutils_filesys_get_cachedir();
    const char* fd = dengineutils_filesys_get_filesdir();
    printf("cache: %s files %s\n", cd, fd);
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "%s/try.file", cd);
    FILE* f = fopen(buffer, "wb");
    if(f)
    {
        time_t t;
        time(&t);
        struct tm* tm = localtime(&t);
        int wrote = strftime(buffer, sizeof(buffer), "%d-%m-%Y, %H:%M:%S", tm);
        const char* msg = "hello!";
        fwrite(msg, 1, strlen(msg), f);
        fwrite(buffer, 1, wrote, f);
        fclose(f);
    }
    return 1;
}

int printmouse_update(void* args)
{
    static double elapsed;
    elapsed += dengineutils_timer_get_delta();
    if(elapsed > 3000.0)
    {
        char msg[40];
        snprintf(msg, sizeof(msg), "mouse x %f", dengine_input_get_mousepos_x());
        dengineutils_logging_log(msg);

        snprintf(msg, sizeof(msg), "mouse y %f", dengine_input_get_mousepos_y());
        dengineutils_logging_log(msg);

        elapsed = 0.0;
    }

    if(dengine_input_get_key('A'))
        dengineutils_logging_log("WARNING::pressed a");

    if(dengine_input_get_key('X'))
        dengineutils_logging_log("WARNING::pressed x");

    return 1;
}
