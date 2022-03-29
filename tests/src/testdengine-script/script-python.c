#include <dengine/dengine.h>

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

    PyScript* script = denginescript_python_new(f2m.mem, script_file);
    dengineutils_filesys_file2mem_free(&f2m);

    if(script)
        denginescript_python_call(script, DENGINE_SCRIPT_FUNC_START, NULL);

    while (dengine_window_isrunning()) {
        if(script)
            denginescript_python_call(script, DENGINE_SCRIPT_FUNC_UPDATE, NULL);
        dengine_update();
    }
    free(script);
    free(prtbf);

    dengine_terminate();
    return 0;
}
