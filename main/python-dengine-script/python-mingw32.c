#include <dengine/dengine.h>

#define APPVER 1U

#if defined(__MINGW32__)
#define ARCH "i686"
#elif defined(__MINGW64__)
#define ARCH "x86_64"
#else
#error "This is only meant for mingw32!"
#endif

int main(int argc, char* argv[])
{
    dengineutils_filesys_init();

    char stdlib[8192];
    const char* denginescriptdir = dengineutils_filesys_get_filesdir_dengine();

    snprintf(stdlib, sizeof(stdlib),
             "%s/python-dengine-script-%u-%s-%s/python-stdlib",
             denginescriptdir, APPVER, PY_VERSION, ARCH);
    wchar_t* home = Py_DecodeLocale(stdlib, NULL);
    Py_SetPythonHome(home);

    snprintf(stdlib, sizeof(stdlib),
             "%s/python-dengine-script-%u-%s-%s/python-stdlib.zip;"
             "%s/python-dengine-script-%u-%s-%s/python-stdlib;"
             "%s/python-dengine-script-%u-%s-%s/python-modules;"
             "%s/python-dengine-script-%u-%s-%s/python-stdlib/lib-dynload;"
             "%s/",
             denginescriptdir, APPVER, PY_VERSION, ARCH,
             denginescriptdir, APPVER, PY_VERSION, ARCH,
             denginescriptdir, APPVER, PY_VERSION, ARCH,
             denginescriptdir, APPVER, PY_VERSION, ARCH,
             denginescriptdir);
    wchar_t* path = Py_DecodeLocale(stdlib, NULL);
    Py_SetPath(path);

    int run = Py_BytesMain(argc, argv);
    dengineutils_filesys_terminate();
    return run;
}
