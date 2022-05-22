#include "dengine/dgl.h"
#include "dengine-utils/dynlib.h"

void* _dgl_getproc(DynLib lib, const char* name)
{
    void* proc = NULL;
    proc = dengineutils_dynlib_get_sym(lib, name);
    return proc;
}

int dgl_load()
{
    const char* lib;
#ifdef DENGINE_LINUX
    lib = "/usr/lib/libGL.so";
#elif defined (DENGINE_WIN32)
    lib = "C:/Windows/system32/opengl32.dll";
#elif defined(DENGINE_ANDROID)
    lib = "/system/lib/libGLESv2.so";
#else
#error "No comapatible DynLib for dgl"
#endif
    DynLib gl = dengineutils_dynlib_open(lib);
    if(!gl)
        return 0;

    glGetString = _dgl_getproc(gl, "glGetString");
    glGetError = _dgl_getproc(gl, "glGetError");
    glGetIntegerv = _dgl_getproc(gl, "glGetIntegerv");
    glGetFloatv = _dgl_getproc(gl, "glGetFloatv");

    glClear = _dgl_getproc(gl, "glClear");
    glClearColor = _dgl_getproc(gl, "glClearColor");

    glGenBuffers = _dgl_getproc(gl, "glGenBuffers");
    glBufferData = _dgl_getproc(gl, "glBufferData");
    glDeleteBuffers = _dgl_getproc(gl, "glDeleteBuffers");

    glDrawElements = _dgl_getproc(gl, "glDrawElements");
    return 1;
}
