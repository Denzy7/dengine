#include <dengine/dengine.h>

int main(int argc, char *argv[])
{
    DengineInitOpts* opts = dengine_init_get_opts();
    opts->window_title = "testdengine-script-nslpulseaudio";
    dengine_init();

    NSL pa = denginescript_nsl_load("./nsl-pulseaudio.nsl");
    if(pa == NULL)
        return 1;

    Script simple;
    denginescript_nsl_get_script("simple", &simple, pa);

    if(denginescript_call(&simple, DENGINE_SCRIPT_FUNC_START, argv[1]) != 0)
    {
        dengineutils_logging_log("ERROR::Cannot init pulseaudio simple script");
        return 1;
    }

    //play whole ogg then exit
    while(dengine_update())
    {
        if(denginescript_call(&simple, DENGINE_SCRIPT_FUNC_UPDATE, NULL))
            break;
    }
    denginescript_call(&simple, DENGINE_SCRIPT_FUNC_TERMINATE, NULL);
    dengine_terminate();

    return 0;
}
