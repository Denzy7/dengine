#include <dengine/dengine.h>

#define APPVER 1U

#ifndef DENGINE_MINGW_ARCH
#error "This is only meant for mingw32!"
#endif

int main(int argc, char* argv[])
{
    dengineutils_filesys_init();

    char stdlib[8192];
    const char* denginescriptdir = dengineutils_filesys_get_filesdir_dengine();

    snprintf(stdlib, sizeof(stdlib),
             "%s/python-dengine-script-%u-%s-%s/python-stdlib",
             denginescriptdir, DENGINE_PYTHON_DENGINE_SCRIPT_VER, PY_VERSION, DENGINE_MINGW_ARCH);
    wchar_t* home = Py_DecodeLocale(stdlib, NULL);
    Py_SetPythonHome(home);

    snprintf(stdlib, sizeof(stdlib),
             "%s/python-dengine-script-%u-%s-%s/python-stdlib.zip;"
             "%s/python-dengine-script-%u-%s-%s/python-stdlib;"
             "%s/python-dengine-script-%u-%s-%s/python-modules;"
             "%s/python-dengine-script-%u-%s-%s/python-stdlib/lib-dynload;"
             "%s/",
             denginescriptdir, DENGINE_PYTHON_DENGINE_SCRIPT_VER, PY_VERSION, DENGINE_MINGW_ARCH,
             denginescriptdir, DENGINE_PYTHON_DENGINE_SCRIPT_VER, PY_VERSION, DENGINE_MINGW_ARCH,
             denginescriptdir, DENGINE_PYTHON_DENGINE_SCRIPT_VER, PY_VERSION, DENGINE_MINGW_ARCH,
             denginescriptdir, DENGINE_PYTHON_DENGINE_SCRIPT_VER, PY_VERSION, DENGINE_MINGW_ARCH,
             denginescriptdir);
    wchar_t* path = Py_DecodeLocale(stdlib, NULL);
    Py_SetPath(path);

    int run = Py_BytesMain(argc, argv);
    dengineutils_filesys_terminate();
    return run;
}
