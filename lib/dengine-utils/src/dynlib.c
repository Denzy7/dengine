#include "dengine-utils/dynlib.h"

#include "dengine-utils/logging.h"
#include "dengine-utils/vtor.h"
#include "dengine-utils/macros.h"

#include <stdio.h> //snprintf
#include <string.h> //strdup
#include <stdlib.h> //malloc,free

#ifdef DENGINE_LINUX
#include <dlfcn.h> //dlopen
#elif defined(DENGINE_WIN32)
#include <windows.h>
#endif

typedef struct{
    char* path;
    char* errmsg;
} DynLib_loadinfo
;

DynLib dengineutils_dynlib_open(const char* path)
{
    DynLib load = NULL;
    DynLib_loadinfo loadinfo;
    DynLib_loadinfo* loadinfos;
    char resolbuf[2048];
    vtor paths;
    const char* appendexts[] = 
    {
        "nsl",
#ifdef DENGINE_LINUX
        "so",
#elif defined(DENGINE_WIN32)
        "dll"
#endif
    };

    memset(&paths, 0, sizeof(paths));
    vtor_create(&paths, sizeof(DynLib_loadinfo));

    loadinfo.path = strdup(path);
    loadinfo.errmsg = NULL;
    vtor_pushback(&paths, &loadinfo);

    for(size_t i = 0; i < DENGINE_ARY_SZ(appendexts); i++)
    {
        snprintf(resolbuf, sizeof(resolbuf), "%s.%s", path, appendexts[i]);
        loadinfo.path = strdup(resolbuf);
        vtor_pushback(&paths, &loadinfo);
    }

#ifdef DENGINE_LINUX

        char* exepath = realpath("/proc/self/exe", NULL);
    *strrchr(exepath, '/') = 0;

    snprintf(resolbuf, sizeof(resolbuf), "%s/%s", exepath, path);
    loadinfo.path = strdup(resolbuf);
    vtor_pushback(&paths, &loadinfo);

    for(size_t i = 0; i < DENGINE_ARY_SZ(appendexts); i++)
    {
        snprintf(resolbuf, sizeof(resolbuf), "%s/%s.%s", exepath, path, appendexts[i]);
        loadinfo.path = strdup(resolbuf);
        vtor_pushback(&paths, &loadinfo);
    }

        free(exepath);
#endif

    /* checking for nsl in dynlib huh? complements LD_LIBRARY_PATH on lunux */
    if(getenv("DENGINE_NSLPATH")){
        snprintf(resolbuf, sizeof(resolbuf), "%s/%s", getenv("DENGINE_NSLPATH"), path);
        loadinfo.path = strdup(resolbuf);
        vtor_pushback(&paths, &loadinfo);

        for(size_t i = 0; i < DENGINE_ARY_SZ(appendexts); i++)
        {
            snprintf(resolbuf, sizeof(resolbuf), "%s/%s", getenv("DENGINE_NSLPATH"), path);
            loadinfo.path = strdup(resolbuf);
            vtor_pushback(&paths, &loadinfo);
        }
    }
    loadinfos = paths.data;
#ifdef DENGINE_LINUX
    for(size_t i = 0; i < paths.count; i++)
    {
        load = dlopen(loadinfos[i].path, RTLD_LAZY);
        if(!load)
            loadinfos[i].errmsg = strdup(dlerror());
        else 
            break;
    }
#elif defined(DENGINE_WIN32)
    for(size_t i = 0; i < paths.count; i++)
    {
        /* windows brain fart */
        for(size_t j = 0; j < strlen(loadinfos[i].path); j++)
        {
            if(loadinfos[i].path[j] == '/')
                loadinfos[i].path[j] = '\\';
        }

        load = LoadLibraryA(loadinfos[i].path);
        if(!load)
        {
            DWORD dw = GetLastError();
            LPSTR lpMsgBuf;
            FormatMessageA(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    dw,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPSTR)&lpMsgBuf,
                    0, NULL );
            loadinfos[i].errmsg = lpMsgBuf;
        }else {
            break;
        }
    }

#else
    dengineutils_logging_log("ERROR::Dynamic loading not supported on this platform!");
    return NULL;;
#endif
    if(load == NULL)
    {
        dengineutils_logging_log("ERROR::Cannot load Dynamic Library [%s]\nTried the following paths and got the following errors:\n", path);
        for(size_t i = 0; i < paths.count; i++)
        {
            dengineutils_logging_log("%s: %s", loadinfos[i].path, loadinfos[i].errmsg);
        }
        dengineutils_logging_log("INFO::You can use envvar DENGINE_NSLPATH with the directory with the nsl");
    }

    for(size_t i = 0; i < paths.count; i++)
    {
        free(loadinfos[i].errmsg);
        free(loadinfos[i].path);
    }
    vtor_free(&paths);
    return load;
}

void dengineutils_dynlib_close(DynLib dynlib)
{
#ifdef DENGINE_LINUX
    dlclose(dynlib);
#elif defined(DENGINE_WIN32)
    FreeLibrary(dynlib);
#endif
}

void* dengineutils_dynlib_get_sym(const DynLib dynlib, const char* name)
{
#ifdef DENGINE_LINUX
    return dlsym(dynlib, name);
#elif defined(DENGINE_WIN32)
    return GetProcAddress(dynlib, name);
#endif
}
