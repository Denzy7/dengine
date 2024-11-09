#include "dengine-script/script.h"

#include <stdio.h>
#include <stdlib.h> //free
#include <string.h> //strdup, memset

#include "dengine-utils/logging.h"

#ifdef DENGINE_ANDROID
#include "dengine-utils/platform/android.h"
#endif
#ifdef DENGINE_SCRIPTING_PYTHON //init table for main python module
#include "dengine-script/python/python.h"
#endif

DENGINE_EXPORT int denginescript_init()
{
#ifdef DENGINE_SCRIPTING_PYTHON
    return denginescript_python_init();
#endif

    //no scripting lang found;
    dengineutils_logging_log("WARNING::Could not find a valid scripting library. Non-native scripting not initialized");
    return 0;
}

DENGINE_EXPORT int denginescript_isinit()
{
#ifdef DENGINE_SCRIPTING_PYTHON
    return denginescript_python_isinit();
#else
    return 0;
#endif
}

DENGINE_EXPORT void denginescript_terminate()
{
#ifdef DENGINE_SCRIPTING_PYTHON
    denginescript_python_terminate();
#endif
}

int denginescript_compile(const char* src, const char* name, ScriptType type, Script* script)
{
    if(type == DENGINE_SCRIPT_TYPE_PYTHON)
    {
        #ifdef DENGINE_SCRIPTING_PYTHON
        return denginescript_python_compile(src, name, script);
        #else
        dengineutils_logging_log("ERROR::Script %s not compiled. Python was not linked at build", name);
        return 0;
        #endif
    }else if(type == DENGINE_SCRIPT_TYPE_NSL)
    {
        dengineutils_logging_log("WARNING::Why are you compiling an NSL library?");
        return 0;
    }
    {
        dengineutils_logging_log("ERROR::Unknown script type");
        return 0;
    }
}

DENGINE_EXPORT int denginescript_call(const Script* script, ScriptFunc func, void* args)
{
    if(script->type == DENGINE_SCRIPT_TYPE_PYTHON)
    {
        #ifdef DENGINE_SCRIPTING_PYTHON
        return denginescript_python_call(script, func, args);
        #else
        dengineutils_logging_log("ERROR::Script not callable. Python was not linked at build");
        return 0;
        #endif
    }else if(script->type == DENGINE_SCRIPT_TYPE_NSL)
    {
        return denginescript_nsl_call(script, func, args);
    }else{
        dengineutils_logging_log("ERROR::Unknown script type");
        return 0;
    }
}

int denginescript_isavailable(ScriptType type)
{
    if(type == DENGINE_SCRIPT_TYPE_PYTHON)
    {
        #ifdef DENGINE_SCRIPTING_PYTHON
        return 1;
        #else
        return 0;
        #endif
    }else if(type == DENGINE_SCRIPT_TYPE_NSL)
    {
        return 1;
    }else{
        dengineutils_logging_log("ERROR::Unknown script type");
        return 0;
    }
}

DENGINE_EXPORT NSL denginescript_nsl_load(const char* file)
{
#ifndef BUILD_SHARED_LIBS
    dengineutils_logging_log("WARNING::Using NSL when static linking is extremely dangerous and may cause crash and incosistencies between upstream libraries and nsl. please recompile with -DBUILD_SHARED_LIBS=ON");
#endif
    NSL load = dengineutils_dynlib_open(file);
    if(!load)
    {
        dengineutils_logging_log("ERROR::Cannot load NSL [%s]", file);
    }
    return load;
}

DENGINE_EXPORT void denginescript_nsl_free(NSL nsl)
{
    dengineutils_dynlib_close(nsl);
}

nslfunc denginescript_nsl_get_func(const NSL nsl, const char* name)
{
    return dengineutils_dynlib_get_sym(nsl, name);;
}

DENGINE_EXPORT int denginescript_nsl_get_script(const char* name, Script* script, const NSL nsl)
{
    memset(script, 0, sizeof(Script));
    char buf[256];
    snprintf(buf, sizeof(buf), "%s_start",name);
    script->nsl_start = denginescript_nsl_get_func(nsl, buf);
    snprintf(buf, sizeof(buf), "%s_update",name);
    script->nsl_update = denginescript_nsl_get_func(nsl, buf);
    snprintf(buf, sizeof(buf), "%s_terminate",name);
    script->nsl_terminate = denginescript_nsl_get_func(nsl, buf);
    if(script->nsl_start || script->nsl_update)
    {
        script->type = DENGINE_SCRIPT_TYPE_NSL;
        return 1;
    }else
    {
        dengineutils_logging_log("WARNING::NSL has no script [%s] with start or update", name);
        return 0;
    }
}

int denginescript_nsl_call(const Script* script, ScriptFunc func, void* args)
{
    int nsl_call = 0;
    if(func == DENGINE_SCRIPT_FUNC_START)
    {
        if(script->nsl_start)
            nsl_call = script->nsl_start(args);
    }else if(func == DENGINE_SCRIPT_FUNC_UPDATE)
    {
        if(script->nsl_update)
            nsl_call = script->nsl_update(args);
    }else if(func == DENGINE_SCRIPT_FUNC_TERMINATE)
    {
        if(script->nsl_terminate)
            nsl_call = script->nsl_terminate(args);
    }
    return nsl_call;
}
