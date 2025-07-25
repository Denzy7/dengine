#include <dengine/dengine.h>
#include <dengine-script/python/python.h>
int main(int argc, char *argv[])
{
    DengineInitOpts* opts = dengine_init_get_opts();
    opts->window_title = "testdengine-script-python";
    dengine_init();

    const char* script_file = "scripts/printmouse.py";
    File2Mem f2m;
    dengine_load_asset(script_file, &f2m.mem, &f2m.size);

    Script printmouse;
    int compiled = denginescript_python_compile(f2m.mem, script_file, &printmouse);
    dengineutils_filesys_file2mem_free(&f2m);

    /*
     * this script does not use ecs. so we just pass NULL to update() and start().
     * if we wanted, we'd create a dummy ecs object and ecs python object,
     * pull and push some data (this is actually how scene works by creating
     * a dummy object which pulls/pushes data. see assets/scripts/moveduck.py) :
     *
     * #include <dengine-script/py_modules/scenemodule.h>
     * ...
     *
     * Entity* dummy_entitiy = denginescene_ecs_new_entity();
     * denginescene_ecs_set_entity_name(dummy_entitiy, "this is a dummy entity with a script attached");
     * PyObject* dummy_object = denginescript_pymod_scene_entity_new();
     * denginescript_pymod_scene_entity_pull(dummy_object, dummy_entitiy);
     * ...
     *
     * denginescript_python_call(<SCRIPT>, <FUNC>, dummy_object);
     * ...
     *
     * denginescene_ecs_destroy_entity(dummy_entitiy);
     *
     * ===============================================
     *
     * if its a scene, simply add the script with:
     * denginescene_ecs_add_script(<ENTITY>, <SCRIPT>);
     *
     * and its execute start and update and passes the
     * ecs python object as args once the scene object
     * is traversed
     *
     * see a complete example in tests/src/testdengine-scene/scene-ecs.c
     */


    if(compiled)
        denginescript_python_call(&printmouse, DENGINE_SCRIPT_FUNC_START, NULL);

    while (dengine_update()) {
        if(compiled)
            denginescript_python_call(&printmouse, DENGINE_SCRIPT_FUNC_UPDATE, NULL);
    }

    dengine_terminate();
    return 0;
}
