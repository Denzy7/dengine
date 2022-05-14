#include <dengine/dengine.h>
int main(int argc, char *argv[])
{
    DengineInitOpts* opts = dengine_init_get_opts();
    opts->window_title = "testdengine-script-nsl";
    dengine_init();

    NSL nsl = denginescript_nsl_load("nsl-test.nsl");
    if(!nsl)
        return -1;

    const size_t prtbf_sz = 2048;
    char* prtbf = malloc(prtbf_sz);

    Script printmouse;
    int get = denginescript_nsl_get_script("printmouse", &printmouse, nsl);

    /*
     * this script(plugin) does not use ecs. so we just pass NULL to update() and start().
     *
     * NSL loads symbols in the library in the form of <script>_update and <script>_start
     *
     * Build as shared to avoid link errors
     *
     * NSL can also access anything in the eco-system and is more powerful than regular
     * python
     */

    if(get)
        denginescript_call(&printmouse, DENGINE_SCRIPT_FUNC_START, NULL);

    while (dengine_window_isrunning()) {
        if(get)
            denginescript_call(&printmouse, DENGINE_SCRIPT_FUNC_UPDATE, NULL);
        dengine_update();
    }
    free(prtbf);
    denginescript_nsl_free(nsl);

    dengine_terminate();
    return 0;
}
