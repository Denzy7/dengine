#include <dengine/dengine.h>

int main(int argc, char *argv[])
{
    DengineInitOpts* opts = dengine_init_get_opts();
    opts->window_title = "testdengine-script-nslbullet";
    dengine_init();

    NSL nsl = denginescript_nsl_load("./nsl-bulletphysics.nsl");
    if(!nsl)
        return -1;

    Script basic_world;
    denginescript_nsl_get_script("basic_world", &basic_world, nsl);

    //manually call start to create world
    denginescript_call(&basic_world, DENGINE_SCRIPT_FUNC_START, NULL);

    /*
     * this script(plugin) uses bulletphysics to move an entity object.
     *
     * Notice how NSL enables C and C++ to communicate with the engine
     *
     * Build as shared to avoid link errors
     *
     * NSL can also access anything in the eco-system and is more powerful than regular
     * python
     */

    while (dengine_update()) {
        //stepSimulation
        denginescript_call(&basic_world, DENGINE_SCRIPT_FUNC_UPDATE, NULL);
    }
    //destroy world
    denginescript_call(&basic_world, DENGINE_SCRIPT_FUNC_TERMINATE, NULL);
    denginescript_nsl_free(nsl);

    dengine_terminate();
    return 0;
}
