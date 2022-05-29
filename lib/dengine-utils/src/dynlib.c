#include "dengine-utils/dynlib.h"

#include "dengine-utils/logging.h"

#include <stdio.h> //snprintf
#include <string.h> //strdup
#include <stdlib.h> //malloc,free

#ifdef DENGINE_LINUX
#include <dlfcn.h> //dlopen
#elif defined(DENGINE_WIN32)
#include <libloaderapi.h> //LoadLibrary
#include <errhandlingapi.h> //GetLastError
#endif

DynLib dengineutils_dynlib_open(const char* path)
{
    DynLib load = NULL;
    char* errmsg = NULL;
#ifdef DENGINE_LINUX
    load = dlopen(path, RTLD_LAZY);
    if(!load)
        errmsg = strdup(dlerror());
#elif defined(DENGINE_WIN32)
    char* dlpath = strdup(path);
    for(size_t i = 0; i < strlen(dlpath); i++)
    {
        if(dlpath[i] == '/')
            dlpath[i] = '\\';
    }
    load = LoadLibraryA(dlpath);
    if(!load)
    {
          DWORD dw = GetLastError();
//        LPVOID lpMsgBuf;


//        FormatMessage(
//            FORMAT_MESSAGE_ALLOCATE_BUFFER |
//            FORMAT_MESSAGE_FROM_SYSTEM |
//            FORMAT_MESSAGE_IGNORE_INSERTS,
//            NULL,
//            dw,
//            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
//            (LPTSTR) &lpMsgBuf,
//            0, NULL );
//        int len = lstrlen((LPCTSTR)lpMsgBuf);
        errmsg = malloc(256);
        snprintf(errmsg, 256, "Failed to load DLL : %ld", dw);
//        LocalFree(lpMsgBuf);
    }
    free(dlpath);
#else
    dengineutils_logging_log("ERROR::Dynamic loading not supported on this platform!");
    return NULL;;
#endif
    if(errmsg)
    {
        dengineutils_logging_log("ERROR::Cannot load Dynamic Library [%s]\nReason : %s\n", path, errmsg);
        free(errmsg);
    }
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
