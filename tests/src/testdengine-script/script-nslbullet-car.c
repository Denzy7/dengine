#include <dengine/dengine.h>

int testdengine_script_nslbullet_car_main(int argc, char **argv)
{
    NSL nsl;
#ifdef DENGINE_ANDROID
    nsl = denginescript_nsl_load("libnsl_bulletcar_android.so");
#else
    nsl = denginescript_nsl_load("./nsl-bulletphysics-car.nsl");
#endif
    if(!nsl)
        return -1;
    Script car_world;
    denginescript_nsl_get_script("car_world", &car_world, nsl);
    //manually call start to create world
    denginescript_call(&car_world, DENGINE_SCRIPT_FUNC_START, NULL);
    
    while (dengine_update()) {
        //stepSimulation
        denginescript_call(&car_world, DENGINE_SCRIPT_FUNC_UPDATE, NULL);
    }
    //destroy world
    denginescript_call(&car_world, DENGINE_SCRIPT_FUNC_TERMINATE, NULL);
    denginescript_nsl_free(nsl);

    return 0;
}

#ifndef DENGINE_ANDROID
int main(int argc, char** argv)
{
    DengineInitOpts* opts = dengine_init_get_opts();
    opts->window_title = "testdengine-script-nslbullet-car";
    dengine_init();
    int ret = testdengine_script_nslbullet_car_main(argc, argv);
    dengine_terminate();
    return ret;
}
#endif
