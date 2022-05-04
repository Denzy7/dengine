#include <dengine/dengine.h>
#include <dengine-script/py_modules/scenemodule.h>
int main(int argc, char *argv[])
{
    DengineInitOpts* opts = dengine_init_get_opts();
    opts->window_title = "testdengine-script-python";
    dengine_init();

    const size_t prtbf_sz = 2048;
    char* prtbf = malloc(prtbf_sz);

    const char* script_file = "scripts/printmouse.py";
    File2Mem f2m;
    snprintf(prtbf, prtbf_sz, "%s/%s", dengineutils_filesys_get_assetsdir(), script_file);
    f2m.file = prtbf;
    dengineutils_filesys_file2mem_load(&f2m);

    Script printmouse;
    int compiled = denginescript_python_compile(f2m.mem, script_file, &printmouse);
    dengineutils_filesys_file2mem_free(&f2m);

    Entity* dummy_entitiy = denginescene_ecs_new_entity();
    denginescene_ecs_set_entity_name(dummy_entitiy, "this is a dummy entity with a script attached");
    PyObject* dummy_object = denginescript_pymod_scene_entity_new();
    denginescript_pymod_scene_entity_pull(dummy_object, dummy_entitiy);

    if(compiled)
        denginescript_python_call(&printmouse, DENGINE_SCRIPT_FUNC_START, dummy_object);

    while (dengine_window_isrunning()) {
        if(compiled)
            denginescript_python_call(&printmouse, DENGINE_SCRIPT_FUNC_UPDATE, dummy_object);
        dengine_update();
    }
    free(prtbf);

    dengine_terminate();
    return 0;
}
